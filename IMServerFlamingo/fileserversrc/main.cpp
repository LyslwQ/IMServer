/**
 *  �ļ����������ں���
 **/
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include "../base/logging.h"
#include "../base/singleton.h"
#include "../base/configfilereader.h"
#include "../base/asynclogging.h"
#include "../net/eventloop.h"
#include "../net/eventloopthreadpool.h"
#include "FileManager.h"
#include "FileServer.h"

using namespace net;

EventLoop g_mainLoop;

AsyncLogging* g_asyncLog = NULL;
void asyncOutput(const char* msg, int len)
{
    if (g_asyncLog != NULL)
    {
        g_asyncLog->append(msg, len);
        std::cout << msg << std::endl;
    }
}

void prog_exit(int signo)
{
    std::cout << "program recv signal [" << signo << "] to exit." << std::endl;

    g_mainLoop.quit();
}

void daemon_run()
{
    int pid;
    signal(SIGCHLD, SIG_IGN);
    //1���ڸ������У�fork�����´����ӽ��̵Ľ���ID��
    //2�����ӽ����У�fork����0��
    //3��������ִ���fork����һ����ֵ��
    pid = fork();
    if (pid < 0)
    {
        std::cout << "fork error" << std::endl;
        exit(-1);
    }
    //�������˳����ӽ��̶�������
    else if (pid > 0) {
        exit(0);
    }
    //֮ǰparent��child������ͬһ��session��,parent�ǻỰ��session������ͷ����,
    //parent������Ϊ�Ự����ͷ���̣����exit����ִ�еĻ�����ô�ӽ��̻��Ϊ�¶����̣�����init������
    //ִ��setsid()֮��,child�����»��һ���µĻỰ(session)id��
    //��ʱparent�˳�֮��,������Ӱ�쵽child�ˡ�
    setsid();
    int fd;
    fd = open("/dev/null", O_RDWR, 0);
    if (fd != -1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    if (fd > 2)
        close(fd);
}


int main(int argc, char* argv[])
{
    //�����źŴ���
    signal(SIGCHLD, SIG_DFL);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, prog_exit);
    signal(SIGTERM, prog_exit);

    int ch;
    bool bdaemon = false;
    while ((ch = getopt(argc, argv, "d")) != -1)
    {
        switch (ch)
        {
        case 'd':
            bdaemon = true;
            break;
        }
    }

    if (bdaemon)
        daemon_run();

    CConfigFileReader config("fileserver.conf");
    const char* logfilepath = config.GetConfigName("logfiledir");
    if (logfilepath == NULL)
    {
        LOG_SYSFATAL << "logdir is not set in config file";
        return 1;
    }
    //���logĿ¼�������򴴽�֮
    DIR* dp = opendir(logfilepath);
    if (dp == NULL)
    {
        if (mkdir(logfilepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
        {
            LOG_SYSFATAL << "create base dir error, " << logfilepath << ", errno: " << errno << ", " << strerror(errno);
            return 1;
        }
    }
    closedir(dp);

    const char* logfilename = config.GetConfigName("logfilename");
    if (logfilename == NULL)
    {
        LOG_SYSFATAL << "logfilename is not set in config file";
        return 1;
    }
    std::string strLogFileFullPath(logfilepath);
    strLogFileFullPath += logfilename;
    Logger::setLogLevel(Logger::INFO);
    int kRollSize = 500 * 1000 * 1000;
    AsyncLogging log(strLogFileFullPath.c_str(), kRollSize);
    log.start();
    g_asyncLog = &log;
    Logger::setOutput(asyncOutput);

    const char* filecachedir = config.GetConfigName("filecachedir");
    Singleton<FileManager>::Instance().Init(filecachedir);

    Singleton<EventLoopThreadPool>::Instance().Init(&g_mainLoop, 6);
    Singleton<EventLoopThreadPool>::Instance().start();

    const char* listenip = config.GetConfigName("listenip");
    short listenport = (short)atol(config.GetConfigName("listenport"));
    Singleton<FileServer>::Instance().Init(listenip, listenport, &g_mainLoop, filecachedir);

    LOG_INFO << "fileserver initialization complete.";
    
    g_mainLoop.loop();

    return 0;
}
