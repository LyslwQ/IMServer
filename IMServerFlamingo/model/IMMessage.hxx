/*
 @1.fileName: IMMessage.hxx
*/ 


#ifndef IMMESSAGE_HXX
#define IMMESSAGE_HXX

#include <string>
#include <list>

#include <odb/core.hxx>

#include "Model.h"
#include "type.h"
#include "structInfo.h"


class ostream;

#pragma db object
class IMMessage:public Model
{
public:
  IMMessage();
  IMMessage (tid_t fromId, tid_t toId, 
             const std::string& content)
  	     :fromId_(fromId), toId_(toId), content_(content) {}

  /*
   @1.通过IMMessageId查找IMMessage实例，
      存在则返回实例，否则返回nullptr。
  */
  static std::auto_ptr<IMMessage>
  getIMMessage(tid_t id);
  /*
  todo：@1.查找表中是否存在该实例，
           不存在则返回nullptr.
  */
  static std::auto_ptr<IMMessage>
  getIMMessage( const std::string& name);
 
  static bool isExist(const std::string& name); 
  
  std::auto_ptr<IMMessage>
  getIMMessage(const IMMessage& msg);
  
  bool isExist();

  /*
  @1.初始化IMMessage table：不存则创建。
  @2.从数据库中加载IMMessage做缓存。
  */ 
  static void init();
  /*
  @1.数据库中加载所有用户.
  @2.todo--暂时用list做缓存，后期可靠redis。
  */
  static std::list<IMMessage>* allIMMessage();
  /*
  @1.保存IMMessage记录。
  */
  bool save();
  /*
   @1.更新IMMessage记录。
  */
  bool update();

 // bool remove(); //暂且不实现。

  inline const unsigned long getTid() { return tid_;}
// inline const std::string getName() {return user_name_;}
// inline std::string& name()  { return user_name_;}
//  inline std::string& passwd() { return user_passwd_;} 
  /*
  @1.用于打印IMMessage实例信息。
  */
  friend std::ostream& operator <<(std::ostream& cout, const IMMessage& u);
public:
//#pragma db transient     //申明一个不固化的成员。
   static std::auto_ptr<std::list<IMMessage> >  messageList_;  //做缓存用。	

private:
  friend class odb::access;

  tid_t        tid_;         //IMMessage table的主键
  tid_t        relateId_;    //from和to两者的关系。(外键，relationship) ：暂未使用
  tid_t        fromId_;      //发送者Id
  tid_t        toId_;        //接受者Id
  ttime_t      createTime_;  //创建时间,当有多条信息时，可通过该字段区分。
  std::string  content_;     //消息内容，
//#pragma db transient
 };
#pragma db member(IMMessage::tid_) id auto


#endif // IMMESSAGE_HXX
