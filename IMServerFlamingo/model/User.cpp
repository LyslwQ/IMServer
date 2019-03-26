/*
fileName: User.cpp 
*/
#include <iostream>
#include <string>

#include <odb/transaction.hxx>
#include "database.h"

#include "User.hxx"
#include "User-odb.hxx"
#include "Relationship.hxx"

using namespace odb::core;

std::map<std::string, User>*
list2Map(std::auto_ptr<std::list<User> > list);

std::auto_ptr<std::list<User> >
map2List(std::map<std::string, User>* map);

User::User(){}

User::User(const struct UserInfo& u)
{
  assign(u.username, u.password, u.nickname, u.customface,
         u.customfacefmt, u.signature, u.address, u.phonenumber,
         u.mail, 20180808, "nothing", u.teaminfo, u.userid,
         u.facetype, u.gender, u.birthday);
}

User::User(const User& u)
{
   assign(u.user_name_, u.user_passwd_, u.user_nickName_, u.customface_,
          u.customFaceFomt_, u.signature_, u.address_, u.phoneNum_,
          u.email_, 20180808, "nothing", u.teamInfo_, u.tid_,
          u.faceType_, u.gender_, u.birthday_);
}


User& User::operator =(const User& u)
{
  if(this == &u)
    return *this;
  else
    {
      assign(u.user_name_, u.user_passwd_, u.user_nickName_, u.customface_,
             u.customFaceFomt_, u.signature_, u.address_, u.phoneNum_,
             u.email_, 20180808, "nothing", u.teamInfo_, u.tid_,
             u.faceType_, u.gender_, u.birthday_);
      return *this;
    }
}





/*
 *@1.进行成员赋值操作
 */
void User::assign(const std::string& name, const string& passwd,
            const std::string& nickName,
            const std::string& customFace, const std::string& customFaceFomt,
            const std::string& signature, const std::string& address,
            const std::string& phoneNum, const std::string& email,
            ttime_t registTime, const std::string& remark,
            const std::string& teamInfo, tid_t tid,
            int32_t faceType = 0,  bool gender = 0,
            ttime_t birthday = 1996)
{
  registTime_     = registTime;
  tid_            = tid;
  user_passwd_    = passwd;
  user_name_      = name;
  user_nickName_  = nickName;
  faceType_       = faceType;
  customface_     = customFace;
  customFaceFomt_ = customFaceFomt;
  gender_         = gender;
  birthday_       = birthday;
  signature_      = signature;
  address_        = address;
  phoneNum_       = phoneNum;
  email_          = email;
  teamInfo_       = teamInfo;
}

/*bool User::isExist(const std::string& name) 
{
  if( getUser(name).get() == NULL) 
    return false; 
  else
    return true;
}





bool User::isExist() 
{
  if( getUser(*this).get() == NULL) 
    return false; 
  else
    return true;
}
*/


/*
 * @1.更新User记录。
 * @2.先检查数据库中是否有该实例，
 *    没有则保存。
 */
bool User::update()
{
  std::auto_ptr<User> u = getUser(*this);
  if(u.get() == NULL) 
    {
      std::cout << "get nothing.." << std::endl;
      save();
      return true;
    }
  tid_ = u->getTid();
  if( Model::update(*this))
    return true;
  else
    return false;
}


//std::map<std::string, User>* User::userContainer_ = NULL;
/*
保存User实例。 
*/
bool User::save()
{
   if(getUser(*this).get() != NULL)
     {
       std::cout << user_name_ << " is existed..." << std::endl;
       *this = *(getUser(*this));  //保存实例后，重新从数据库加载，才有tid信息。
       return false;
     }
   else
     {
       Model::save(*this);
       *this = *(getUser(*this));  //保存实例后，重新从数据库加载，才有tid信息。
       return true;
     }
}



UserInfo User::getInfo()
{
  UserInfo u;    
  u.userid        = tid_;           
  u.password      = user_passwd_; 
  u.username      = user_name_;     
  u.nickname      = user_nickName_; 
  u.facetype      = faceType_;      
  u.customface    = customface_;    
  u.customfacefmt = customFaceFomt_;
  u.gender        = gender_;        
  u.birthday      = birthday_;      
  u.signature     = signature_;     
  u.address       = address_;       
  u.phonenumber   = phoneNum_;      
  u.mail          = email_;         
  u.teaminfo      = teamInfo_;
 
  return u; 
}
/*
@1.static. 在程序初始化阶段调用。
@2.初始化User表。
@3.从数据库中加载User做缓存。
*/
void User::init()
{  
  Model::init_table<User>("User");
 // query<User> q;
 // std::auto_ptr<std::list<User> > ulist = Model::query_all<User>(q);
 // User::userContainer_ = list2Map(ulist);
 // ulist.release();
}

/*
 * @1.static
 */
std::auto_ptr<User>
User::getUser(tid_t id)
{
  query<User> q =  (query<User>::tid == id);
  return  Model::query_one<User>(q);
}



/*
 * @1.static
 */
std::auto_ptr<User> 
User::getUser( const User& user) 
{  
   query<User> q =  (query<User>::user_name == user.user_name_);
   return Model::query_one<User>(q);
}



/*
 * @1.通过UserId查找User实例，
 *    存在则返回实例，否则返回nullptr。
 */

std::auto_ptr<User> 
User::getUser( const std::string& name) 
{
   query<User> q =  (query<User>::user_name == name);
   return  Model::query_one<User>(q); 
}



std::auto_ptr<std::list<User> >
User::getUsers(std::list<tid_t>* list)
{
  std::auto_ptr<std::list<User> > ulist;
  if(list == NULL)
    {
     std::cout << "getUsers(): this list is NULL" << std::endl;
     return ulist;
    }
  std::vector<tid_t> vec;
  std::list<tid_t>::iterator it = list->begin();
  for(; it != list->end(); ++it)
    {
     // std::cout << "getUsers(): " << *it << std::endl;
      vec.push_back(*it);
    }
  query<User> q = (query<User>::tid.in_range(vec.begin(), vec.end()));
  ulist = Model::query_all<User>(q);
  return ulist;
}




/*
 * @1.static
 * @2.换回所有的User记录，
 *    缓存层的数据结构是map，
 *    所以需要调用转换函数
 */
std::auto_ptr<std::list<User> >
User::allUser()
{
  //return map2List(userContainer_);
  query<User> q;
  return Model::query_all<User>(q);
}




/*
 * @1.private
 * @2.从odb层返回的数据结构是list，
 *    而缓存层为了便于搜索采用的是map，
 *    所以这里作为转换用。
 */
std::map<std::string, User>*
list2Map(std::auto_ptr<std::list<User> > list)
{
   if( list.get() == NULL)
    return NULL;
  else
    { 
      std::map<std::string, User>*  map(new std::map<std::string, User>);
      std::list<User>::iterator it = list->begin();
      for(; it != list->end(); ++it)
        map->insert(std::pair<std::string, User>(it->getName(), *it));
      return map;
    }
}


/*
 * @1.private
 * @2.缓存层的数据是用map存储，
 *    这里将其转换为list返回。
 */
std::auto_ptr<std::list<User> >
map2List(std::map<std::string, User>* map)
{
  std::auto_ptr<std::list<User> > list;
  if(map == NULL)
    return list;
  else
    {
      list.reset(new std::list<User>);
      std::map<std::string, User>::iterator it = map->begin();
      for(; it != map->end(); ++it)
        list->push_back(it->second);
      return list;
    }
}


/*
 * 重载一个打印实例信息的函数。
 */
std::ostream& operator <<(std::ostream& cout, const User& u)
{
  std::cout << "the User infomation is:" << std::endl;
  std::cout << "tid_ = " << u.tid_ << std::endl;
  std::cout << "user_passwd_ = " << u.user_passwd_ << std::endl;
  return cout; 
}
