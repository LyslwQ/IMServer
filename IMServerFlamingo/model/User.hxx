/*
 @1.fileName: User.hxx
*/ 


#ifndef USER_HXX
#define USER_HXX

#include <string>
#include <list>

#include <odb/core.hxx>

#include "Model.h"
#include "structInfo.h"

class ostream;

typedef unsigned long tid_t;

#pragma db object
class User:public Model
{
public:
  User();
  User(const struct UserInfo& u);  
  User (const std::string& name,
        const std::string& passwd)
       :user_name_ (name), user_passwd_ (passwd){}
  
  User(const User& u);
  User& operator =(const User& u);

   
  //bool isExist();

  /*
  @1.保存User记录。
  */
  bool save();
  /*
  @1.更新User记录。
  */
  bool update();

  UserInfo getInfo();
  /*
  @1.有关User的初始化。
  */
  static void init();

  /*
  @1.返回所有的User记录.
  */
  static std::auto_ptr<std::list<User> > allUser();

  static std::auto_ptr<User>
  getUser(tid_t id);
 
  static std::auto_ptr<User>
  getUser( const std::string& name);
 
  static bool isExist(const std::string& name); 
  
  static std::auto_ptr<User>
  getUser(const User& user);
 
  static std::auto_ptr<std::list<User> >
  getUsers(std::list<tid_t>* list);

 // bool remove(); //暂且不实现。

  inline tid_t getTid() { return tid_;}
  inline std::string getName() {return user_name_;}
  inline std::string getPasswd() {return user_passwd_;}
  
 
  inline std::string& name()  { return user_name_;}
  inline std::string& passwd() { return user_passwd_;} 
  /*
  @1.用于打印User实例信息。
  */
  friend std::ostream& operator <<(std::ostream& cout, const User& u);


private:
/*
 *@1.进行成员赋值操作
 */
void assign(const std::string& name, const string& passwd,
            const std::string& nickName,
            const std::string& customFace, const std::string& customFaceFomt,
            const std::string& signature, const std::string& address,
            const std::string& phoneNum, const std::string& email,
            ttime_t registTime, const std::string& remark,
            const std::string& teamInfo, tid_t tid,
            int32_t faceType,  bool gender,
            ttime_t birthday);

//public:
//  static std::map<std::string, User>* userContainer_;
private:
  friend class odb::access;

  tid_t tid_;       //user table的主键。
  //unsigned long user_id_;   //作为其他表的外键，可用phone number充当。

  std::string  user_name_;
  std::string  user_passwd_;
  std::string  user_nickName_;
  int32_t      faceType_;      //用户头像类型
  std::string  customface_;    //自定义头像名
  std::string  customFaceFomt_;//自定义头像格式
  bool         gender_;        //性别
  ttime_t      birthday_;
  std::string  signature_;     //个性签名
  std::string  address_;
  std::string  phoneNum_;
  std::string  email_;  
  ttime_t      registTime_;
  std::string  remark_;         //备注
  ttime_t      updateTime_;
  std::string  teamInfo_;       //好友分组情况
//#pragma db transient
 };
#pragma db member(User::tid_) id auto


#endif // USER_HXX
