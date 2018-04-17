#include "types.h"
#include "user.h"
#include "fcntl.h"
#define NUM_OF_CHILDREN 1

int flag1=0;
int flag2=0;
int numToPrint=0;

void setflag(int);
void setflag2(int);
void setNumToPrint(int);


void killTest(){
  int children[NUM_OF_CHILDREN];
  for(int i=0;i<NUM_OF_CHILDREN;++i){
    children[i] = fork();
    if(children[i]==0){while(1);exit();}
  }
  for(int i=0;i<NUM_OF_CHILDREN;++i){
    kill(children[i],9);
  }
  wait();
  printf(2,"All children killed!\n");
}

void basicTest(){
  printf(2,"starting basic test\n");
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

void multipleSignalsTest(){
  signal(2,&setflag);
  for(int i=3;i<9;++i){signal(i,&setNumToPrint);}
  int child;
  if((child=fork())==0){
    while(1){
      if(flag1){
        printf(2,"I'm printing number %d\n",numToPrint);
      }
      if(numToPrint==6){
        printf(2,"Bye!\n");
        numToPrint=0;
        flag1=0;
        exit();
      }
    }
  }
  printf(2,"Sending signal to child\n");
  kill(child,2);
  printf(2,"Sending multiple signals to child\n");
  for(int i=3;i<9;++i){kill(child,i);}
  wait();
}

void stopContTest(){
  int child;
  if((child=fork())==0){
    while(1){
      printf(2,"Running!!! lalalalal\n");
    }
  }
  sleep(5);
  printf(2,"Stopping child!\n");
  kill(child,17);
  while(!isStopped(child));
  printf(2,"Child stopped!\n");
  sleep(5);
  kill(child,19);
  while(isStopped(child));
  kill(child,9);
  printf(2,"child killed!\n");
}

int main(int argc,char** argv){
  killTest();
  stopContTest();
  basicTest();
  multipleSignalsTest();
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

void setNumToPrint(int signum){
  numToPrint++;
}