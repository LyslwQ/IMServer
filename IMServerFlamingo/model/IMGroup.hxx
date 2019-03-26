/*
 @1.fileName: IMGroup.hxx
*/ 


#ifndef MODEL_IMGROUP_HXX
#define MODEL_IMGROUP_HXX

#include <string>
#include <map>

#include <odb/core.hxx>

#include "Model.h"
#include "type.h"
#include "structInfo.h"
class ostream;


#pragma db object
class IMGroup:public Model
{
public:
  IMGroup();
  IMGroup (const std::string& name,
           tid_t creator_id, tid_t groupid)
          :group_name_ (name), creator_id_ (creator_id), tid_(groupid){}


  /*
   @1.通过IMGroupId查找IMGroup实例，
      存在则返回实例，否则返回nullptr。
  */
  static std::auto_ptr<IMGroup>
  getIMGroup(tid_t id);
  /*
  todo：@1.查找表中是否存在该实例，
           不存在则返回nullptr.
  */
//  static std::auto_ptr<IMGroup>
//  getIMGroup( const std::string& name);
 
//  static bool isExist(const std::string& name); 
  
  static std::auto_ptr<IMGroup>
  getIMGroup(const IMGroup& IMGroup);
 

  static tid_t getMaxGroupId();

  static void getGroups(std::list<tid_t>& idList, std::list<IMGroup>& groupList); 
  bool isExist();

  /*
  @1.有关IMGroup的初始化。
  */ 
  static void init();

  /*
  @1.数据库中加载所有用户.
  @2.todo--暂时用list做缓存，后期可靠redis。
  */
  static std::auto_ptr<std::list<IMGroup> > 
  allIMGroup();
  /*
  @1.保存IMGroup记录。
  */
  bool save();

  /*
   @1.更新IMGroup记录。
  */
  //bool update();

 // bool remove(); //暂且不实现。
 
  GroupInfo getInfo();
  inline const unsigned long getTid() { return tid_; }
  inline const std::string getName() {return group_name_;}
  inline std::string& name()  { return group_name_;}
  /*
  @1.用于打印IMGroup实例信息。
  */
  friend std::ostream& operator <<(std::ostream& cout, IMGroup& u);


private:
  //static void init_container();
  

  friend class odb::access;

  tid_t tid_;      
  tid_t groupId_;     //group table的主键(该表比较特殊)。
  tid_t creator_id_;
  std::string  group_name_;
/*
 @1.缓存容器。
 @2.这里应该用share_ptr更合适，但odb编译器不支持。
 */
   typedef std::map<std::string ,IMGroup> groupContainer_t;
   static std::map<std::string ,IMGroup>* groupContainer_;  //做缓存用。	

//#pragma db transient
 };
#pragma db member(IMGroup::groupId_) id auto

#endif // USER_HXX
