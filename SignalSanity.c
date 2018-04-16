#include "types.h"
#include "user.h"
#include "fcntl.h"

int flag1=0;
int flag2=0;

void setflag(int);
void setflag2(int);


void killTest(){
  int child=fork();
  if(child==0){
    while(1){
    
    }
  }
  kill(child,9);
  wait();
  printf(2,"Child killed!\n");
}

void basicTest(){
  printf(2,"starting test\n");
  signal(6,&setflag);
  signal(8,&setflag2);
  int pid=getpid();
  int child=fork();
  if(child==0){
    while(1){
      if(flag1){
        flag1=0;
        printf(2,"sending signal to father\n");
        kill(pid,8);
        printf(2,"exiting\n");
        exit();
      }
    }
  }
  printf(2,"sending signal to child\n");
  kill(child,6);
  while(1){
    if(flag2){
      printf(2,"received signal from child\n");
      flag2=0;
      wait();
      break;
    }
  }
}

int main(int argc,char** argv){
  killTest();
  for(int i=0;i<5;++i)basicTest();
  exit();
}


void setflag(int signum){
  flag1=1;
  return;
}

void setflag2(int signum){
  flag2=1;
  return;
}