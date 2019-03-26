#ifndef MYSQL_MYSQLTASK_H_
#define MYSQL_MYSQLTASK_H_

enum EMysqlError
{
    EME_ERROR = -1,
    EME_OK,
    EME_NOT_EXIST,
    EME_EXIST,
};

class CDatabaseMysql;

class IMysqlTask
{
public:
    IMysqlTask(void) {};
    virtual ~IMysqlTask(void) {};

public:
	virtual void Execute(CDatabaseMysql* poConn) = 0;
    virtual void Reply() = 0;
    virtual void Release() { delete this; };
};

#endif  //MYSQL_MYSQLTASK_H_


