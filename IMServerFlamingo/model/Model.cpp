#include <memory>
#include <iostream>
#include <string>

#include <odb/transaction.hxx>
#include <odb/schema-catalog.hxx>
#include "database.h"
#include "Model.h"
#include "User.hxx"
#include "User-odb.hxx"

using namespace odb::core;

odb::database* Model::db_ = NULL;

void Model::init( const char* db_user,
                  const char* db_passwd,
                  const char* db_name,
                  const char* host)
{ /*
   @1.创建数据库连接，
      初始化数据库。
   @2.todo:完善到从文件读取配置信息。
  */
  /* const char* user     = "--user";
   const char* passwd   = "--password";
   const char* database =  "--database";
   char* argv[]={
                 const_cast<char*>(user), 
                 const_cast<char*>(db_user.data()), 
                 const_cast<char*>(passwd), 
                 const_cast<char*>(db_passwd.data()), 
                 const_cast<char*>(database), 
                 const_cast<char*>(db_name.data())
                 };
  int argc = sizeof(argv)/sizeof(char*);
  db_ = create_database(argc , argiv);*/
  db_ = new odb::mysql::database(db_user, db_passwd, 
                                 db_name, host);
}


