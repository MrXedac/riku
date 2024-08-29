#ifndef __UNISTD__
#define __UNISTD__

int execve(char* name, char** argv, char** env);
void exit();
int wait(int pid);

#endif