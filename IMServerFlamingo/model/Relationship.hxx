#ifndef RELATIONSHIP_H_
#define RELATIONSHIP_H_

/*
 *@1.fileName: Relationship.hxx.
 */
#include <map>

#include <odb/core.hxx>

#include "Model.h"
#include "type.h"
#pragma db object
class Relationship: public Model
{
public:
  Relationship(){}
  Relationship(tid_t userA_id, tid_t userB_id);
  Relationship(const Relationship& r);

  Relationship& operator =(const Relationship& r);
/*  
 *@1.有关Relationship表的初始化。
 */
  static void init();

/*
 *@1.保存Relationship记录。
 */
  bool save();

/*
 * @1.更新记录。
 */
  //bool update();

/*
 *@1.删除relaseship记录。
 */
  bool remove();

/*
 * @1.确认好友关系。
 */
//  bool confirm();
/*
 * @1.获取Relationship记录。
 * @2.函数中用到动态内存分配，
 *    返回值采用unique_ptr更合适（odb编译器不支持c++11）？
 */
  static std::auto_ptr<Relationship> 
  getRelationship(const Relationship& r);

/*
 * @1.获取跟user_id相关的relationship记录。
 * @2.todo：返回值改为set更合适？？？。
 * @3.函数中用到动态内存分配，
 *    返回值采用unique_ptr更合适（odb编译器不支持c++11）？
 */
  static std::auto_ptr<std::list<tid_t> >
  getRelationshipSet(tid_t user_id);


  inline tid_t getTid() { return tid_;}
  inline tid_t getSmallId() { return small_id_;}
  inline tid_t getBigId() {return big_id_;}
/*
 @1.用于打印Relationship实例信息。
 */
  friend std::ostream& operator <<(std::ostream& cout, Relationship& u);
 

private:
/*
 *@1.判断表中是否存在该实例。
 */
 // bool isExist(const Relationship& r);

/*
 *@1.进行成员赋值操作
 */
  void assign(tid_t tid, tid_t idA, 
	      tid_t idB, bool status); 


//public:
// static std::multimap<tid_t, Relationship>* relationshipContainer_; 

private:
  friend class odb::access;
  tid_t tid_; //主键 
  tid_t small_id_;       //外键
  tid_t big_id_;       //外键
  bool          status_ = false;    //只有对方同意，才为true。
/*
 @1.todo: 这里应该存储share_ptr更合适，但odb编译器不支持。
 @2.key为small_id
 */ 
 };
#pragma db member(Relationship::tid_) id auto

#endif //Relationship.hxx
