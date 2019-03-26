/* 
 * @1.fileName: UserManager.h
 * @2.�������е��û���Ϣ����ʼ��Ϣ�����ݿ��м���,   
 */

#ifndef CHATSERVERSRC_USERMANAGER_H_
#define CHATSERVERSRC_USERMANAGER_H_

#include <stdint.h>
#include <string>
#include <list>
#include <mutex>
#include <set>

#include "../model/structInfo.h"

using std::string;
using std::set;
using std::list;

#define GROUPID_BOUBDARY   0x0FFFFFFF 

//�û�����Ⱥ
/*struct UserInfo
{
    int32_t        userid;      //0x0FFFFFFF������Ⱥ�ţ���������ͨ�û�
    string         username;    //Ⱥ�˻���usernameҲ��Ⱥ��userid���ַ�����ʽ
    string         password;
    string         nickname;    //Ⱥ�˺�ΪȺ����
    int32_t        facetype;
    string         customface;
    string         customfacefmt;//�Զ���ͷ���ʽ
    int32_t        gender;
    int32_t        birthday;
    string         signature;
    string         address;
    string         phonenumber;
    string         mail;
	string         teaminfo;
	int32_t        ownerid;        //����Ⱥ�˺ţ�ΪȺ��userid
	set<int32_t>   friends;        //Ϊ�˱����ظ�
    /*
    �����û����ѷ�����Ϣ������Ⱥ�˻���Ϊ�գ�����:
    [
    {
        "teamindex": 0,
        "teamname": "�ҵĺ���",
        "members": [
            {
                "userid": 1,
                "markname": "��ĳĳ"
            },
            {
                "userid": 2,
                "markname": "��xx"
            }
        ]
    },
    {
        "teamindex": 1,
        "teamname": "�ҵ�����",
        "members": [
            {
                "userid": 3,
                "markname": "��ĳĳ"
            },
            {
                "userid": 4,
                "markname": "��xx"
            }
        ]
    }
]
   
};
*/

//final���ε��಻�ܱ��̳�
class UserManager final
{
public:
    UserManager();
    ~UserManager();

    bool Init(const char* dbServer, const char* dbUserName, const char* dbPassword, const char* dbName);

    UserManager(const UserManager& rhs) = delete;
    UserManager& operator=(const UserManager& rhs) = delete;

    bool addUser(UserInfo& u);
    bool makeFriendRelationship(int32_t smallUserid, int32_t greaterUserid);
    bool releaseFriendRelationship(int32_t smallUserid, int32_t greaterUserid);
    bool removeGroupMember(int32_t smallUserid, int32_t greaterUserid);
    bool getGroupMembers(int32_t groupId, list<UserInfo>& groupMembers);
    bool addFriendToUser(int32_t userid, int32_t friendid);
    bool deleteFriendToUser(int32_t userid, int32_t friendid);
    bool updateUserInfo(const UserInfo& newuserinfo);
    bool modifyUserPassword(int32_t userid, const std::string& newpassword);

    bool addGroup(const char* groupname, int32_t ownerid, int32_t& groupid);
	bool joinGroup(int32_t userid, int32_t groupid);

    bool insertDeviceInfo(int32_t userid, int32_t deviceid, int32_t classtype, int64_t uploadtime, const std::string& deviceinfo);

    //������Ϣ���
    bool saveChatMsgToDb(int32_t senderid, int32_t targetid, const std::string& chatmsg);

    //TODO: ���û�Խ��Խ�࣬������Խ��Խ���ʱ�����ϵ�еĺ���Ч�ʸ���
    bool getUserInfoByUsername(const std::string& username, UserInfo& u);
    bool getUserInfoByUserId(int32_t userid, UserInfo& u);
    bool getFriendInfoByUserId(int32_t userid, std::list<UserInfo>& friends);
    bool getGroupsInfoByUserId(int32_t userid, std::list<GroupInfo>& groupsInfo);
    bool getTeamInfoByUserId(int32_t userid, std::string& teaminfo);
    bool getGroupInfoByGroupId(int32_t groupId, GroupInfo& group);
private:
    bool LoadUsersFromDb();
    bool LoadRelationshipFromDb(int32_t userid, std::set<int32_t>& r);

private:
    int                 m_baseUserId{ 0 };         //m_baseUserId, ȡ���ݿ�����userid���ֵ�������û�����������ϵ���
    int                 m_baseGroupId{0x0FFFFFFF}; //�����ų�ʼ����c++11����
    list<UserInfo>      m_allCachedUsers;         // �û���Ϣ������������ ��list��������ҡ�ToDo����Ϊmap�������redis��
    std::mutex          m_mutex;

    string              m_strDbServer;
    string              m_strDbUserName;
    string              m_strDbPassword;
    string              m_strDbName;
};
#endif //CHATSERVERSRC_USERMANAGER_H_
