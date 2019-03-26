
#ifndef FLAMINGOSERVER_MYSQL_MYSQLMANAGER_H_
#define FLAMINGOSERVER_MYSQL_MYSQLMANAGER_H_

#include <memory>
#include <map>
#include <vector>

#include "../database/databasemysql.h"

#define MAXCMDLEN 8192

using std::string;

struct STableField
{
	STableField(){}
	STableField(std::string strName,std::string strType,std::string strIndex):
		m_strName(strName),
		m_strType(strType),
		m_strDesc(strIndex)
	{
	}
	string m_strName;
	string m_strType;
	string m_strDesc;
};

struct STableInfo
{
	STableInfo(){}
	STableInfo(std::string strName)
		:m_strName(strName)
	{
	}
	string m_strName;
	std::map<std::string,STableField> m_mapField;
	string m_strKeyString;
};

class CMysqlManager
{
public:
    CMysqlManager(void);
    virtual ~CMysqlManager(void);

public:
    bool Init(const char* host, const char* user, const char* pwd, const char* dbname);

	string getHost() { return m_strHost; }
	string getUser() { return m_strUser; }
	string getPwd() { return m_strPassword; }
	string getDBName() { return m_strDataBase; }
	string getCharSet() { return m_strCharactSet;  }

private:
	bool isDBExist();
	bool createDB();
	bool checkTable(const STableInfo& table);
	bool createTable(const STableInfo& table);
	bool cpdateTable(const STableInfo& table);

protected:
	std::shared_ptr<CDatabaseMysql>  m_poConn;

    string m_strHost;
    string m_strUser;
    string m_strPassword;
    string m_strDataBase;
    string m_strCharactSet;

	std::vector<STableInfo> m_vecTableInfo;
};

#endif

