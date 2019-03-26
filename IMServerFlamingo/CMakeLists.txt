cmake_minimum_required(VERSION 2.6)

project (MYSERVER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -O0 -Wno-unused-variable -pthread")

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
	/usr/lib64/mysql/
)

find_library(MYSQL_LIB libmysqlclient.so.18 /usr/lib64/mysql/)
IF (NOT MYSQL_LIB)
    MESSAGE(FATAL_ERROR "mysqlclient.so not found")
ENDIF(NOT MYSQL_LIB)

set(net_srcs 
base/timestamp.cpp
base/countdownlatch.cpp
base/fileutil.cpp
base/logging.cpp
base/logfile.cpp
base/logstream.cpp
base/asynclogging.cpp
base/configfilereader.cpp

net/acceptor.cpp
net/buffer.cpp
net/channel.cpp
net/connector.cpp
net/epollpoller.cpp
net/eventloop.cpp
net/inetaddress.cpp
net/sockets.cpp
net/tcpclient.cpp
net/tcpconnection.cpp
net/tcpserver.cpp
net/eventloopthread.cpp
net/eventloopthreadpool.cpp
net/protocolstream.cpp
net/protocolstream.cpp
net/Timer.cpp
net/TimerQueue.cpp

common/ngx_md5.cpp
)

set(database_srcs
database/databasemysql.cpp
database/field.cpp
database/queryresult.cpp
)

set(mysql_srcs
mysql/mysqlmanager.cpp
mysql/mysqlthrd.cpp
mysql/mysqlthrdmgr.cpp
mysql/tasklist.cpp
)

set(json_srcs
jsoncpp-0.5.0/json_reader.cpp
jsoncpp-0.5.0/json_value.cpp
jsoncpp-0.5.0/json_writer.cpp
)


set(model_srcs
model/IMGroup.cpp
model/IMGroup-odb.cpp
model/IMGroupMember.cpp
model/IMGroupMember-odb.cpp
model/IMMessage.cpp
model/IMMessage-odb.cpp
model/Model.cpp
model/Relationship.cpp
model/Relationship-odb.cpp
model/User.cpp
model/User-odb.cpp
)

set(mychatserver_srcs
chatserversrc/main.cpp
chatserversrc/IMServer.cpp
chatserversrc/ClientSession.cpp
chatserversrc/UserManager.cpp
chatserversrc/MsgCacheManager.cpp
chatserversrc/TcpSession.cpp)

set(myfileserver_srcs
fileserversrc/main.cpp
fileserversrc/FileServer.cpp
fileserversrc/FileSession.cpp
fileserversrc/FileManager.cpp
fileserversrc/MD5.cpp
fileserversrc/TcpSession.cpp)

set(myimgserver_srcs
imgserversrc/main.cpp
fileserversrc/FileServer.cpp
fileserversrc/FileSession.cpp
fileserversrc/FileManager.cpp
fileserversrc/MD5.cpp
fileserversrc/TcpSession.cpp)


add_executable(mychatserver ${net_srcs} ${json_srcs}  ${mychatserver_srcs} ${mysql_srcs} ${database_srcs} ${model_srcs})
#光包含库目录是没用的，还必须使用TARGET_LINK_LIBRARIES链接该库
TARGET_LINK_LIBRARIES(mychatserver mysqlclient odb-mysql odb)

add_executable(myfileserver ${net_srcs}  ${myfileserver_srcs} )
TARGET_LINK_LIBRARIES(myfileserver)

add_executable(myimgserver ${net_srcs}  ${myimgserver_srcs} )
TARGET_LINK_LIBRARIES(myimgserver)





