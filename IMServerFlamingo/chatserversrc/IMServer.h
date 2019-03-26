/** 
 *  �������������࣬IMServer.h
 **/
#ifndef CHATSERVERSRC_IMSERVER_H_
#define CHATSERVERSRC_IMSERVER_H_

#include <memory>
#include <list>
#include <map>
#include <mutex>
#include "../net/tcpserver.h"
#include "../net/eventloop.h"
#include "ClientSession.h"


using namespace net;
using std::string;

enum CLIENT_TYPE
{
    CLIENT_TYPE_UNKOWN,
    CLIENT_TYPE_PC,
    CLIENT_TYPE_ANDROID,
    CLIENT_TYPE_IOS,
    CLIENT_TYPE_MAC
};

struct StoredUserInfo
{
    int32_t    userid;
    string     username;
    string     password;
    string     nickname;
};

class IMServer final
{
public:
    IMServer() = default;    //��ʽ�ظ��߱��������ɸ�Ĭ�Ϻ�����
    ~IMServer() = default;

    IMServer(const IMServer& rhs) = delete;    //��ʽ�ظ��߱�������ֹ��Ĭ�Ϻ�����Ҳ��ͨ��privateʵ�֣���
    IMServer& operator =(const IMServer& rhs) = delete;

    bool Init(const char* ip, short port, EventLoop* loop);

    void GetSessions(std::list<std::shared_ptr<ClientSession>>& sessions);
    //�û�id��clienttype��Ψһȷ��һ��session
    bool GetSessionByUserIdAndClientType(std::shared_ptr<ClientSession>& session, int32_t userid, int32_t clientType);

    bool GetSessionsByUserId(std::list<std::shared_ptr<ClientSession>>& sessions, int32_t userid);

    //��ȡ�û�״̬�������û������ڣ��򷵻�0
    int32_t GetUserStatusByUserId(int32_t userid);
    //��ȡ�û��ͻ������ͣ�������û������ڣ��򷵻�0
    int32_t GetUserClientTypeByUserId(int32_t userid);

private:
    //�����ӵ������û����ӶϿ���������Ҫͨ��conn->connected()���жϣ�һ��ֻ����loop�������
    void OnConnection(std::shared_ptr<TcpConnection> conn);  
    //���ӶϿ�
    void OnClose(const std::shared_ptr<TcpConnection>& conn);
   

private:
	//ǰ��Ŀ�������͸�ֵ�����ѱ�delete������Ϊ�λ�shared_ptr������
    std::shared_ptr<TcpServer>                     m_server;	
    std::list<std::shared_ptr<ClientSession>>      m_sessions;
    std::mutex                                     m_sessionMutex;      //���߳�֮�䱣��m_sessions
    int                                            m_baseUserId{};
    std::mutex                                     m_idMutex;           //���߳�֮�䱣��m_baseUserId
};
#endif //CHATSERVERSRC_IMSERVER_H_
