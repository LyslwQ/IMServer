/*
 *  管理所有的用户信息，初始信息从数据库中加载, UserManager.h
 */
#include <memory>
#include <sstream>
#include <stdio.h>
#include "../database/databasemysql.h"
#include "../base/logging.h"
#include "UserManager.h"
#include "../model/Model.h"
#include "../model/Relationship.hxx"
#include "../model/IMGroup.hxx"
#include "../model/IMGroupMember.hxx"
#include "../model/IMMessage.hxx"
#include "../model/type.h"
#include "../model/User.hxx"



using std::list;

UserManager::UserManager()
{
    
}

UserManager::~UserManager()
{

}

bool UserManager::Init(const char* dbServer, const char* dbUserName, const char* dbPassword, const char* dbName)
{
  tid_t max = IMGroup::getMaxGroupId();
  if( max != 0)
    m_baseGroupId = max;    
  std::cout << "the max group id is..... " << m_baseGroupId << std::endl; 
  return true;
}
 
//已完成
bool UserManager::addUser(UserInfo& info)
{
	User u(info);   //通过传进一个结构体，构造一个User实例。
	if (!u.save())
	{
		LOG_INFO << "addUser error: the user as follow:";
		cout << u << endl;
		return false;
	}
	return true;
}
//已完成
bool UserManager::makeFriendRelationship(int32_t smallUserid, int32_t greaterUserid)
{
	Relationship r(smallUserid, greaterUserid);
	if (!r.save())
	{
		LOG_INFO << "makeRelationship error: the relationship as follow:";
		cout << r << endl;
		return false;
	}
    return true;
}
//已完成
bool UserManager::removeGroupMember(int32_t userId, int32_t groupId)
{
	IMGroupMember m(groupId, userId);
	if (!m.remove())
	{
		LOG_INFO << "removeGroupMember error:";
		cout << m << endl;
		return false;
	}
	else
		true;
}



bool UserManager::getGroupMembers(int32_t groupId, list<UserInfo>& groupMembers)
{
  std::auto_ptr<std::list<tid_t> > list;
  std::auto_ptr<std::list<User> > ulist;
  list = IMGroupMember::getIMGroupMembers(groupId);
  if (list.get() != NULL)
  {
    ulist = User::getUsers(list.get());
    if (ulist.get() != NULL)
    {
      std::list<User>::iterator it = ulist->begin();
      for (; it != ulist->end(); ++it)
        groupMembers.push_back(it->getInfo());
	return true;
    }
  }
  return false;
}



//已完成
bool UserManager::releaseFriendRelationship(int32_t smallUserid, int32_t greaterUserid)
{
	Relationship r(smallUserid, greaterUserid);
	if (!r.remove())
	{
		LOG_INFO << "removeRelationship error: the relationship as follow:";
		cout << r << endl;
		return false;
	}
	return true;
}


/*
 @1.输入：待更新的UserInfo结构体。
*/
//已完成
bool UserManager::updateUserInfo( const UserInfo& info)
{
	User u(info);
	if (!u.update())
	{
		LOG_INFO << "update user error: the user as follow:" ;
                cout <<u << endl;
		return false;
	}
	return true;
}

//已完成
bool UserManager::modifyUserPassword(int32_t userid, const std::string& newpasswd)
{
	auto_ptr<User> u = User::getUser(userid);
	if (u.get() == NULL)
	{
		LOG_FATAL << "The user is not existed...";
		return false;
	}
	u->passwd() = newpasswd;
	if( !u->update() )
	{
		LOG_FATAL << "The user update error...";
		return false;
	}
	return true;
}


//已完成
bool UserManager::addGroup(const char* groupname, int32_t ownerid, int32_t& groupid)
{
	IMGroup g(groupname, ownerid, ++m_baseGroupId);
        std::cout << "create a group... id = " << m_baseGroupId << std::endl;
	if (!g.save())
	{
		LOG_FATAL << "addGroup error: the group as follow:";
		cout << g.getName() << endl;
		return false;
	}

	groupid = g.getTid();
    return true;
}
//已完成
bool UserManager::joinGroup(int32_t userid, int32_t groupid)
{
	IMGroupMember m(userid, groupid);
	if (m.save())
		return true;
	else
		return false;
}

//未做更改
bool UserManager::insertDeviceInfo(int32_t userid, int32_t deviceid, int32_t classtype, int64_t uploadtime, const std::string& deviceinfo)
{
    std::unique_ptr<CDatabaseMysql> pConn;
    pConn.reset(new CDatabaseMysql());
    if (!pConn->Initialize(m_strDbServer, m_strDbUserName, m_strDbPassword, m_strDbName))
    {
        LOG_FATAL << "UserManager::InsertDeviceInfo failed, please check params: dbserver=" << m_strDbServer
            << ", dbusername=" << m_strDbUserName << ", dbpassword" << m_strDbPassword
            << ", dbname=" << m_strDbName;
        return false;
    }

    if (uploadtime <= 0)
        uploadtime = time(NULL);

    std::ostringstream osSql;
    osSql << "INSERT INTO t_device(f_user_id, f_deviceid, f_classtype, f_deviceinfo, f_upload_time, f_create_time) VALUES("
          << userid << ", "
          << deviceid << ", "
          << classtype << ", '"
          << deviceinfo << "',  FROM_UNIXTIME("
          << uploadtime << ", '%Y-%m-%d %H:%i:%S'), "
          << "NOW())";
    if (!pConn->Execute(osSql.str().c_str()))
    {
        LOG_WARN << "insert group error, sql=" << osSql.str();
        return false;
    }

    return true;
}

//已完成
bool UserManager::saveChatMsgToDb(int32_t fromId, int32_t toId, const std::string& content)
{
	IMMessage m(fromId, toId, content);
	if( m.save() )
		return true;
	else
	{
		LOG_FATAL << "saveChatMsgToDb error: the ChatMsg as follow:\n";
	        cout << m << endl;
		return false;
	}
}

//已完成
bool UserManager::getUserInfoByUsername(const std::string& username, UserInfo& u)
{
	auto_ptr<User> pu = User::getUser(username);
	if (pu.get() == NULL)
		return false;
	else
	{
		u = pu->getInfo();
		return true;
	}
}
//已完成
bool UserManager::getUserInfoByUserId(int32_t userid, UserInfo& u)
{
	auto_ptr<User> pu = User::getUser(userid);
	if (pu.get() == NULL)
		return false;
	else
	{
		u = pu->getInfo();
		return true;
	}
}



bool UserManager::getGroupsInfoByUserId(int32_t userid, std::list<GroupInfo>& groupsInfo)
{
  std::list<tid_t> idList;
  std::cout << "start get the list" << std::endl;
  IMGroupMember::getGroupsOfUser(userid, idList);
  std::cout << "hhh......get the list" << std::endl; 
  if ( idList.empty() )
     return true;
  std::list<IMGroup> groupList;
  IMGroup::getGroups(idList, groupList);
  std::list<IMGroup>::iterator it = groupList.begin();
  for(; it != groupList.end(); ++it)
    {
      groupsInfo.push_back(it->getInfo());
    }
  return true;
}

bool UserManager::getGroupInfoByGroupId(int32_t groupId, GroupInfo& group)
{
  std::auto_ptr<IMGroup> g = IMGroup::getIMGroup(groupId);
  if(g.get() != NULL)
    {
      group = g->getInfo();
      return true;
    }
  else
    return false;
}

/*
 @1.通过getRelationshipSet获得好友id
 @2.通过id获得User信息。
 */
//已完成
bool UserManager::getFriendInfoByUserId(int32_t userid, std::list<UserInfo>& friends)
{
	auto_ptr<list<tid_t> >  idList = Relationship::getRelationshipSet(userid);
	auto_ptr<list<User>> fList = User::getUsers(idList.release());
	if ( fList.get() == NULL)
		return false;
	else
	{
		list<User>::iterator it = fList->begin();
		for (; it != fList->end(); ++it)
			friends.push_back( it->getInfo() );
		return true;
	}
}

bool UserManager::getTeamInfoByUserId(int32_t userid, std::string& teaminfo)
{
    std::set<int32_t> friendsId;
    std::lock_guard<std::mutex> guard(m_mutex);
    for (const auto& iter : m_allCachedUsers)
    {
        if (iter.userid == userid)
        {
            teaminfo = iter.teaminfo;
            return true;
        }
    } 
    return false;
}
