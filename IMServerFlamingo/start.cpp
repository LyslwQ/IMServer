#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <iostream>

using std::cout;
using std::endl;

int childProcess(int i)
{
  //char* argv[1] = "Hey child,Im your parent...";
  if( i == 0)
    {
      if( execv("myfileserver",NULL) == -1)
        {
          cout << "exec():error--" << errno << endl;
          return -1;
        }
    }
   else if( i == 1)
    {
      if( execv("mychatserver",NULL) == -1)
        {
          cout << "exec():error--" << errno << endl;
          return -1;
        }
    }
   else if( i == 2)
    {
      if( execv("myimgserver",NULL) == -1)
        {
          cout << "exec():error--" << errno << endl;
          return -1;
        }
    }

  return 0;
}


int main()
{
  cout << "starting..." << endl;
  int ret;
  int cid[3];
  for(int i = 0; i < 3; ++i)
    {
      if( (ret = fork() ) == -1 )
        {
          cout << "fork():error -- " << errno << endl;
          continue;
        }
      else if( ret ==  0)
        {
          childProcess(i);
        }
      cid[i] = ret;
    }

  for(int i = 0; i < 3; ++i)
    {
      cout << "the childProcess-id = " << cid[i] << endl;
    }
  
  int state;
  for(int i = 0; i< 3; ++i)
    {
      int ret;
      if( (ret = waitpid(-1, &state, 0) ) < 0)
        {
          cout << "waitpit():error--" << errno << endl;
          continue;
        }
      cout << "child--" << ret 
           << "\texitState--" << (WIFEXITED(state) != 0? ("normal":"abnormal") << endl;
    }
  return 0;
}

