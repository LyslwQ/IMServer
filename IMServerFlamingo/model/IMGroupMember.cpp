/*
fileName: IMGroupMember.cpp 
*/
#include <iostream>
#include <string>

#include <odb/transaction.hxx>
#include "database.h"

#include "IMGroupMember.hxx"
#include "IMGroupMember-odb.hxx"
#include "Relationship.hxx"

using namespace odb::core;



//std::auto_ptr<std::list<IMGroupMember> >
//map2List(std::map<std::string, IMGroupMember>* map);


//std::map<std::string, IMGroupMember>* 
//list2Map(std::auto_ptr<std::list<IMGroupMember> > list);


IMGroupMember::IMGroupMember(){}


/*
 * @1.更新IMGroupMember记录。
 * @2.先检查数据库中是否有该实例，
 *    没有则保存。
 */
bool IMGroupMember::save()
{
  std::auto_ptr<IMGroupMember> g = getIMGroupMember(*this);
  if(g.get() == NULL) 
    {
      Model::save(*this);
      return true;
    }
  tid_ = g->getTid();  //只有保存后才能获得id。
  if( Model::update(*this) )
    return true;
  else
    return false;
}



bool IMGroupMember::remove()
{
  std::auto_ptr<IMGroupMember> g = getIMGroupMember(*this);
  if(g.get() != NULL) 
    {
      Model::remove(*this);
      return true;
    }
  else
    return false;
}
/*
 * @1.static，在程序初始化阶段调用。 
 * @2.初始化IMGroupMember表
 * @3.加载数据库数据到缓存层。
 */
void IMGroupMember::init()
{  
  Model::init_table<IMGroupMember>("IMGroupMember");
  }



/*
 * @1.通过IMGroupMemberId查找IMGroupMember实例，
 *    存在则返回实例，否则返回nullptr。
*/

std::auto_ptr<std::list<tid_t> >
IMGroupMember::getIMGroupMembers(tid_t groupId)
{
  std::auto_ptr<std::list<IMGroupMember> > mlist;
  std::auto_ptr<std::list<tid_t> > ulist;
  query<IMGroupMember> q = (query<IMGroupMember>::group_id == groupId);
  mlist = Model::query_all<IMGroupMember>(q);
  if(mlist.get() != NULL)
    {
      ulist.reset(new std::list<tid_t>);
      std::list<IMGroupMember>::iterator it = mlist->begin();
      for(; it != mlist->end(); ++it)
          ulist->push_back(it->user_id_);
    }
  return ulist;
}

/*
std::auto_ptr<IMGroupMember> 
IMGroupMember::getIMGroupMember( const std::string& name) 
{
   query<IMGroupMember> q =  (query<IMGroupMember>::group_name == name);
   return  Model::query_one<IMGroupMember>(q); 
}

*/
/*bool IMGroupMember::isExist(const std::string& name) 
{
  if( getIMGroupMember(name).get() == NULL) 
    return false; 
  else
    return true;
}
*/

std::auto_ptr<IMGroupMember> 
IMGroupMember::getIMGroupMember( const IMGroupMember& group) 
{  
   query<IMGroupMember> q =  (query<IMGroupMember>::user_id == group.user_id_ 
                              && query<IMGroupMember>::group_id == group.group_id_);
   return Model::query_one<IMGroupMember>(q);
}




void IMGroupMember::getGroupsOfUser(tid_t userId, std::list<tid_t>& idList)
{
  std::auto_ptr<std::list<IMGroupMember> > list;
  query<IMGroupMember> q; 
  list = Model::query_all<IMGroupMember>(q);
  if(list.get() == NULL)
    return;
  std::list<IMGroupMember>::iterator it = list->begin();
  for(; it != list->end(); ++it)
    {
      if(it->getUserId() == userId)
        idList.push_back(it->getGroupId());
    }
  return;
}
/*
bool IMGroupMember::isExist() 
{
  if( getIMGroupMember(*this).get() == NULL) 
    return false; 
  else
    return true;
}

*/

//std::map<std::string, IMGroupMember>* IMGroupMember::groupContainer_ = NULL;

/*
 *@1.todo--可改用auto_ptr??? 
 */
/*std::auto_ptr<std::list<IMGroupMember> > 
IMGroupMember::allIMGroupMember()
{
 return map2List(groupContainer_);
}*/







/*
 *@1.private
 *@2.从odb层返回的数据结构是list，
 *   而缓存层为了便于搜索采用的是map，
 *   所以这里作为转换用。
 */
/*std::map<std::string, IMGroupMember>* 
list2Map(std::auto_ptr<std::list<IMGroupMember> > list)
{
  std::map<std::string, IMGroupMember>*   map(new std::map<std::string, IMGroupMember>);
  if( list.get() == NULL)
    return map;
  else
    {
      std::list<IMGroupMember>::iterator it = list->begin();
      for(; it != list->end(); ++it)
        map->insert(std::pair<std::string, IMGroupMember>(it->getName(), *it));
      return map;
    }
}
*/


/*
 *@1.private
 *@2.缓存层的数据是用map存储，
 *   这里将其转换为list返回。
 */
/*std::auto_ptr<std::list<IMGroupMember> >
map2List(std::map<std::string, IMGroupMember>* map)
{
  std::auto_ptr<std::list<IMGroupMember> > list;
  if(map == NULL)
    return list;
  else
    {
      list.reset(new std::list<IMGroupMember>);
      std::map<std::string, IMGroupMember>::iterator it = map->begin();
      for(; it != map->end(); ++it)
        list->push_back(it->second);
      return list;
    }
}
*/

/*
 * 重载一个打印实例信息的函数。
 */
std::ostream& operator <<(std::ostream& cout, IMGroupMember& g)
{
  std::cout << "the IMGroupMember as follow is saved:" << std::endl;
  std::cout << "tid_ = " << g.tid_ << std::endl;
  std::cout << "group_id_ = " << g.group_id_ << std::endl;
  std::cout << "user_id_ = " << g.user_id_ << std::endl;
  return cout; 
}
