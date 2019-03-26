
#ifndef FILESERVERSRC_TCPSESSION_H_
#define FILESERVERSRC_TCPSESSION_H_

#include <memory>
#include "../net/tcpconnection.h"

using namespace net;

//Ϊ����ҵ�����߼��ֿ���ʵ��Ӧ������һ������̳���TcpSession����TcpSession��ֻ���߼����룬��������ҵ�����
class TcpSession
{
public:
    TcpSession(const std::weak_ptr<TcpConnection>& tmpconn);
    ~TcpSession();

    TcpSession(const TcpSession& rhs) = delete;
    TcpSession& operator =(const TcpSession& rhs) = delete;

    std::shared_ptr<TcpConnection> GetConnectionPtr()
    {
        if (tmpConn_.expired())
            return NULL;
        
        return tmpConn_.lock();
    }

    void Send(int32_t cmd, int32_t seq, int32_t errorcode, const std::string& filemd5, int64_t offset, int64_t filesize, const std::string& filedata);

private:
    //֧�ִ��ļ�����int64_t���洢�������ǵ�����һ���ļ��ϴ��������߼�
    void SendPackage(const char* body, int64_t bodylength);

protected:
    //TcpSession����TcpConnection���������ָ�룬��ΪTcpConnection���ܻ�����������Լ����٣���ʱTcpSessionӦ��ҲҪ����
    std::weak_ptr<TcpConnection>    tmpConn_;
};
#endif  //FILESERVERSRC_TCPSESSION_H_
