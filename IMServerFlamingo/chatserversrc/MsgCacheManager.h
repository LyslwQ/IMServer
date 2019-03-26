/** 
 *  消息缓存类， MsgCacheManager.h
 **/
#ifndef CHATSERVERSRC_MSGCACHEMANAGER_H_
#define CHATSERVERSRC_MSGCACHEMANAGER_H_

#include <list>
#include <stdint.h>
#include <string>
#include <mutex>

using std::list;
using std::string;
using std::mutex;

struct NotifyMsgCache
{
    int32_t     userid;
    string notifymsg;  
};

struct ChatMsgCache
{
    int32_t     userid;
    string chatmsg;
};

class MsgCacheManager final
{
public:
    MsgCacheManager();
    ~MsgCacheManager();

    MsgCacheManager(const MsgCacheManager& rhs) = delete;
    MsgCacheManager& operator =(const MsgCacheManager& rhs) = delete;

    bool AddNotifyMsgCache(int32_t userid, const std::string& cache);
    void GetNotifyMsgCache(int32_t userid, std::list<NotifyMsgCache>& cached);

    bool AddChatMsgCache(int32_t userid, const std::string& cache);
    void GetChatMsgCache(int32_t userid, std::list<ChatMsgCache>& cached);


private:
    list<NotifyMsgCache>       m_listNotifyMsgCache;    //通知类消息缓存，比如加好友消息
    mutex                      m_mtNotifyMsgCache;
    list<ChatMsgCache>         m_listChatMsgCache;      //聊天消息缓存
    mutex                      m_mtChatMsgCache;
};
#endif //CHATSERVERSRC_MSGCACHEMANAGER_H_
