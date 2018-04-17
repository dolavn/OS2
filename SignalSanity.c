#include "types.h"
#include "user.h"
#include "fcntl.h"

int flag1=0;
int flag2=0;
int numToPrint=0;

void setflag(int);
void setflag2(int);
void setNumToPrint(int);


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
        printf(2,"I  signal(2,&setflag);'m printing number %d\n",numToPrint);
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
  printf(2,"Sending multiple signal to child\n");
  for(int i=3;i<9;++i){kill(child,i);}
  wait();
}

int main(int argc,char** argv){
  killTest();
  basicTest();
  multipleSignalsTest();
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