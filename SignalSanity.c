#include "types.h"
#include "user.h"
#include "fcntl.h"
#define NUM_OF_CHILDREN 10

int flag1=0;
int flag2=0;
int numToPrint=0;

void setflag(int);
void setflag2(int);
void setNumToPrint(int);
void setMask(int);

void killTest(){
  printf(2,"starting kill test\n");
  int children[NUM_OF_CHILDREN];
  for(int i=0;i<NUM_OF_CHILDREN;++i){
    children[i] = fork();
    if(children[i]==0){while(1);exit();}
  }
  for(int i=0;i<NUM_OF_CHILDREN;++i){
    kill(children[i],9);
  }
  for(int i=0;i<NUM_OF_CHILDREN;++i){wait();}
  printf(2,"All children killed!\n");
}

void multipleChildrenTest(){
  numToPrint=0;
  int numOfSigs=32;
  for(int i=0;i<numOfSigs;++i){
    signal(i,&setNumToPrint);
  }
  int pid=getpid();
  for(int i=0;i<numOfSigs;++i){
    if(fork()==0){
      sleep(100);
      int signum = i;
      kill(pid,signum);
      printf(2,"%d sent signal (%d,%d)\n",getpid(),pid,signum);
      exit();
    }
  }
  while(1){
    printf(2,"%d\n",numToPrint);
    if(numToPrint==numOfSigs){
      printf(2,"All signals received!\n");
      break;
    }
  }
  for(int i=0;i<numOfSigs;++i){
    wait();
  }
  for(int i=0;i<numOfSigs;++i){
    signal(i,(void*)-1);
  }
}

void maskChangeTest(){
  printf(2,"starting mask change test\n");
  signal(2,&setMask);
  signal(8,&setflag2);
  int pid=getpid();
  int child=fork();
  if(child==0){
    int count=0;
    while(1){
      if(flag1){
        count++;
        flag1=0;
        printf(2,"sending signal to father\n");
        kill(pid,8);
      }
      if(count==2){
        exit();
      }
    }
  }
  printf(2,"sending signal to child\n");
  kill(child,2);
  int count=0;
  while(1){
    if(flag2){
      count++;
      printf(2,"received signal from child\n");
      printf(2,"trying to send signal again. Then sleeping for 1000\n");
      kill(child,2);
      sleep(1000);
    }
  }
}

void communicationTest(){
  printf(2,"starting communication test\n");
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
  numToPrint=0;
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
  multipleChildrenTest();
  killTest();
  //stopContTest();
  for(int i=0;i<5;++i)communicationTest();
  //multipleSignalsTest();
  exit();
}

void setMask(int signum){
  flag1=1;
  sigprocmask(1<<signum);
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