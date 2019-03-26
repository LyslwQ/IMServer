// file      : schema/embedded/database.hxx
// copyright : not copyrighted - public domain

//
// Create concrete database instance based on the DATABASE_* macros.
//

#ifndef DATABASE_HXX
#define DATABASE_HXX

#include <string>
#include <memory>   // std::auto_ptr
#include <cstdlib>  // std::exit
#include <iostream>

#include <odb/database.hxx>

#include <odb/mysql/database.hxx>

inline odb::database*
create_database (int argc, char* argv[])
{
  using namespace std;
  using namespace odb::core;

  if (argc > 1 && argv[1] == string ("--help"))
  {
    cout << "Usage: " << argv[0] << " [options]" << endl
         << "Options:" << endl;

    odb::mysql::database::print_usage (cout);
    exit (0);
  }

  odb::database* db = new odb::mysql::database (argc, argv);
  return db;
}

#endif // DATABASE_HX
