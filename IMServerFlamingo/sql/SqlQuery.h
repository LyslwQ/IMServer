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
	ִ�в�ѯ��䡣
	*/
	bool exec(const string & query);


	/*
	ִ����exec���QSqlQuery�ᴦ��Active״̬��
	ֻ�д���Active��QSqlQuery�еĽ��������Ч�ġ�
	*/
	bool isActive() const;


	/*
	���ص�ǰ��¼��
	*/
	QSqlRecord record() const;


	/*
	���ص�ǰ��¼�У�index�е�ֵ��
	*/
	QVariant value(int index) const;


	/*
	���� ����У���ǰ��¼��λ�á�
	*/
	int at() const;

	/*
	���� ����У���ǰ�ļ�¼��
	*/
	bool previous();

	/*
	���� ����У���һ����¼��
	*/
	bool first();

	/*
	���� ����У����һ����¼��
	*/
	bool last();

	/*
	���� ����У�index��ָλ�ü�¼��
	*/
	bool seek(int index, bool relative = false);


	/*
	�жϵ�ǰ��¼ ָ���ֶ��Ƿ�Ϊnull��
	*/
	bool isNull(int field) const;

	/*
	�жϵ�ǰ��ѯ�Ƿ�λ����Ч��¼�ϡ�
	*/
	bool isValid() const;



};

#endif
