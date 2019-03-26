#ifndef SQL_SQLDATABASE_H_
#define SQL_SQLDATABASE_H_

#include <string>
#include <list>

using std::string;

class SqlDriver;

typedef std::list<string> ConnectionList;

const string defaultConnection = "defaultConnection";

class SqlDatabase
{

public:
	SqlDatabase();
	/*
	创建并返回一个新连接。
	？？？：为什么不直接在构造函数中完成就好？
	！！！：因为该类会维护一张数据库连接表（static成员），
	        创建的每个SqlDatabase实例将会被加到该表中，
			所以用static方法可以更新该表。
	*/
	static SqlDatabase addDatabase(const string& type, const string& connectionName = defaultConnection);


	/*
	当用到自定义的SqlDriver,调用该重载函数。
	*/
	static SqlDatabase addDatabase(SqlDriver * driver, const string & connectionName = defaultConnection);


	/*
	关闭连接释放资源
	*/
	void close();

	/*
	返回已创建的连接名。
	*/
	static ConnectionList connectionNames();

	/*
	打开连接。
	*/
	bool open();

	void setDatabaseName(const string & name);
	void setHostName(const string & host);
	void setUserName(const string & name);
	void setPassword(const string & password);

};
#endif
