/**
 *  �������������࣬IMServer.cpp
 **/
#include "../net/inetaddress.h"
#include "../base/logging.h"
#include "../base/singleton.h"
#include "IMServer.h"
#include "ClientSession.h"
#include "UserManager.h"

using std::shared_ptr;
using std::lock_guard;

/*
 @1.��ʼ������������������
*/
bool IMServer::Init(const char* ip, short port, EventLoop* loop)
{   
    InetAddress addr(ip, port);
    m_server.reset(new TcpServer(loop, addr, "ZYL-MYIMSERVER", TcpServer::kReusePort));
	//ͨ��bind������һ���ɵ��ö��󣬿ɸ�ֵ��std::function����
    m_server->setConnectionCallback(std::bind(&IMServer::OnConnection, this, std::placeholders::_1));
    //��������
    m_server->start();

    return true;
}


/*
 @1.��������connection callback������
 @2.����������ʱ������øú�����
 @3.���������趨��MessageCallback --  ClientSession::OnRead��
*/
void IMServer::OnConnection(std::shared_ptr<TcpConnection> conn)
{
    if (conn->connected())
    {
        //LOG_INFO << "client connected:" << conn->peerAddress().toIpPort();
        ++ m_baseUserId;
        shared_ptr<ClientSession> spSession(new ClientSession(conn));
        conn->setMessageCallback(std::bind(&ClientSession::OnRead, spSession.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));       

        lock_guard<std::mutex> guard(m_sessionMutex);
        m_sessions.push_back(spSession);
    }
    else
    {
        OnClose(conn);
    }
}

void IMServer::OnClose(const std::shared_ptr<TcpConnection>& conn)
{
    //�Ƿ����û�����
    //bool bUserOffline = false;
    UserManager& userManager = Singleton<UserManager>::Instance();

    //TODO: �����Ĵ����߼�̫���ң���Ҫ�Ż�
    lock_guard<std::mutex> guard(m_sessionMutex);
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        if ((*iter)->GetConnectionPtr() == NULL)
        {
            LOG_ERROR << "connection is NULL";
            break;
        }
        
        //ͨ���ȶ�connection�����ҵ���Ӧ��session
        if ((*iter)->GetConnectionPtr() == conn)
        {
            //��Session����֮ǰ�������ߵ���ЧSession������Ϊ���������ߣ��Ÿ������������������Ϣ
            if ((*iter)->IsSessionValid())
            { 
                //���������ߺ��ѣ��������������������Ϣ
                std::list<UserInfo> friends;
                int32_t offlineUserId = (*iter)->GetUserId();
                userManager.getFriendInfoByUserId(offlineUserId, friends);
                for (const auto& iter2 : friends)
                {
                    for (auto& iter3 : m_sessions)
                    {
                        //�ú����Ƿ����ߣ����߻����session��
                        if (iter2.userid == iter3->GetUserId())
                        {
                            iter3->SendUserStatusChangeMsg(offlineUserId, 2);

                            LOG_INFO << "SendUserStatusChangeMsg to user(userid=" << iter3->GetUserId() << "): user go offline, offline userid = " << offlineUserId;
                        }
                    }
                }
            }
            else
            {
                LOG_INFO << "Session is invalid, userid=" << (*iter)->GetUserId();
            }
            
            //ͣ����Session�ĵ��߼��
            //(*iter)->DisableHeartbaetCheck();
            //�û�����
            m_sessions.erase(iter);
            //bUserOffline = true;
            LOG_INFO << "client disconnected: " << conn->peerAddress().toIpPort();
            break;
        }
    }

    LOG_INFO << "current online user count: " << m_sessions.size();
}

void IMServer::GetSessions(std::list<std::shared_ptr<ClientSession>>& sessions)
{
    lock_guard<std::mutex> guard(m_sessionMutex);
    sessions = m_sessions;
}

bool IMServer::GetSessionByUserIdAndClientType(std::shared_ptr<ClientSession>& session, int32_t userid, int32_t clientType)
{
    lock_guard<std::mutex> guard(m_sessionMutex);
    shared_ptr<ClientSession> tmpSession;
    for (const auto& iter : m_sessions)
    {
        tmpSession = iter;
        if (iter->GetUserId() == userid && iter->GetClientType() == clientType)
        {
            session = tmpSession;
            return true;
        }
    }

    return false;
}

bool IMServer::GetSessionsByUserId(std::list<std::shared_ptr<ClientSession>>& sessions, int32_t userid)
{
    lock_guard<std::mutex> guard(m_sessionMutex);
    shared_ptr<ClientSession> tmpSession;
    for (const auto& iter : m_sessions)
    {
        tmpSession = iter;
        if (iter->GetUserId() == userid)
        {
            sessions.push_back(tmpSession);
            return true;
        }
    }

    return false;
}

int32_t IMServer::GetUserStatusByUserId(int32_t userid)
{
    lock_guard<std::mutex> guard(m_sessionMutex);
    for (const auto& iter : m_sessions)
    {
        if (iter->GetUserId() == userid)
        {
            return iter->GetUserStatus();
        }
    }

    return 0;
}

int32_t IMServer::GetUserClientTypeByUserId(int32_t userid)
{
    lock_guard<std::mutex> guard(m_sessionMutex);
    bool bMobileOnline = false;
    int clientType = CLIENT_TYPE_UNKOWN;
    for (const auto& iter : m_sessions)
    {
        if (iter->GetUserId() == userid)
        {   
            clientType = iter->GetUserClientType();
            //��������ֱ�ӷ��ص�������״̬
            if (clientType == CLIENT_TYPE_PC)
                return clientType;
            else if (clientType == CLIENT_TYPE_ANDROID || clientType == CLIENT_TYPE_IOS)
                bMobileOnline = true;
        }
    }

    //ֻ���ֻ����߲ŷ����ֻ�����״̬
    if (bMobileOnline)
        return clientType;

    return CLIENT_TYPE_UNKOWN;
}
