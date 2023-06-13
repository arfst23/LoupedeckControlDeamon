#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include "command.h"

extern const char *__progname;

void run(const char *name, const char *arg)
{
  pid_t pid = fork();
  assert(pid >= 0);
  if (pid) // parent
  {
    int stat;
    pid_t ret = wait(&stat);
    assert(ret == pid);
    assert(!stat);
  }
  else // child
  {
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid) // parent
      _exit(0);
    else // child
    {
      execlp(name, name, arg, NULL);
      assert(0);
    }
  }
}

void restart()
{
  execlp(__progname, __progname , NULL);
  assert(0);
}
