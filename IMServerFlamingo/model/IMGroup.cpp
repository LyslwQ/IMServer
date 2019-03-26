/*
fileName: IMGroup.cpp 
*/
#include <iostream>
#include <string>

#include <odb/transaction.hxx>
#include "database.h"

#include "IMGroup.hxx"
#include "IMGroup-odb.hxx"
#include "Relationship.hxx"

using namespace odb::core;



std::auto_ptr<std::list<IMGroup> >
map2List(std::map<std::string, IMGroup>* map);


std::map<std::string, IMGroup>* 
list2Map(std::auto_ptr<std::list<IMGroup> > list);


IMGroup::IMGroup(){}

/*
保存IMGroup实例。 
*/
/*
bool IMGroup::save()
{
   if(getIMGroup(*this).get() != NULL)
     {
       std::cout << group_name_ << " is existed..." << std::endl;
       return false;
     }
   Model::save(*this)
   return true;
}
*/

/*
 * @1.更新IMGroup记录。
 * @2.先检查数据库中是否有该实例，
 *    没有则保存。
 */
bool IMGroup::save()
{
  std::auto_ptr<IMGroup> g = getIMGroup(*this);
  if(g.get() == NULL) 
    {
      Model::save(*this);
      return true;
    }
  tid_ = g->getTid(); 
  if( Model::update(*this) )
    return true;
  else
    return false;
}



/*
 * @1.static，在程序初始化阶段调用。 
 * @2.初始化IMGroup表
 * @3.加载数据库数据到缓存层。
 */
void IMGroup::init()
{  
  Model::init_table<IMGroup>("IMGroup");
  query<IMGroup> q;
  groupContainer_ = list2Map(Model::query_all<IMGroup>(q));
}



/*
 * @1.通过IMGroupId查找IMGroup实例，
 *    存在则返回实例，否则返回nullptr。
*/

std::auto_ptr<IMGroup>
IMGroup::getIMGroup(tid_t id)
{
  query<IMGroup> q =  (query<IMGroup>::tid == id);
  return  Model::query_one<IMGroup>(q);
}

/*
std::auto_ptr<IMGroup> 
IMGroup::getIMGroup( const std::string& name) 
{
   query<IMGroup> q =  (query<IMGroup>::group_name == name);
   return  Model::query_one<IMGroup>(q); 
}

*/
/*bool IMGroup::isExist(const std::string& name) 
{
  if( getIMGroup(name).get() == NULL) 
    return false; 
  else
    return true;
}
*/

std::auto_ptr<IMGroup> 
IMGroup::getIMGroup( const IMGroup& group) 
{  
   query<IMGroup> q =  (query<IMGroup>::group_name == group.group_name_);
   return Model::query_one<IMGroup>(q);
}



void IMGroup::getGroups(std::list<tid_t>& idList, std::list<IMGroup>& groupList)
{
  std::auto_ptr<std::list<IMGroup> > glist;
  std::vector<tid_t> vec;
  std::list<tid_t>::iterator it = idList.begin();
  for(; it != idList.end(); ++it)
    {
     // std::cout << "getUsers(): " << *it << std::endl;
        vec.push_back(*it);
    }
    query<IMGroup> q = (query<IMGroup>::tid.in_range(vec.begin(), vec.end()));
    glist = Model::query_all<IMGroup>(q);
    if(glist.get() != NULL)
      {
        std::list<IMGroup>::iterator it01 = glist->begin();
        for(; it01 != glist->end(); ++it01)
          groupList.push_back(*it01);
      }
    return;
}


tid_t IMGroup::getMaxGroupId()
{
   query<IMGroup> q;
   tid_t max = 0;
   std::auto_ptr<std::list<IMGroup> > list = Model::query_all<IMGroup>(q);
   if( list.get() != NULL)
     {
       std::list<IMGroup>::iterator it = list->begin();
       for(; it != list->end(); ++it)
         {
           if(it->tid_ > max)
             max = it->tid_;
         }
     }
  return max;
}


GroupInfo IMGroup::getInfo()
{
  GroupInfo g;
  g.tid = tid_;
  g.creatorId = creator_id_;
  g.groupName = group_name_;
  return g;
}

/*
bool IMGroup::isExist() 
{
  if( getIMGroup(*this).get() == NULL) 
    return false; 
  else
    return true;
}

*/

std::map<std::string, IMGroup>* IMGroup::groupContainer_ = NULL;

/*
 *@1.todo--可改用auto_ptr??? 
 */
std::auto_ptr<std::list<IMGroup> > 
IMGroup::allIMGroup()
{
 return map2List(groupContainer_);
}







/*
 *@1.private
 *@2.从odb层返回的数据结构是list，
 *   而缓存层为了便于搜索采用的是map，
 *   所以这里作为转换用。
 */
std::map<std::string, IMGroup>* 
list2Map(std::auto_ptr<std::list<IMGroup> > list)
{
  std::map<std::string, IMGroup>*   map(new std::map<std::string, IMGroup>);
  if( list.get() == NULL)
    return map;
  else
    {
      std::list<IMGroup>::iterator it = list->begin();
      for(; it != list->end(); ++it)
        map->insert(std::pair<std::string, IMGroup>(it->getName(), *it));
      return map;
    }
}



/*
 *@1.private
 *@2.缓存层的数据是用map存储，
 *   这里将其转换为list返回。
 */
std::auto_ptr<std::list<IMGroup> >
map2List(std::map<std::string, IMGroup>* map)
{
  std::auto_ptr<std::list<IMGroup> > list;
  if(map == NULL)
    return list;
  else
    {
      list.reset(new std::list<IMGroup>);
      std::map<std::string, IMGroup>::iterator it = map->begin();
      for(; it != map->end(); ++it)
        list->push_back(it->second);
      return list;
    }
}


/*
 * 重载一个打印实例信息的函数。
 */
std::ostream& operator <<(std::ostream& cout, IMGroup& u)
{
  std::cout << "the IMGroup as follow is saved:" << std::endl;
  std::cout << "tid_ = " << u.tid_ << std::endl;
  std::cout << "group_name_ = " << u.group_name_ << std::endl;
  std::cout << "creator_id_ = " << u.creator_id_ << std::endl;
  return cout; 
}
