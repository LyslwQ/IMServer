/*
 @1.fileName: Model.hxx. 
 */
#ifndef MODEL_H_
#define MODEL_H_

#include <cstdlib>
#include <memory>
#include <iostream>
#include <string>
#include <list>
#include <odb/database.hxx>
#include <odb/schema-catalog.hxx>
/*
 @1.fileName: model.hxx 

 @2.model 是一个基类，
    负责跟数据库有关的操作。
*/

using namespace odb::core;

//#pragma db object abstract 
class Model
{

public:
  Model(){}
  static void init(  const char* db_user,
                     const char* db_passwd,
                     const char* db_name,
                     const char* host);
 
  /*
   @1.初始化表格，确保表格在使用前是存在的。
   @2.todo：如何后期有更复杂的初始化业务，
            可以传进table对象。 
  */
  template<typename T>
  static void init_table(const char* table);

  inline static odb::database* getDB() { return db_;}

  /*
   @1.用以查询操作。
   @2.具体表信息，由template T获得。
   @3.查询语句由用户提供。
   @4.只适合 判断表中是否存在某条记录的场景，
      查到第一条匹配的记录时，则结束查询,并返回结果。
   */ 
  template<typename T>
  static std::auto_ptr<T> 
  query_one(query<T>& q); 


  template<typename T>
  static std::auto_ptr<std::list<T> >
  query_all(query<T>& q);
  /*
   @1.用以保存用户实例信息。
   @2.具体实例，由template T获得。
  */
  template<typename T>
  static void save(T& mod);

  /*
   @1.删除记录。
  */
  template<typename T>
  static void remove(T& mod);

  /*
   @1.更新数据库记录
  */
  template<typename T>
  static bool update(T& mod);
private:
  static odb::database* db_;
};



/*
 @1.初始化表格。
 @2.静态函数。 
*/
template<typename T>
void Model::init_table(const char* table)
{
   {
      transaction t (Model::getDB()->begin ());
      try
      {
        Model::getDB()->query<T> (false);
        //std::cout << table << " table  is existed..." << std::endl;
      }
      catch (const odb::exception& e)
      {
        schema_catalog::create_schema (*Model::getDB(), table, false);
        //std::cout << "create" << table <<"-table..." << std::endl;
      }
      t.commit ();
    }
}


  /*
   @1.找到一个匹配的item时，结束查找，立刻返回。
   @2.静态函数。
  */
  template<class T>
  std::auto_ptr<T>
  Model::query_one(query<T>& q)
  {
    //todo：加锁
    std::auto_ptr<T> u(new T());

    transaction t(Model::getDB()->begin());
    if( Model::getDB()->query_one<T> (q, *u))
       {
          t.commit();
          return u;
       }
    else
       {  
         t.commit();
         std::auto_ptr<T> uu(NULL);
         return uu;
       }
  }



/*
 @1.静态函数
 @2.返回表中所有匹配的记录。
 @3.todo：这里使用了动态内存分配，
          所以不可重入（非线程安全）？？？
 */
  template<class T>
  std::auto_ptr<std::list<T> >
  Model::query_all(query<T>& q)
  { 
    std::auto_ptr<std::list<T> > uList;

    transaction t(Model::getDB()->begin());
    result<T> r (Model::getDB()->query<T>(q)); 

   if(r.empty()) 
     {
       std::cout << "query no thing..." << std::endl;
       return uList;
     }
   else
     {
        uList.reset(new std::list<T>);
        typename result<T>::iterator i = r.begin();
        for (; i != r.end (); ++i)
          {
            //std::cout << "the query result " << std::endl;
            //std::cout << *i << std::endl;
            (*uList).push_back(*i);
          }
        return uList;
      }
    t.commit();
  }

  /*
   @1.静态函数。
   @2.保存实例
   */
  template<typename T>
  void Model::save(T& mod)
  {
    transaction t(Model::getDB()->begin());

    Model::getDB()->persist(mod);

    t.commit();
    //std::cout << "Model::save()--saved:\n" << mod << std::endl;
  }


  /*
   @1.静态函数。
   @2.删除实例。
  */
  template<typename T>
  void Model::remove(T& mod)
  {
    transaction t(Model::getDB()->begin());

    Model::getDB()->erase(mod);
    t.commit();
  }

  /*
   @1.静态函数。
   @2.更新实例。
   @3.todo--这些odb操作都未做异常处理。
            暂且用true处理。
  */
  template<typename T>
  bool Model::update(T& mod)
  {
    T uu;
    try{
        transaction t(Model::getDB()->begin());

        Model::getDB()->load( mod.getTid(), uu);
        uu = mod;
        Model::getDB()->update(uu);
        t.commit();
        return true;
       }
   catch (const odb::exception& e)
    {
      std::cout << e.what () << std::endl;
      return 1;
    }
  }
//#pragma db member(model::model_id) id
#endif   //model.h 
