/**
 * ClientSession.cpp
 **/
#include <string.h>
#include <sstream>
#include <list>
#include "../net/tcpconnection.h"
#include "../net/protocolstream.h"
#include "../base/logging.h"
#include "../base/singleton.h"
#include "../jsoncpp-0.5.0/json.h"
#include "Msg.h"
#include "UserManager.h"
#include "IMServer.h"
#include "MsgCacheManager.h"
#include "ClientSession.h"

//°ü×î´ó×Ö½ÚÊýÏÞÖÆÎª10M
#define MAX_PACKAGE_SIZE    10 * 1024 * 1024

using std::shared_ptr;
using std::string;

using namespace balloon;

//ÔÊÐíµÄ×î´óÊ±Êý¾Ý°üÀ´Íù¼ä¸ô£¬ÕâÀïÉèÖÃ³É30Ãë
#define MAX_NO_PACKAGE_INTERVAL  30

ClientSession::ClientSession(const shared_ptr<TcpConnection>& conn) :  
TcpSession(conn), 
m_id(0),
m_seq(0),
m_isLogin(false)
{
	m_userinfo.userid = 0;
    m_lastPackageTime = time(NULL);

    //ÔÝÇÒ×¢ÊÍµô£¬²»ÀûÓÚµ÷ÊÔ
    //EnableHearbeatCheck();
}

ClientSession::~ClientSession()
{
    
}


/*
@1.Êý¾Ý°üµÄÑéÖ¤¹ý³Ì¡£
@2.Ñ­»·´¦Àí£¬ÈôÒ»¸ö°ü´¦ÀíÍêºó£¬
   ¿ÉÒÔ¼ÌÐøÔÚÑ­»·ÖÐ´¦ÀíÏÂÒ»¸ö°ü£¬¼õÉÙº¯Êý·µ»ØºÍµ÷ÓÃ¡£
*/
void ClientSession::OnRead(const shared_ptr<TcpConnection>& conn, Buffer* pBuffer, Timestamp receivTime)
{
    while (true)
    {
		//@2.ÑéÖ¤°üÍ·ÊÇ·ñ½ÓÊÕÍêÕû¡£
        if (pBuffer->readableBytes() < (size_t)sizeof(msg))
        {
            /*LOG_INFO << "buffer is not enough for a package header, pBuffer->readableBytes()=" 
			         << pBuffer->readableBytes() << ", sizeof(msg)=" << sizeof(msg);*/
            return;
        }

		//@3.ÑéÖ¤°üÍ·ÐÅÏ¢ÊÇ·ñºÏ·¨¡£
        msg header;	
        memcpy(&header, pBuffer->peek(), sizeof(msg));
        if (header.packagesize <= 0 || header.packagesize > MAX_PACKAGE_SIZE)
        {
            //¿Í»§¶Ë·¢·Ç·¨Êý¾Ý°ü£¬·þÎñÆ÷Ö÷¶¯¹Ø±ÕÖ®
            LOG_ERROR << "Illegal package heade size, close TcpConnection, client: " << conn->peerAddress().toIpPort();
            conn->forceClose();
        }
		//@4.ÑéÖ¤°üÊý¾ÝÊÇ·ñ½ÓÊÕÍêÕû¡£
        if (pBuffer->readableBytes() < (size_t)header.packagesize + sizeof(msg))
            return;

		//@5.¶ÁÈ¡Êý¾Ý£¬µ÷ÓÃÊý¾Ý´¦Àíº¯Êý¡£
        pBuffer->retrieve(sizeof(msg));
        string inbuf;
        inbuf.append(pBuffer->peek(), header.packagesize);
        pBuffer->retrieve(header.packagesize);
        if (!Process(conn, inbuf.c_str(), inbuf.length()))
        {
            //¿Í»§¶Ë·¢·Ç·¨Êý¾Ý°ü£¬·þÎñÆ÷Ö÷¶¯¹Ø±ÕÖ®
            LOG_ERROR << "Process error, close TcpConnection, client: " << conn->peerAddress().toIpPort();
            conn->forceClose();
        }
        else
            m_lastPackageTime = time(NULL);
    }// end while-loop

}


/*
@1.const char* + size_t ²»´í!!!
@.2½âÎöÊý¾Ý£¬´¦ÀíÊý¾Ý¡£
*/
bool ClientSession::Process(const shared_ptr<TcpConnection>& conn, const char* inbuf, size_t length)
{
    balloon::BinaryReadStream readStream(inbuf, length);
    int32_t cmd;
    if (!readStream.ReadInt32(cmd))
    {
        LOG_WARN << "read cmd error, client: " << conn->peerAddress().toIpPort();
        return false;
    }

    //int seq;
    if (!readStream.ReadInt32(m_seq))
    {
        LOG_ERROR << "read seq error, client: " << conn->peerAddress().toIpPort();
        return false;
    }

    string data;
    size_t datalength;
    if (!readStream.ReadString(&data, 0, datalength))
    {
        LOG_ERROR << "read data error, client: " << conn->peerAddress().toIpPort();
        return false;
    }
   
    //ÐÄÌø°üÌ«Æµ·±£¬²»´òÓ¡
    if (cmd != msg_type_heartbeat)
        LOG_INFO << "Request from client: userid=" << m_userinfo.userid << ", cmd=" << cmd << ", seq=" << m_seq << ", data=" << data << ", datalength=" << datalength << ", packageBodySize=" << length;
    //LOG_DEBUG_BIN((unsigned char*)inbuf, length);

    switch (cmd)
    {
        //ÐÄÌø°ü
        case msg_type_heartbeat:
            OnHeartbeatResponse(conn);
            break;

        //×¢²á
        case msg_type_register:
            OnRegisterResponse(data, conn);
            break;        

        //µÇÂ¼
        case msg_type_login:                          
            OnLoginResponse(data, conn);
            break;     

        //ÆäËûÃüÁî±ØÐëÔÚÒÑ¾­µÇÂ¼µÄÇ°ÌáÏÂ²ÅÄÜ½øÐÐ²Ù×÷
        default:
        {
            if (m_isLogin)
            {
                switch (cmd)
                {
                    //»ñÈ¡ºÃÓÑÁÐ±í
                    case msg_type_getofriendlist:
                        OnGetFriendListResponse(conn);
                        break;

                    //²éÕÒÓÃ»§
                    case msg_type_finduser:
                        OnFindUserResponse(data, conn);
                        break;

                    //¼ÓºÃÓÑ
                    case msg_type_operatefriend:    
                        OnOperateFriendResponse(data, conn);
                        break;

                    //ÓÃ»§Ö÷¶¯¸ü¸Ä×Ô¼ºÔÚÏß×´Ì¬
                    case msg_type_userstatuschange:
        	            OnChangeUserStatusResponse(data, conn);
                        break;

                    //¸üÐÂÓÃ»§ÐÅÏ¢
                    case msg_type_updateuserinfo:
                        OnUpdateUserInfoResponse(data, conn);
                        break;
        
                    //ÐÞ¸ÄÃÜÂë
                    case msg_type_modifypassword:
                        OnModifyPasswordResponse(data, conn);
                        break;
        
                    //´´½¨Èº
                    case msg_type_creategroup:
                        OnCreateGroupResponse(data, conn);
                        break;

                    //»ñÈ¡Ö¸¶¨Èº³ÉÔ±ÐÅÏ¢
                    case msg_type_getgroupmembers:
                        OnGetGroupMembersResponse(data, conn);
                        break;

                    //ÁÄÌìÏûÏ¢
                    case msg_type_chat:
                    {
                        int32_t target;
                        if (!readStream.ReadInt32(target))
                        {
                            LOG_ERROR << "read target error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }
                        OnChatResponse(target, data, conn);
                    }
                        break;
        
                    //Èº·¢ÏûÏ¢
                    case msg_type_multichat:
                    {
                        std::string targets;
                        size_t targetslength;
                        if (!readStream.ReadString(&targets, 0, targetslength))
                        {
                            LOG_ERROR << "read targets error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }

                        OnMultiChatResponse(targets, data, conn);
                    }

                        break;

                    //ÆÁÄ»½ØÍ¼
                    case msg_type_screenshot:
                    {
                        string bmpHeader;
                        size_t bmpHeaderlength;
                        if (!readStream.ReadString(&bmpHeader, 0, bmpHeaderlength))
                        {
                            LOG_ERROR << "read bmpheader error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }

                        string bmpData;
                        size_t bmpDatalength;
                        if (!readStream.ReadString(&bmpData, 0, bmpDatalength))
                        {
                            LOG_ERROR << "read bmpdata error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }
                                   
                        int32_t target;
                        if (!readStream.ReadInt32(target))
                        {
                            LOG_ERROR << "read target error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }
                        OnScreenshotResponse(target, bmpHeader, bmpData, conn);
                    }
                        break;

                    //ÉÏ´«Éè±¸ÐÅÏ¢
                    case msg_type_uploaddeviceinfo:
                    {
                        int32_t deviceid;
                        if (!readStream.ReadInt32(deviceid))
                        {
                            LOG_ERROR << "read deviceid error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }

                        int32_t classtype;
                        if (!readStream.ReadInt32(classtype))
                        {
                            LOG_ERROR << "read classtype error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }

                        int64_t uploadtime;
                        if (!readStream.ReadInt64(uploadtime))
                        {
                            LOG_ERROR << "read uploadtime error, client: " << conn->peerAddress().toIpPort();
                            return false;
                        }

                        OnUploadDeviceInfo(deviceid, classtype, uploadtime, data, conn);
                    }
                        break;
                        

                    default:
                        //pBuffer->retrieveAll();
                        LOG_ERROR << "unsupport cmd, cmd:" << cmd << ", data=" << data << ", connection name:" << conn->peerAddress().toIpPort();
                        //conn->forceClose();
                        return false;
                }// end inner-switch
            }
            else
            {
                //ÓÃ»§Î´µÇÂ¼£¬¸æËß¿Í»§¶Ë²»ÄÜ½øÐÐ²Ù×÷ÌáÊ¾¡°Î´µÇÂ¼¡±
                string data = "{\"code\": 2, \"msg\": \"not login, please login first!\"}";
                Send(cmd, m_seq, data);
                LOG_INFO << "Response to client: cmd=" << cmd << ", data=" << data << ", sessionId=" << m_id;                
            }// end if
         }// end default
    }// end outer-switch
    ++ m_seq;
    return true;
}

void ClientSession::OnHeartbeatResponse(const shared_ptr<TcpConnection>& conn)
{
    string dummydata;    
    Send(msg_type_heartbeat, m_seq, dummydata);

    //ÐÄÌø°üÈÕÖ¾¾Í²»Òª´òÓ¡ÁË£¬ºÜÈÝÒ×Ð´ÂúÈÕÖ¾
    //LOG_INFO << "Response to client: cmd=1000" << ", sessionId=" << m_id;
}
//ÒÑÍê³É
void ClientSession::OnRegisterResponse(const string& data, const shared_ptr<TcpConnection>& conn)
{
    //{ "user": "13917043329", "nickname" : "balloon", "password" : "123" }
    Json::Reader JsonReader;
    Json::Value JsonRoot;

	//@1.½âÎö³ö´íÔò·µ»Ø¡£
	if(  ( !JsonReader.parse(data, JsonRoot) ) ||
		 ( !JsonRoot["username"].isString() || 
		   !JsonRoot["nickname"].isString() || 
		   !JsonRoot["password"].isString()) )
	  {
		  LOG_WARN << "invalid json: " << data << ", sessionId = " << m_id << ", client: " << conn->peerAddress().toIpPort();
		  return;
	  }

    UserInfo u;
    u.username = JsonRoot["username"].asString();
    u.nickname = JsonRoot["nickname"].asString();
    u.password = JsonRoot["password"].asString();

    string retData;
    UserInfo cachedUser;
    cachedUser.userid = 0;
    if(Singleton<UserManager>::Instance().getUserInfoByUsername(u.username, cachedUser) == true)
        retData = "{\"code\": 101, \"msg\": \"registered already\"}";
    else
    {
        if (!Singleton<UserManager>::Instance().addUser(u))
            retData = "{\"code\": 100, \"msg\": \"register failed\"}";
        else
            retData = "{\"code\": 0, \"msg\": \"ok\"}";
    }


    Send(msg_type_register, m_seq, retData);

    LOG_INFO << "Response to client: cmd=msg_type_register" << ", userid=" << u.userid << ", data=" << retData;
}
//ÒÑÍê³É
void ClientSession::OnLoginResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    //{"username": "13917043329", "password": "123", "clienttype": 1, "status": 1}
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", sessionId = " << m_id  << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["username"].isString() || !JsonRoot["password"].isString() || !JsonRoot["clienttype"].isInt() || !JsonRoot["status"].isInt())
    {
        LOG_WARN << "invalid json: " << data << ", sessionId = " << m_id << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    string username = JsonRoot["username"].asString();
    string password = JsonRoot["password"].asString();
    int clientType = JsonRoot["clienttype"].asInt();
    std::ostringstream os;
    UserInfo cachedUser;
    cachedUser.userid = 0;
    IMServer& imserver = Singleton<IMServer>::Instance();
    if (Singleton<UserManager>::Instance().getUserInfoByUsername(username, cachedUser) == false)
    {
        //TODO: ÕâÐ©Ó²±àÂëµÄ×Ö·ûÓ¦¸ÃÍ³Ò»·Åµ½Ä³¸öµØ·½Í³Ò»¹ÜÀí
        os << "{\"code\": 102, \"msg\": \"not registered\"}";
    }
    else
    {
        if (cachedUser.password != password)
            os << "{\"code\": 103, \"msg\": \"incorrect password\"}";
        else
        {
            //Èç¹û¸ÃÕËºÅÒÑ¾­µÇÂ¼£¬Ôò½«Ç°Ò»¸öÕËºÅÌßÏÂÏß
            std::shared_ptr<ClientSession> targetSession;
            //ÓÉÓÚ·þÎñÆ÷¶ËÖ§³Ö¶àÀàÐÍÖÕ¶ËµÇÂ¼£¬ËùÒÔÖ»ÓÐÍ¬Ò»ÀàÐÍµÄÖÕ¶ËÇÒÍ¬Ò»¿Í»§¶ËÀàÐÍ²ÅÈÏÎªÊÇÍ¬Ò»¸ösession
            imserver.GetSessionByUserIdAndClientType(targetSession, cachedUser.userid, clientType);
            if (targetSession)
            {                              
                string dummydata;
                targetSession->Send(msg_type_kickuser, m_seq, dummydata);
                //±»ÌßÏÂÏßµÄSession±ê¼ÇÎªÎÞÐ§µÄ
                targetSession->MakeSessionInvalid();

                LOG_INFO << "Response to client: userid=" << targetSession->GetUserId() << ", cmd=msg_type_kickuser";

                //¹Ø±ÕÁ¬½Ó
                //targetSession->GetConnectionPtr()->forceClose();
            }           
            
            //¼ÇÂ¼ÓÃ»§ÐÅÏ¢
            m_userinfo.userid = cachedUser.userid;
            m_userinfo.username = username;
            m_userinfo.nickname = cachedUser.nickname;
            m_userinfo.password = password;
            m_userinfo.clienttype = JsonRoot["clienttype"].asInt();
            m_userinfo.status = JsonRoot["status"].asInt();

            os << "{\"code\": 0, \"msg\": \"ok\", \"userid\": " << m_userinfo.userid << ",\"username\":\"" << cachedUser.username << "\", \"nickname\":\"" 
               << cachedUser.nickname << "\", \"facetype\": " << cachedUser.facetype << ", \"customface\":\"" << cachedUser.customface << "\", \"gender\":" << cachedUser.gender
               << ", \"birthday\":" << cachedUser.birthday << ", \"signature\":\"" << cachedUser.signature << "\", \"address\": \"" << cachedUser.address
               << "\", \"phonenumber\": \"" << cachedUser.phonenumber << "\", \"mail\":\"" << cachedUser.mail << "\"}";            
        }
    }
   
    //µÇÂ¼ÐÅÏ¢Ó¦´ð
    Send(msg_type_login, m_seq, os.str());

    LOG_INFO << "Response to client: cmd=msg_type_login, data=" << os.str() << ", userid=" << m_userinfo.userid;

    //ÉèÖÃÒÑ¾­µÇÂ¼µÄ±êÖ¾
    m_isLogin = true;

    //ÍÆËÍÀëÏßÍ¨ÖªÏûÏ¢
    std::list<NotifyMsgCache> listNotifyCache;
    Singleton<MsgCacheManager>::Instance().GetNotifyMsgCache(m_userinfo.userid, listNotifyCache);
    for (const auto &iter : listNotifyCache)
    {
        Send(iter.notifymsg);
    }

    //ÍÆËÍÀëÏßÁÄÌìÏûÏ¢
    std::list<ChatMsgCache> listChatCache;
    Singleton<MsgCacheManager>::Instance().GetChatMsgCache(m_userinfo.userid, listChatCache);
    for (const auto &iter : listChatCache)
    {
        Send(iter.chatmsg);
    }

    //¸øÆäËûÓÃ»§ÍÆËÍÉÏÏßÏûÏ¢
    std::list<UserInfo> friends;
    Singleton<UserManager>::Instance().getFriendInfoByUserId(m_userinfo.userid, friends);
    for (const auto& iter : friends)
    {
        //ÒòÎª´æÔÚÒ»¸öÓÃ»§id£¬¶à¸öÖÕ¶Ë£¬ËùÒÔ£¬Í¬Ò»¸öuserid¿ÉÄÜ¶ÔÓ¦¶à¸ösession
        std::list<std::shared_ptr<ClientSession>> sessions;
        imserver.GetSessionsByUserId(sessions, iter.userid);
        for (auto& iter2 : sessions)
        {
            if (iter2)
            {
                iter2->SendUserStatusChangeMsg(m_userinfo.userid, 1, m_userinfo.status);

                LOG_INFO << "SendUserStatusChangeMsg to user(userid=" << iter2->GetUserId() << "): user go online, online userid = " << m_userinfo.userid << ", status = " << m_userinfo.status;
            }
        }
    }  
}
//Î´×öÐÞ¸Ä
void ClientSession::OnGetFriendListResponse(const std::shared_ptr<TcpConnection>& conn)
{
    string teaminfo;
    Singleton<UserManager>::Instance().getTeamInfoByUserId(m_userinfo.userid, teaminfo);
    if (teaminfo.empty())
    {
        std::list<UserInfo> friends;
        string strUserInfo;
        int32_t userstatus = 0;
        int32_t clientType = 0;
        Singleton<UserManager>::Instance().getFriendInfoByUserId(m_userinfo.userid, friends);
        std::list<GroupInfo> groupsInfo;
        Singleton<UserManager>::Instance().getGroupsInfoByUserId(m_userinfo.userid, groupsInfo);
       std::list<GroupInfo>::iterator it = groupsInfo.begin();
       for(; it != groupsInfo.end(); ++it)
         std::cout << "here .......get the group name=" << it->groupName << std::endl;
        /*
        [
        {
        "teamindex": 0,
        "teamname": "ÎÒµÄºÃÓÑ",
        "members": [
        {
        "userid": 1,
        "markname": "ÕÅÄ³Ä³"
        },
        {
        "userid": 2,
        "markname": "ÕÅxx"
        }
        ]
        }
        ]
        */

        ostringstream osTeamInfo;
        osTeamInfo << "[{\"teamindex\": 0, \"teamname\" : \"My Friends\", \"members\" : ";
        //¿¿¿¿
        for (const auto& iter : friends)
        {
            //userstatus = imserver.GetUserStatusByUserId(iter.userid);
            //clientType = imserver.GetUserClientTypeByUserId(iter.userid);
            /*
            {"code": 0, "msg": "ok", "userinfo":[{"userid": 1,"username":"qqq,
            "nickname":"qqq, "facetype": 0, "customface":"", "gender":0, "birthday":19900101,
            "signature":", "address": "", "phonenumber": "", "mail":", "clienttype": 1, "status":1"]}
            */
            ostringstream osSingleUserInfo;
            osSingleUserInfo << "{\"userid\": " << iter.userid << ",\"username\":\"" << iter.username << "\", \"nickname\":\"" << iter.nickname
                << "\", \"facetype\": " << iter.facetype << ", \"customface\":\"" << iter.customface << "\", \"gender\":" << iter.gender
                << ", \"birthday\":" << iter.birthday << ", \"signature\":\"" << iter.signature << "\", \"address\": \"" << iter.address
                << "\", \"phonenumber\": \"" << iter.phonenumber << "\", \"mail\":\"" << iter.mail << "\", \"clienttype\":" << clientType
                << ", \"status\":" << userstatus << "}";

            strUserInfo += osSingleUserInfo.str();
            strUserInfo += ",";
        }
      
        //¿¿¿
        for (const auto& iter : groupsInfo)
        {
            ostringstream osSingleUserInfo;
            osSingleUserInfo << "{\"userid\": " << iter.tid << ",\"username\":\"" << iter.groupName << "\", \"nickname\":\"" << iter.groupName
                << "\", \"facetype\": " << 0 << ", \"customface\":\"" <<"" << "\", \"gender\":" << 0
                << ", \"birthday\":" << 0 << ", \"signature\":\"" << "" << "\", \"address\": \"" << ""
                << "\", \"phonenumber\": \"" << "" << "\", \"mail\":\"" << "" << "\", \"clienttype\":" << 1
                << ", \"status\":" << 1 << "}";

            strUserInfo += osSingleUserInfo.str();
            strUserInfo += ",";
        }


        
        //È¥µô×îºó¶àÓàµÄ¶ººÅ
        strUserInfo = strUserInfo.substr(0, strUserInfo.length() - 1);
        std::ostringstream os;
        os << "{\"code\": 0, \"msg\": \"ok\", \"userinfo\":" << osTeamInfo.str() << "[" << strUserInfo << "]}]}";
        Send(msg_type_getofriendlist, m_seq, os.str());

        LOG_INFO << "Response to client: userid=" << m_userinfo.userid << ", cmd=msg_type_getofriendlist, data=" << os.str();
    }
}
//ÒÑÍê³É
void ClientSession::OnChangeUserStatusResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    //{"type": 1, "onlinestatus" : 1}
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["type"].isInt() || !JsonRoot["onlinestatus"].isInt())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    int newstatus = JsonRoot["onlinestatus"].asInt();
    if (m_userinfo.status == newstatus)
        return;

    //¸üÐÂÏÂµ±Ç°ÓÃ»§µÄ×´Ì¬
    m_userinfo.status = newstatus;

    //TODO: Ó¦´ðÏÂ×Ô¼º¸æËß¿Í»§¶ËÐÞ¸Ä³É¹¦

    IMServer& imserver = Singleton<IMServer>::Instance();
    std::list<UserInfo> friends;
    Singleton<UserManager>::Instance().getFriendInfoByUserId(m_userinfo.userid, friends);
    for (const auto& iter : friends)
    {
        //ÒòÎª´æÔÚÒ»¸öÓÃ»§id£¬¶à¸öÖÕ¶Ë£¬ËùÒÔ£¬Í¬Ò»¸öuserid¿ÉÄÜ¶ÔÓ¦¶à¸ösession
        std::list<std::shared_ptr<ClientSession>> sessions;
        imserver.GetSessionsByUserId(sessions, iter.userid);
        for (auto& iter2 : sessions)
        {
            if (iter2)
                iter2->SendUserStatusChangeMsg(m_userinfo.userid, 1, newstatus);
        }
    }
}
//ÒÑÍê³É
void ClientSession::OnFindUserResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    //{ "type": 1, "username" : "zhangyl" }
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["type"].isInt() || !JsonRoot["username"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    string retData;
    //TODO: Ä¿Ç°Ö»Ö§³Ö²éÕÒµ¥¸öÓÃ»§
    string username = JsonRoot["username"].asString();
    int usernameNum = atoi(username.c_str());
    if( username.size() == 11 ) 
    {
        std::cout << "find user ..." << username << std::endl;
        UserInfo cachedUser;
        if (!Singleton<UserManager>::Instance().getUserInfoByUsername(username, cachedUser))
            retData = "{ \"code\": 0, \"msg\": \"ok\", \"userinfo\": [] }";
        else
        {
          //TODO: ÓÃ»§±È½Ï¶àµÄÊ±ºò£¬Ó¦¸ÃÊ¹ÓÃ¶¯Ì¬string
            char szUserInfo[256] = { 0 };
            snprintf(szUserInfo, 256, "{ \"code\": 0, \"msg\": \"ok\", \"userinfo\": [{\"userid\": %d, \"username\": \"%s\", \"nickname\": \"%s\", \"facetype\":%d}] }", cachedUser.userid, cachedUser.username.c_str(), cachedUser.nickname.c_str(), cachedUser.facetype);
            retData = szUserInfo;
        } 
    }

    else
    {
       std::cout << "find group ..." << usernameNum << std::endl;
        GroupInfo cachedGroup;
        if (!Singleton<UserManager>::Instance(). getGroupInfoByGroupId(usernameNum, cachedGroup))
            retData = "{ \"code\": 0, \"msg\": \"ok\", \"userinfo\": [] }";    
         else
        {
           char szUserInfo[256] = { 0 };
           snprintf(szUserInfo, 256, "{ \"code\": 0, \"msg\": \"ok\", \"userinfo\": [{\"userid\": %d, \"username\": \"%s\", \"nickname\": \"%s\", \"facetype\":%d}] }", cachedGroup.tid, username.c_str(), cachedGroup.groupName.c_str(), 0);
            retData = szUserInfo;
        }
    }

  
    Send(msg_type_finduser, m_seq, retData);
    LOG_INFO << "Response to client: userid = " << m_userinfo.userid << ", cmd=msg_type_finduser, data=" << retData;
}

void ClientSession::OnOperateFriendResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["type"].isInt() || !JsonRoot["userid"].isInt())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    int type = JsonRoot["type"].asInt();
    int32_t targetUserid = JsonRoot["userid"].asInt();
    if (targetUserid >= GROUPID_BOUBDARY)
    {
        if (type == 4)
        {
            //ÍËÈº
            DeleteFriend(conn, targetUserid);
            return;
        }

        //¼ÓÈºÖ±½ÓÍ¬Òâ
        OnAddGroupResponse(targetUserid, conn);
        return;
    }

    char szData[256] = { 0 };
    //É¾³ýºÃÓÑ
    if (type == 4)
    {
        DeleteFriend(conn, targetUserid);
        return;
    }
    //·¢³ö¼ÓºÃÓÑÉêÇë
    if (type == 1)
    {
        //{"userid": 9, "type": 1, }        
        snprintf(szData, 256, "{\"userid\":%d, \"type\":2, \"username\": \"%s\"}", m_userinfo.userid, m_userinfo.username.c_str());
    }
    //Ó¦´ð¼ÓºÃÓÑ
    else if (type == 3)
    {
        if (!JsonRoot["accept"].isInt())
        {
            LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << "client: " << conn->peerAddress().toIpPort();
            return;
        }

        int accept = JsonRoot["accept"].asInt();
        //½ÓÊÜ¼ÓºÃÓÑÉêÇëºó£¬½¨Á¢ºÃÓÑ¹ØÏµ
        if (accept == 1)
        {
            int smallid = m_userinfo.userid;
            int greatid = targetUserid;
            //Êý¾Ý¿âÀïÃæ»¥ÎªºÃÓÑµÄÁ½¸öÈËid£¬Ð¡ÕßÔÚÏÈ£¬´óÕßÔÚºó
            if (smallid > greatid)
            {
                smallid = targetUserid;
                greatid = m_userinfo.userid;
            }

            if (!Singleton<UserManager>::Instance().makeFriendRelationship(smallid, greatid))
            {
                LOG_ERROR << "make relationship error: " << data << ", userid: " << m_userinfo.userid << "client: " << conn->peerAddress().toIpPort();
                return;
            }
        }

        //{ "userid": 9, "type" : 3, "userid" : 9, "username" : "xxx", "accept" : 1 }
        snprintf(szData, 256, "{\"userid\": %d, \"type\": 3, \"username\": \"%s\", \"accept\": %d}", m_userinfo.userid, m_userinfo.username.c_str(), accept);

        //ÌáÊ¾×Ô¼ºµ±Ç°ÓÃ»§¼ÓºÃÓÑ³É¹¦
        UserInfo targetUser;
        if (!Singleton<UserManager>::Instance().getUserInfoByUserId(targetUserid, targetUser))
        {
            LOG_ERROR << "Get Userinfo by id error, targetuserid: " << targetUserid << ", userid: " << m_userinfo.userid << ", data: "<< data << ", client: " << conn->peerAddress().toIpPort();
            return;
        }
        char szSelfData[256] = { 0 };
        snprintf(szSelfData, 256, "{\"userid\": %d, \"type\": 3, \"username\": \"%s\", \"accept\": %d}", targetUser.userid, targetUser.username.c_str(), accept);
        Send(msg_type_operatefriend, m_seq, szSelfData, strlen(szSelfData));
        LOG_INFO << "Response to client: userid=" << m_userinfo.userid << ", cmd=msg_type_addfriend, data=" << szSelfData;
    }

    //ÌáÊ¾¶Ô·½¼ÓºÃÓÑ³É¹¦
    string outbuf;
    balloon::BinaryWriteStream writeStream(&outbuf);
    writeStream.WriteInt32(msg_type_operatefriend);
    writeStream.WriteInt32(m_seq);
    writeStream.WriteCString(szData, strlen(szData));
    writeStream.Flush();


    //ÏÈ¿´Ä¿±êÓÃ»§ÊÇ·ñÔÚÏß
    std::list<std::shared_ptr<ClientSession>> sessions;
    Singleton<IMServer>::Instance().GetSessionsByUserId(sessions, targetUserid);
    //Ä¿±êÓÃ»§²»ÔÚÏß£¬»º´æÕâ¸öÏûÏ¢
    if (sessions.empty())
    {
        Singleton<MsgCacheManager>::Instance().AddNotifyMsgCache(targetUserid, outbuf);
        LOG_INFO << "userid: " << targetUserid << " is not online, cache notify msg, msg: " << outbuf;
        return;
    }

    for (auto& iter : sessions)
    {
        iter->Send(outbuf);
    }

    LOG_INFO << "Response to client: userid = " << targetUserid << ", cmd=msg_type_addfriend, data=" << data;
}
//ÒÑÍê³É
void ClientSession::OnAddGroupResponse(int32_t groupId, const std::shared_ptr<TcpConnection>& conn)
{
    if (!Singleton<UserManager>::Instance().joinGroup(m_userinfo.userid, groupId))
    {
        LOG_ERROR << "make relationship error, groupId: " << groupId << ", userid: " << m_userinfo.userid << "client: " << conn->peerAddress().toIpPort();
        return;
    }
    
    GroupInfo group;
    if (!Singleton<UserManager>::Instance().getGroupInfoByGroupId(groupId, group))
    {
        LOG_ERROR << "Get group info by id error, targetuserid: " << groupId << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }
    char szSelfData[256] = { 0 };
    snprintf(szSelfData, 256, "{\"userid\": %d, \"type\": 3, \"username\": \"%s\", \"accept\": 3}", group.tid, group.groupName.c_str());
    Send(msg_type_operatefriend, m_seq, szSelfData, strlen(szSelfData));
    LOG_INFO << "Response to client: cmd=msg_type_addfriend, data=" << szSelfData << ", userid=" << m_userinfo.userid;

    //¸øÆäËûÔÚÏßÈº³ÉÔ±ÍÆËÍÈºÐÅÏ¢·¢Éú±ä»¯µÄÏûÏ¢
    std::list<UserInfo> friends;
    Singleton<UserManager>::Instance().getGroupMembers(groupId, friends);
    IMServer& imserver = Singleton<IMServer>::Instance();
    for (const auto& iter : friends)
    {
        //ÏÈ¿´Ä¿±êÓÃ»§ÊÇ·ñÔÚÏß
        std::list< std::shared_ptr<ClientSession>> targetSessions;
        imserver.GetSessionsByUserId(targetSessions, iter.userid);
        for (auto& iter2 : targetSessions)
        {
            if (iter2)
                iter2->SendUserStatusChangeMsg(groupId, 3);
        }
    }
}
//ÒÑÍê³É
void ClientSession::OnUpdateUserInfoResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["nickname"].isString() || !JsonRoot["facetype"].isInt() || 
        !JsonRoot["customface"].isString() || !JsonRoot["gender"].isInt() || 
        !JsonRoot["birthday"].isInt() || !JsonRoot["signature"].isString() || 
        !JsonRoot["address"].isString() || !JsonRoot["phonenumber"].isString() || 
        !JsonRoot["mail"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    UserInfo newuserinfo;
    newuserinfo.userid = m_userinfo.userid;
    newuserinfo.nickname = JsonRoot["nickname"].asString();
    newuserinfo.facetype = JsonRoot["facetype"].asInt();
    newuserinfo.customface = JsonRoot["customface"].asString();
    newuserinfo.gender = JsonRoot["gender"].asInt();
    newuserinfo.birthday = JsonRoot["birthday"].asInt();
    newuserinfo.signature = JsonRoot["signature"].asString();
    newuserinfo.address = JsonRoot["address"].asString();
    newuserinfo.phonenumber = JsonRoot["phonenumber"].asString();
    newuserinfo.mail = JsonRoot["mail"].asString();
    
    ostringstream retdata;
    ostringstream currentuserinfo;
    if (!Singleton<UserManager>::Instance().updateUserInfo(newuserinfo))
    {
        retdata << "{ \"code\": 104, \"msg\": \"update user info failed\" }";
    }
    else
    {
        /*
        { "code": 0, "msg" : "ok", "userid" : 2, "username" : "xxxx", 
         "nickname":"zzz", "facetype" : 26, "customface" : "", "gender" : 0, "birthday" : 19900101, 
         "signature" : "xxxx", "address": "", "phonenumber": "", "mail":""}
        */
        currentuserinfo << "\"userid\": " << m_userinfo.userid << ",\"username\":\"" << m_userinfo.username
                        << "\", \"nickname\":\"" << newuserinfo.nickname
                        << "\", \"facetype\": " << newuserinfo.facetype << ", \"customface\":\"" << newuserinfo.customface
                        << "\", \"gender\":" << newuserinfo.gender
                        << ", \"birthday\":" << newuserinfo.birthday << ", \"signature\":\"" << newuserinfo.signature
                        << "\", \"address\": \"" << newuserinfo.address
                        << "\", \"phonenumber\": \"" << newuserinfo.phonenumber << "\", \"mail\":\""
                        << newuserinfo.mail;
        retdata << "{\"code\": 0, \"msg\": \"ok\"," << currentuserinfo.str()  << "\"}";
    }

    //Ó¦´ð¿Í»§¶Ë
    Send(msg_type_updateuserinfo, m_seq, retdata.str());

    LOG_INFO << "Response to client: userid=" << m_userinfo.userid << ", cmd=msg_type_updateuserinfo, data=" << retdata.str();

    //¸øÆäËûÔÚÏßºÃÓÑÍÆËÍ¸öÈËÐÅÏ¢·¢Éú¸Ä±äÏûÏ¢
    std::list<UserInfo> friends;
    Singleton<UserManager>::Instance().getFriendInfoByUserId(m_userinfo.userid, friends);
    IMServer& imserver = Singleton<IMServer>::Instance();
    for (const auto& iter : friends)
    {
        //ÏÈ¿´Ä¿±êÓÃ»§ÊÇ·ñÔÚÏß
        std::list<std::shared_ptr<ClientSession>> targetSessions;
        imserver.GetSessionsByUserId(targetSessions, iter.userid);
        for (auto& iter2 : targetSessions)
        {
            if (iter2)
                iter2->SendUserStatusChangeMsg(m_userinfo.userid, 3);
        }
    }
}
//ÒÑÍê³É
void ClientSession::OnModifyPasswordResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["oldpassword"].isString() || !JsonRoot["newpassword"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    string oldpass = JsonRoot["oldpassword"].asString();
    string newPass = JsonRoot["newpassword"].asString();

    string retdata;
    UserInfo cachedUser;
    if (!Singleton<UserManager>::Instance().getUserInfoByUserId(m_userinfo.userid, cachedUser))
    {
        LOG_ERROR << "get userinfo error, userid: " << m_userinfo.userid << ", data: " << data << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (cachedUser.password != oldpass)
    {
        retdata = "{\"code\": 103, \"msg\": \"incorrect old password\"}";
    }
    else
    {       
        if (!Singleton<UserManager>::Instance().modifyUserPassword(m_userinfo.userid, newPass))
        {
            retdata = "{\"code\": 105, \"msg\": \"modify password error\"}";
            LOG_ERROR << "modify password error, userid: " << m_userinfo.userid << ", data: " << data << ", client: " << conn->peerAddress().toIpPort();
        }
        else
            retdata = "{\"code\": 0, \"msg\": \"ok\"}";
    }

    //Ó¦´ð¿Í»§¶Ë
    Send(msg_type_modifypassword, m_seq, retdata);

    LOG_INFO << "Response to client: userid=" << m_userinfo.userid << ", cmd=msg_type_modifypassword, data=" << data;
}
//ÒÑÍê³É
void ClientSession::OnCreateGroupResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["groupname"].isString())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    ostringstream retdata;
    string groupname = JsonRoot["groupname"].asString();
    int32_t groupid;
    if (!Singleton<UserManager>::Instance().addGroup(groupname.c_str(), m_userinfo.userid, groupid))
    {
        LOG_WARN << "Add group error, data: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        retdata << "{ \"code\": 106, \"msg\" : \"create group error\"}";
    }
    else
    {
        retdata << "{\"code\": 0, \"msg\": \"ok\", \"groupid\":" << groupid << ", \"groupname\": \"" << groupname << "\"}";
    }

    //´´½¨³É¹¦ÒÔºó¸ÃÓÃ»§×Ô¶¯¼ÓÈº
    if (!Singleton<UserManager>::Instance().joinGroup(m_userinfo.userid, groupid))
    {
        LOG_ERROR << "join in group, errordata: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }
    //Ó¦´ð¿Í»§¶Ë£¬½¨Èº³É¹¦
    Send(msg_type_creategroup, m_seq, retdata.str());

    LOG_INFO << "Response to client: userid=" << m_userinfo.userid << ", cmd=msg_type_creategroup, data=" << retdata.str();

    //Ó¦´ð¿Í»§¶Ë£¬³É¹¦¼ÓÈº
    {
        char szSelfData[256] = { 0 };
        snprintf(szSelfData, 256, "{\"userid\": %d, \"type\": 3, \"username\": \"%s\", \"accept\": 1}", groupid, groupname.c_str());
        Send(msg_type_operatefriend, m_seq, szSelfData, strlen(szSelfData));
        LOG_INFO << "Response to client: userid=" << m_userinfo.userid << ", cmd=msg_type_addfriend, data=" << szSelfData;
    }
}

void ClientSession::OnGetGroupMembersResponse(const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    //{"groupid": Èºid}
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["groupid"].isInt())
    {
        LOG_WARN << "invalid json: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    int32_t groupid = JsonRoot["groupid"].asInt();
    
    std::list<UserInfo> friends;
    Singleton<UserManager>::Instance().getGroupMembers(groupid, friends);
    string strUserInfo;
    int userOnline = 0;
    IMServer& imserver = Singleton<IMServer>::Instance();
    for (const auto& iter : friends)
    {
        userOnline = imserver.GetUserStatusByUserId(iter.userid);
        /*
        {"code": 0, "msg": "ok", "members":[{"userid": 1,"username":"qqq,
        "nickname":"qqq, "facetype": 0, "customface":"", "gender":0, "birthday":19900101,
        "signature":", "address": "", "phonenumber": "", "mail":", "clienttype": 1, "status":1"]}
        */
        ostringstream osSingleUserInfo;
        osSingleUserInfo << "{\"userid\": " << iter.userid << ", \"username\":\"" << iter.username << "\", \"nickname\":\"" << iter.nickname
            << "\", \"facetype\": " << iter.facetype << ", \"customface\":\"" << iter.customface << "\", \"gender\":" << iter.gender
            << ", \"birthday\":" << iter.birthday << ", \"signature\":\"" << iter.signature << "\", \"address\": \"" << iter.address
            << "\", \"phonenumber\": \"" << iter.phonenumber << "\", \"mail\":\"" << iter.mail << "\", \"clienttype\": 1, \"status\":"
            << userOnline << "}";

        strUserInfo += osSingleUserInfo.str();
        strUserInfo += ",";
    }
    //È¥µô×îºó¶àÓàµÄ¶ººÅ
    strUserInfo = strUserInfo.substr(0, strUserInfo.length() - 1);
    std::ostringstream os;
    os << "{\"code\": 0, \"msg\": \"ok\", \"groupid\": " << groupid << ", \"members\":[" << strUserInfo << "]}";
    Send(msg_type_getgroupmembers, m_seq, os.str());

    LOG_INFO << "Response to client: userid=" << m_userinfo.userid << ", cmd=msg_type_getgroupmembers, data=" << os.str();
}

void ClientSession::SendUserStatusChangeMsg(int32_t userid, int type, int status/* = 0*/)
{
    string data; 
    //ÓÃ»§ÉÏÏß
    if (type == 1)
    {
        int32_t clientType = Singleton<IMServer>::Instance().GetUserClientTypeByUserId(userid);
        char szData[32];
        memset(szData, 0, sizeof(szData));
        sprintf(szData, "{ \"type\": 1, \"onlinestatus\": %d, \"clienttype\": %d}", status, clientType);
        data = szData;
    }
    //ÓÃ»§ÏÂÏß
    else if (type == 2)
    {
        data = "{\"type\": 2, \"onlinestatus\": 0}";
    }
    //¸öÈËêÇ³Æ¡¢Í·Ïñ¡¢Ç©ÃûµÈÐÅÏ¢¸ü¸Ä
    else if (type == 3)
    {
        data = "{\"type\": 3}";
    }

    string outbuf;
    BinaryWriteStream writeStream(&outbuf);
    writeStream.WriteInt32(msg_type_userstatuschange);
    writeStream.WriteInt32(m_seq);
    writeStream.WriteString(data);
    writeStream.WriteInt32(userid);
    writeStream.Flush();

    Send(outbuf);

    LOG_INFO << "Send to client: userid=" << m_userinfo.userid << ", cmd=msg_type_userstatuschange, data=" << data;
}

void ClientSession::MakeSessionInvalid()
{
    m_userinfo.userid = 0;
}

bool ClientSession::IsSessionValid()
{
    return m_userinfo.userid > 0;
}

void ClientSession::OnChatResponse(int32_t targetid, const string& data, const shared_ptr<TcpConnection>& conn)
{
    string outbuf;
    BinaryWriteStream writeStream(&outbuf);
    writeStream.WriteInt32(msg_type_chat);
    writeStream.WriteInt32(m_seq);
    writeStream.WriteString(data);
    //ÏûÏ¢·¢ËÍÕß
    writeStream.WriteInt32(m_userinfo.userid);
    //ÏûÏ¢½ÓÊÜÕß
    writeStream.WriteInt32(targetid);
    writeStream.Flush();

    UserManager& userMgr = Singleton<UserManager>::Instance();
    //Ð´ÈëÏûÏ¢¼ÇÂ¼
    if (!userMgr.saveChatMsgToDb(m_userinfo.userid, targetid, data))
    {
        LOG_ERROR << "Write chat msg to db error, , senderid = " << m_userinfo.userid << ", targetid = " << targetid << ", chatmsg:" << data;
    }

    IMServer& imserver = Singleton<IMServer>::Instance();
    MsgCacheManager& msgCacheMgr = Singleton<MsgCacheManager>::Instance();
    //µ¥ÁÄÏûÏ¢
    if (targetid < GROUPID_BOUBDARY)
    {
        //ÏÈ¿´Ä¿±êÓÃ»§ÊÇ·ñÔÚÏß
        std::list<std::shared_ptr<ClientSession>> targetSessions;
        imserver.GetSessionsByUserId(targetSessions, targetid);
        //Ä¿±êÓÃ»§²»ÔÚÏß£¬»º´æÕâ¸öÏûÏ¢
        if (targetSessions.empty())
        {
            msgCacheMgr.AddChatMsgCache(targetid, outbuf);
        }
        else
        {
            for (auto& iter : targetSessions)
            {
                if (iter)
                    iter->Send(outbuf);
            }
        }
    }
    //ÈºÁÄÏûÏ¢
    else
    {       
        std::list<UserInfo> groupMembers;
       // userMgr.getFriendInfoByUserId(targetid, friends); 
        userMgr.getGroupMembers(targetid, groupMembers);
        string strUserInfo;
        bool userOnline = false;
        for (const auto& iter : groupMembers)
        {
            //ÅÅ³ýÈº³ÉÔ±ÖÐµÄ×Ô¼º
            if (iter.userid == m_userinfo.userid)
                continue;

            //ÏÈ¿´Ä¿±êÓÃ»§ÊÇ·ñÔÚÏß
            std::list<std::shared_ptr<ClientSession>> targetSessions;
            imserver.GetSessionsByUserId(targetSessions, iter.userid);
            //Ä¿±êÓÃ»§²»ÔÚÏß£¬»º´æÕâ¸öÏûÏ¢
            if (targetSessions.empty())
            {
                msgCacheMgr.AddChatMsgCache(iter.userid, outbuf);
                continue;
            }
            else
            {
                for (auto& iter2 : targetSessions)
                {
                    if (iter2)
                        iter2->Send(outbuf);
                }
            }
        }
    }
    
}

void ClientSession::OnMultiChatResponse(const std::string& targets, const std::string& data, const std::shared_ptr<TcpConnection>& conn)
{
    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(targets, JsonRoot))
    {
        LOG_ERROR << "invalid json: targets: " << targets  << "data: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    if (!JsonRoot["targets"].isArray())
    {
        LOG_ERROR << "invalid json: targets: " << targets << "data: " << data << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    for (uint32_t i = 0; i < JsonRoot["targets"].size(); ++i)
    {
        OnChatResponse(JsonRoot["targets"][i].asInt(), data, conn);
    }

    LOG_INFO << "Send to client: cmd=msg_type_multichat, targets: " << targets << "data : " << data << ", from userid : " << m_userinfo.userid << ", from client : " << conn->peerAddress().toIpPort();
}

void ClientSession::OnScreenshotResponse(int32_t targetid, const std::string& bmpHeader, const std::string& bmpData, const std::shared_ptr<TcpConnection>& conn)
{
    string outbuf;
    BinaryWriteStream writeStream(&outbuf);
    writeStream.WriteInt32(msg_type_screenshot);
    writeStream.WriteInt32(m_seq);
    std::string dummy;
    writeStream.WriteString(dummy);
    writeStream.WriteString(bmpHeader);
    writeStream.WriteString(bmpData);
    //ÏûÏ¢½ÓÊÜÕß
    writeStream.WriteInt32(targetid);
    writeStream.Flush();

    IMServer& imserver = Singleton<IMServer>::Instance();
    //µ¥ÁÄÏûÏ¢
    if (targetid >= GROUPID_BOUBDARY)
        return;

    std::list<std::shared_ptr<ClientSession>> targetSessions;
    imserver.GetSessionsByUserId(targetSessions, targetid);
    //ÏÈ¿´Ä¿±êÓÃ»§ÔÚÏß²Å×ª·¢
    if (!targetSessions.empty())
    {
        for (auto& iter : targetSessions)
        {
            if (iter)
                iter->Send(outbuf);
        }
    }

}

void ClientSession::OnUploadDeviceInfo(int32_t deviceid, int32_t classtype, int64_t uploadtime, const std::string& strDeviceInfo, const std::shared_ptr<TcpConnection>& conn)
{
    if (!Singleton<UserManager>::Instance().insertDeviceInfo(m_userinfo.userid, deviceid, classtype, uploadtime, strDeviceInfo))
    {
        LOG_ERROR << "InsertDeviceInfo failed, userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
        return;
    }

    string retData = "{ \"code\": 0, \"msg\" : \"ok\" }";
    Send(msg_type_uploaddeviceinfo, m_seq, retData);

    LOG_INFO << "Send to client: userid=" << m_userinfo.userid << ", cmd=msg_type_uploaddeviceinfo, data=" << retData << ", client: " << conn->peerAddress().toIpPort();;
}
//ÒÑÍê³É
void ClientSession::DeleteFriend(const std::shared_ptr<TcpConnection>& conn, int32_t friendid)
{
    int32_t smallerid = friendid;
    int32_t greaterid = m_userinfo.userid;
    if (smallerid > greaterid)
       {
           smallerid = m_userinfo.userid;
           greaterid = friendid;
       }
	if (friendid < GROUPID_BOUBDARY)  //É¾³ýºÃÓÑ
	{
        if (!Singleton<UserManager>::Instance().releaseFriendRelationship(smallerid, greaterid))
        {
          LOG_ERROR << "Delete friend error, friendid: " << friendid << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
          return;
        }

        UserInfo cachedUser;
        if (!Singleton<UserManager>::Instance().getUserInfoByUserId(friendid, cachedUser))
        {
           LOG_ERROR << "Delete friend - Get user error, friendid: " << friendid << ", userid: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
           return;
        }

        char szData[256] = { 0 };
        //·¢¸øÖ÷¶¯É¾³ýµÄÒ»·½
        //{"userid": 9, "type": 1, }        
        snprintf(szData, 256, "{\"userid\":%d, \"type\":5, \"username\": \"%s\"}", friendid, cachedUser.username.c_str());
        Send(msg_type_operatefriend, m_seq, szData, strlen(szData));

        LOG_INFO << "Send to client: userid=" << m_userinfo.userid << ", cmd=msg_type_operatefriend, data=" << szData;

        //·¢¸ø±»É¾³ýµÄÒ»·½
        //É¾³ýºÃÓÑÏûÏ¢
   
        //ÏÈ¿´Ä¿±êÓÃ»§ÊÇ·ñÔÚÏß
        std::list<std::shared_ptr<ClientSession>>targetSessions;
        Singleton<IMServer>::Instance().GetSessionsByUserId(targetSessions, friendid);
        //½ö¸øÔÚÏßÓÃ»§ÍÆËÍÕâ¸öÏûÏ¢
        if (!targetSessions.empty())
           {
               memset(szData, 0, sizeof(szData));
               snprintf(szData, 256, "{\"userid\":%d, \"type\":5, \"username\": \"%s\"}", m_userinfo.userid, m_userinfo.username.c_str());
               for (auto& iter : targetSessions)
               {
                   if (iter)
                       iter->Send(msg_type_operatefriend, m_seq, szData, strlen(szData));
               }

               LOG_INFO << "Send to client: userid=" << friendid << ", cmd=msg_type_operatefriend, data=" << szData;
            }
        return;
    }
    
	else  //ÍËÈºµÄ´¦Àí
	{
		if (!Singleton<UserManager>::Instance().removeGroupMember(smallerid, greaterid))
		{
			LOG_ERROR << "Delete groupMember error, groupId: " << greaterid << ", memberId: " << m_userinfo.userid << ", client: " << conn->peerAddress().toIpPort();
			return;
		}

		//ÍËÈºÏûÏ¢
		//¸øÆäËûÔÚÏßÈº³ÉÔ±ÍÆËÍÈºÐÅÏ¢·¢Éú±ä»¯µÄÏûÏ¢
		std::list<UserInfo> groupMembers;
		Singleton<UserManager>::Instance().getGroupMembers(friendid, groupMembers); 
		IMServer& imserver = Singleton<IMServer>::Instance();
		for (const auto& iter : groupMembers)
		{
			//ÏÈ¿´Ä¿±êÓÃ»§ÊÇ·ñÔÚÏß
			std::list<std::shared_ptr<ClientSession>> targetSessions;
			imserver.GetSessionsByUserId(targetSessions, iter.userid);
			if (!targetSessions.empty())
			{
				for (auto& iter2 : targetSessions)
				{
					if (iter2)
						iter2->SendUserStatusChangeMsg(friendid, 3);
				}
			}
		}
	}
}

void ClientSession::EnableHearbeatCheck()
{
    std::shared_ptr<TcpConnection> conn = GetConnectionPtr();
    if (conn)
    {
        //Ã¿ÈýÃëÖÓ¼ì²âÒ»ÏÂÊÇ·ñÓÐµôÏßÏÖÏó
        m_checkOnlineTimerId = conn->getLoop()->runEvery(5, std::bind(&ClientSession::CheckHeartbeat, this, conn));
    }
}

void ClientSession::DisableHeartbaetCheck()
{
    std::shared_ptr<TcpConnection> conn = GetConnectionPtr();
    if (conn)
    {
        LOG_INFO << "remove check online timerId, userid=" << m_userinfo.userid
                 << ", clientType=" << m_userinfo.clienttype
                 << ", client address: " << conn->peerAddress().toIpPort();
        conn->getLoop()->cancel(m_checkOnlineTimerId);
    }
}

void ClientSession::CheckHeartbeat(const std::shared_ptr<TcpConnection>& conn)
{
    if (!conn)
        return;
    
    //LOG_INFO << "check heartbeat, userid=" << m_userinfo.userid
    //        << ", clientType=" << m_userinfo.clienttype
    //        << ", client address: " << conn->peerAddress().toIpPort();

    if (time(NULL) - m_lastPackageTime < MAX_NO_PACKAGE_INTERVAL)
        return;
    
    conn->forceClose();
    LOG_INFO << "in max no-package time, no package, close the connection, userid=" << m_userinfo.userid 
             << ", clientType=" << m_userinfo.clienttype 
             << ", client address: " << conn->peerAddress().toIpPort();
}
