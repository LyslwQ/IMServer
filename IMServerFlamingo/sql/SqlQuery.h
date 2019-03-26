#ifndef SQL_SQLQUERY_H_
#define SQL_SQLQUERY_H_

#include <string>
#include "SqlDatabase.h"

using std::string;

class QSqlRecord;
class QVariant;

class SqlQuery
{
public:
	SqlQuery(const string & query = string(), SqlDatabase db = SqlDatabase());
	SqlQuery(SqlDatabase db);
	~SqlQuery();


	/*
	执行查询语句。
	*/
	bool exec(const string & query);


	/*
	执行了exec后的QSqlQuery会处于Active状态，
	只有处于Active的QSqlQuery中的结果才是有效的。
	*/
	bool isActive() const;


	/*
	返回当前记录。
	*/
	QSqlRecord record() const;


	/*
	返回当前记录中，index列的值。
	*/
	QVariant value(int index) const;


	/*
	检索 结果中，当前记录的位置。
	*/
	int at() const;

	/*
	检索 结果中，先前的记录。
	*/
	bool previous();

	/*
	检索 结果中，第一条记录。
	*/
	bool first();

	/*
	检索 结果中，最后一条记录。
	*/
	bool last();

	/*
	检索 结果中，index所指位置记录。
	*/
	bool seek(int index, bool relative = false);


	/*
	判断当前记录 指定字段是否为null。
	*/
	bool isNull(int field) const;

	/*
	判断当前查询是否位于有效记录上。
	*/
	bool isValid() const;



};

#endif
