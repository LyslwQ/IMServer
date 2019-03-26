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
	����������һ�������ӡ�
	��������Ϊʲô��ֱ���ڹ��캯������ɾͺã�
	����������Ϊ�����ά��һ�����ݿ����ӱ�static��Ա����
	        ������ÿ��SqlDatabaseʵ�����ᱻ�ӵ��ñ��У�
			������static�������Ը��¸ñ�
	*/
	static SqlDatabase addDatabase(const string& type, const string& connectionName = defaultConnection);


	/*
	���õ��Զ����SqlDriver,���ø����غ�����
	*/
	static SqlDatabase addDatabase(SqlDriver * driver, const string & connectionName = defaultConnection);


	/*
	�ر������ͷ���Դ
	*/
	void close();

	/*
	�����Ѵ�������������
	*/
	static ConnectionList connectionNames();

	/*
	�����ӡ�
	*/
	bool open();

	void setDatabaseName(const string & name);
	void setHostName(const string & host);
	void setUserName(const string & name);
	void setPassword(const string & password);

};
#endif
