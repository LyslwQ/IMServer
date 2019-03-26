#ifndef STRUCT_INFO_H_
#define STRUCT_INFO_H_

#include <string>
#include <set>
#include "type.h"

using std::string;

//用户
struct UserInfo
{
  tid_t          userid;      //0x0FFFFFFF以上是群号，以下是普通用户
  string         username;    //群账户的username也是群号userid的字符串形式
  string         password;
  string         nickname;    //群账号为群名称
  int32_t        facetype;
  string         customface;
  string         customfacefmt;//自定义头像格式
  int32_t        gender;
  int32_t        birthday;
  string         signature;
  string         address;
  string         phonenumber;
  string         mail;
  string         teaminfo;
  int32_t        ownerid;        //对于群账号，为群主userid
  std::set<int32_t>   friends;        //为了避免重复
};


struct GroupInfo
{
  tid_t         tid;
  tid_t         creatorId;
  string        groupName;
};
#endif //structinfo.h
