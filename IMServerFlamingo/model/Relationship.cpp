/*
 *@1.fileName: Friendship.cpp.
 */
#include "Relationship.hxx"
#include "Relationship-odb.hxx"
#include "Model.h"


/*
 * @1.内部转换函数，仅在该文件可见。
 */
std::multimap<tid_t, Relationship>*
list2Multimap(std::auto_ptr<std::list<Relationship> > list);


Relationship::Relationship(tid_t userA_id,
                           tid_t userB_id)
{
  assign(0, userA_id, userB_id, false);
}


/*
 * @1.对于复制构造函数，需注意的是，
 *    在有指针成员的情况下，需进行深拷贝。
 *    默认是浅拷贝？？？*/
Relationship::Relationship(const Relationship& r)
{
  if(&r == this)
    return;
  assign(r.tid_, r.small_id_, r.big_id_, r.status_);
}

Relationship& Relationship::operator =(const Relationship& r)
{
  if(&r == this)
    return *this;
  else
    {
      assign(r.tid_, r.small_id_, r.big_id_, r.status_);
      return *this;
    }
}


/*
 @1. 保存Friendship实例。
 */
bool Relationship::save()
{
  if( getRelationship(*this).get() != NULL )
      return false;
   Model::save(*this);
   return true;
}


/*
 *@1.删除relaseship记录。
 *@2.只是删除数据库中的记录，
 *   relationship对象的生命周期由其创建者决定。
 */
bool Relationship::remove()
{
  std::auto_ptr<Relationship> r = getRelationship(*this);
  if( r.get() == NULL)
    return false;
  Model::remove(*r);
  return true;
}


/*
 *@1.刚保存的实例处于false状态，
 *   只有对方同意之后才为true状态。
 */
/*bool Relationship::confirm()
{
  std::auto_ptr<Relationship> r = getRelationship(*this); 
  if( r.get() == NULL || r->status_ == true)
    { 
      std::cout << "confirm error..." << std::endl;
      return false;
     } 

  status_ = true;
  Model::update(*this);
  return true;
}*/



/*
 * @1.private
 * @2.判断表中是否存在该实例。  
 */
/*bool Relationship::isExist(const Relationship& r)
{
  if( getRelationship(r).get() != NULL )  
    return true;
  else
    return false;
}
*/

/*
 *@1.private
 *@2.进行成员赋值操作。
 */
void Relationship::assign(unsigned long tid, 
			  unsigned long idA, 
			  unsigned long idB, bool status)
{
  if(idA < idB)
    {
      small_id_ = idA;
      big_id_   = idB;
    }
  else
    {
      small_id_ = idB;
      big_id_   = idA;
    }

  tid_    = tid;
  status_ = status;
}


//std::multimap<tid_t, Relationship>* 
//Relationship::relationshipContainer_ = NULL;


/*
 @1.static.
 * @2.初始化Relationship数据表，
 *    从数据库中加载数据到缓存中。
 */
void Relationship::init()
{
  Model::init_table<Relationship>("Relationship");
  //query<Relationship> q;
 // Relationship::relationshipContainer_ = list2Multimap(Model::query_all(q));
}



/*
 *@1.static.
 *@2.获取user_id相关的记录。
 *@2.todo：功能还未真正实现。
 *@3.函数中用到动态内存分配，
 *   返回值采用unique_ptr更加合理。
 */
std::auto_ptr<std::list<tid_t> >
Relationship::getRelationshipSet(tid_t user_id)
{
  std::auto_ptr<std::list<tid_t> > list;
  query<Relationship> q = (query<Relationship>::small_id == user_id
                           || query<Relationship>::big_id == user_id);
  std::auto_ptr<std::list<Relationship> > rList = Model::query_all<Relationship>(q);
  if(rList.get() == NULL)
    return list;
  else
    {
      list.reset(new std::list<tid_t>);
      std::list<Relationship>::iterator it = rList->begin();
      for(; it != rList->end(); ++it)
        {
          if(it->small_id_ != user_id)
            list->push_back(it->small_id_);
          else
            list->push_back(it->big_id_);
        }
       return list; 
    }
 
   
  /*if(Relationship::relationshipContainer_ == NULL)
      return list;
  else
    {
      list.reset(new std::list<tid_t>);
      typedef std::multimap<tid_t, Relationship>::iterator iterator;
      iterator iit = relationshipContainer_->begin();
      //std::cout << "start test..." << std::endl;
      //for(; iit != relationshipContainer_->end(); ++iit)
      //  std::cout << "{" <<iit->first << "," <<  iit->second.small_id_ << "," << iit->second.big_id_ << "}" << std::endl;
      std::pair<iterator, iterator> range = Relationship::relationshipContainer_->equal_range(user_id);
      for(iterator it= range.first; it != range.second; ++it)
        {
         // std::cout << "{" << it->second.small_id_ << "," << it->second.big_id_ << "}" << std::endl;
          if(it->second.small_id_ != user_id)
            list->push_back(it->second.small_id_);
          else
            list->push_back(it->second.big_id_);
        }
      return list;
    }*/
}
/*
 * @1.static。
 * @2.函数中用到动态内存分配，
 *    采用unique_pr更加合理。(odb编译器不支持c++11)
 */
std::auto_ptr<Relationship> 
Relationship::getRelationship(const Relationship& r)
{
  query<Relationship> q = (query<Relationship>::small_id == r.small_id_
                           && query<Relationship>::big_id == r.big_id_);
  std::auto_ptr<Relationship> u;
  u =Model::query_one<Relationship>(q);
  return u;
}

/*
bool Relationship::update()
{
}*/
/*
 @1.重载一个打印实例信息的函数。
 */
std::ostream& operator <<(std::ostream& cout, Relationship& u)
{
 // std::cout << "the Friendship as follow is saved:" << std::endl;
  std::cout << "small_id_ = " << u.small_id_ << std::endl;
  std::cout << "big_id_ = " << u.big_id_ << std::endl;
  return cout;
}


std::multimap<tid_t, Relationship>*
list2Multimap(std::auto_ptr<std::list<Relationship> > list)
{
  //std::cout << "list2Map:this list is" << std::endl;
  std::list<Relationship>::iterator it = list->begin();
  for(; it != list->end(); ++it)
    std::cout << *it << std::endl;
  if( list.get() == NULL)
    { 
      std::cout << "list2Multimap(): the list is empty.." << std::endl;
      return NULL;
    }
  else
    {
      std::multimap<tid_t, Relationship>*  map(new std::multimap<tid_t, Relationship>);
      std::list<Relationship>::iterator it = list->begin();
      for(; it != list->end(); ++it)
        { 
          map->insert({it->getSmallId(), *it}); 
          map->insert({it->getBigId(), *it});
        }
        return map;
    }
}


