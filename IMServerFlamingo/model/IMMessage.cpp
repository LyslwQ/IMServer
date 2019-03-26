/*
fileName: IMMessage.cpp 
*/
#include <iostream>
#include <string>

#include <odb/transaction.hxx>
#include "database.h"

#include "IMMessage.hxx"
#include "IMMessage-odb.hxx"

using namespace odb::core;

IMMessage::IMMessage(){}


/*
 @1. 以下四个函数均为静态函数。
*/


/*
 * @1.通过IMMessageId查找IMMessage实例，
 *    存在则返回实例，否则返回nullptr。
*/

/*std::auto_ptr<IMMessage>
IMMessage::getIMMessage(tid_t id)
{
  query<IMMessage> q =  (query<IMMessage>::tid == id);
  return  Model::query_one<IMMessage>(q);
}


std::auto_ptr<IMMessage> 
IMMessage::getIMMessage( const std::string& name) 
{
   query<IMMessage> q =  (query<IMMessage>::message_name == name);
   return  Model::query_one<IMMessage>(q); 
}


bool IMMessage::isExist(const std::string& name) 
{
  if( getIMMessage(name).get() == NULL) 
    return false; 
  else
    return true;
}
*/

std::auto_ptr<IMMessage> 
IMMessage::getIMMessage( const IMMessage& msg) 
{  
   query<IMMessage> q =  (query<IMMessage>::createTime == msg.createTime_ &&
                          query<IMMessage>::fromId == msg.fromId_ &&
                          query<IMMessage>::toId == msg.toId_);
   return Model::query_one<IMMessage>(q);
}


bool IMMessage::isExist() 
{
  if( getIMMessage(*this).get() == NULL) 
    return false; 
  else
    return true;
}


std::auto_ptr<std::list<IMMessage> > IMMessage::messageList_;

/*std::list<IMMessage>* IMMessage::allIMMessage()
{
  if( IMMessage::messageList_ != NULL )
    return IMMessage::messageList_;
  else
    {
      query<IMMessage> q;
      IMMessage::messageList_ = Model::query_all<IMMessage>(q).release();
      return IMMessage::messageList_;
    }
}
*/


/*
*@1.初始化IMMessage表，
*@2.从数据库中加载IMMessage做缓存。
*@3.是静态函数，在程序初始化阶段调用。
*/
void IMMessage::init()
{  
  Model::init_table<IMMessage>("IMMessage");
  query<IMMessage> q;
  IMMessage::messageList_ = Model::query_all<IMMessage>(q);
}

/*
保存IMMessage实例。 
@1.对message判断是否重复存在，无意义。
*/
bool IMMessage::save()
{
  // if(getIMMessage(*this).get() != NULL)
  //   {
 //      std::cout << *this << "  is existed..." << std::endl;
 //      return false;
 //    }
 //  else
  //   {
       Model::save(*this);
       return true;
  //   }
}

/*
 * @1.更新IMMessage记录。
 * @2.先检查数据库中是否有该实例，
 *    没有则保存。
 */
/*bool IMMessage::update()
{
  std::auto_ptr<IMMessage> u = getIMMessage(*this);
  if(u.get() == NULL) 
    {
      save();
      return true;
    } 
  if( Model::update(*u) )
    return true;
  else
    return false;
}*/
/*
 * 重载一个打印实例信息的函数。
 */
std::ostream& operator <<(std::ostream& cout, const IMMessage& u)
{
  std::cout << "the IMMessage as follow is saved:" << std::endl;
  std::cout << "tid_ = " << u.tid_ << std::endl;
  std::cout << "message_content_ = " << u.content_ << std::endl;
  return cout; 
}

