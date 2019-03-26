/*
 @1.fileName: IMGroupMember.hxx
*/ 


#ifndef MODEL_IMGROUPMEMBER_HXX
#define MODEL_IMGROUPMEMBER_HXX

#include <string>
#include <map>

#include <odb/core.hxx>

#include "Model.h"
#include "type.h"

class ostream;


#pragma db object
class IMGroupMember:public Model
{
public:
  IMGroupMember();
  IMGroupMember (tid_t user_id, tid_t group_id)
                :group_id_ (group_id), user_id_ (user_id){}


  /*
   @1.通过IMGroupMemberId查找IMGroupMember实例，
      存在则返回实例，否则返回nullptr。
  */
  static std::auto_ptr<std::list<tid_t> >
  getIMGroupMembers(tid_t groupId);
  /*
  todo：@1.查找表中是否存在该实例，
           不存在则返回nullptr.
  */
//  static std::auto_ptr<IMGroupMember>
//  getIMGroupMember( const std::string& name);
 
//  static bool isExist(const std::string& name); 
  
  static std::auto_ptr<IMGroupMember>
  getIMGroupMember(const IMGroupMember& IMGroupMember);
  
 // bool isExist();

  /*
  @1.有关IMGroupMember的初始化。
  */ 
  static void init();



  static void getGroupsOfUser(tid_t userId, std::list<tid_t>& idList);
  /*
  @1.数据库中加载所有用户.
  @2.todo--暂时用list做缓存，后期可靠redis。
  */
 // static std::auto_ptr<std::list<IMGroupMember> > 
 // allIMGroupMember();
  /*
  @1.保存IMGroupMember记录。
  */
  bool save();

  /*
   @1.更新IMGroupMember记录。
  */
  //bool update();

  bool remove(); 

  const unsigned long getTid() 
   { 
    return tid_;
   }

  const tid_t getUserId()
  {
    return user_id_;
  }
 
  const tid_t getGroupId() { return group_id_;}
 // inline const std::string getName() {return group_name_;}
//  inline std::string& name()  { return group_name_;}
  /*
  @1.用于打印IMGroupMember实例信息。
  */
  friend std::ostream& operator <<(std::ostream& cout, IMGroupMember& m);


private:
  //static void init_container();
  

  friend class odb::access;

  tid_t  tid_;       //group table的主键。
  tid_t  group_id_;
  tid_t  user_id_;
/*
 @1.缓存容器。
 @2.这里应该用share_ptr更合适，但odb编译器不支持。
 */
  // typedef std::map<std::string ,IMGroupMember> groupContainer_t;
  // static std::map<std::string ,IMGroupMember>* groupContainer_;  //做缓存用。	

//#pragma db transient
 };
#pragma db member(IMGroupMember::tid_) id auto

#endif // USER_HXX
