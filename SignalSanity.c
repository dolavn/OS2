#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "signal.h"
#define NUM_OF_CHILDREN 20

int flag1=0;
int flag2=0;
int flag3=0;
int count=0;

void setflag(int);
void setflag2(int);
void setflag3(int);
void incCount(int);
void setMask(int);
void itoa(int,char*);

void killTest(){
  printf(2,"starting kill test\n");
  int children[NUM_OF_CHILDREN];
  for(int i=0;i<NUM_OF_CHILDREN;++i){
    children[i] = fork();
    if(children[i]==0){while(1);exit();}
  }
  for(int i=0;i<NUM_OF_CHILDREN;++i){
    kill(children[i],SIGKILL);
  }
  for(int i=0;i<NUM_OF_CHILDREN;++i){wait();}
  printf(2,"All children killed!\n");
}

void maskChangeTest(){
  printf(2,"Starting mask change test\n");
  uint origMask = sigprocmask((1<<2)|(1<<3));
  if(origMask!=0){
    printf(2,"Original mask wasn't 0. Test failed\n");
    return;
  }
  uint secMask = sigprocmask((1<<2)|(1<<3)|(1<<4));
  if(secMask!=12){
    printf(2,"Mask wasn't changed. Test failed\n");
    return;
  }
  secMask = sigprocmask((1<<2)|(1<<3)|(1<<4));
  if(secMask!=28){
    printf(2,"Mask wasn't changed. Test failed\n");
    return;    
  }
  if(fork()==0){
    secMask = sigprocmask((1<<2)|(1<<3)|(1<<4));
    if(secMask!=28){
      printf(2,"Child didn't inherit father's signal mask. Test failed\n");
    }else{
      char* argv[] = {"SignalSanity","sigmask",0};
      if(exec(argv[0],argv)<0){
        printf(2,"couldn't exec, test failed\n");
        exit();
      }
    }
  }
  wait();
  sigprocmask(0);
}

void handlerChange(){
  printf(2,"Starting handler change test\n");
  sighandler_t sig = signal(2,&setflag);
  if(sig!=(void*)SIG_DFL){
    printf(2,"Original handler wasn't SIG_DFL, test failed\n");
    return;
  }
  sig = signal(2,&setflag);
  if(sig!=&setflag){
    printf(2,"Handler wasn't changed to custom handler, test failed\n");
    return;
  }
  sig = signal(2,(void*)SIG_DFL);
  sig = signal(2,(void*)SIG_DFL);
  if(sig!=(void*)SIG_DFL){
    printf(2,"Couldn't restore SIG_DFL, sig=%p\n test failed\n",sig);
    return;
  }
  sig = signal(3,(void*)SIG_IGN);
  sig = signal(4,&setflag);
  if(fork()==0){
    if(signal(3,(void*)SIG_IGN)!=(void*)SIG_IGN || signal(4,&setflag)!=&setflag){
      printf(2,"Signal handlers changed after fork. test failed\n");
      return;
    }
    char* argv[] = {"SignalSanity","sighandlers",0};
    if(exec(argv[0],argv)<0){
        printf(2,"couldn't exec, test failed\n");
        exit();
    }
  }
  wait();
  for(int i=0;i<NUM_OF_SIGS;++i){
    signal(i,(void*)SIG_DFL);
  }
  return;
}

void multipleChildrenTest(){
  count=0;
  int numOfSigs=32;
  for(int i=0;i<numOfSigs;++i){
    signal(i,&incCount);
  }
  int pid=getpid();
  for(int i=0;i<numOfSigs;++i){
    int child = fork();
    if(child==0){
      int signum = i;
      kill(pid,signum);
      printf(2,"%d sent signal (%d,%d)\n",getpid(),pid,signum);
      exit();
    }
  }
  while(1){
    printf(2,"%d\n",count);
    if(count==numOfSigs){
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

void maskChangeSignalTest(){
  printf(2,"Starting mask change signal test\n");
  signal(2,&setMask);
  signal(3,&setflag3);
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
      if(flag3){
        printf(2,"mask workerd\n");
        kill(pid,8);
        exit();
      }
    }
  }
  printf(2,"sending signal to child\n");
  kill(child,2);
  int count=0;
  while(1){
    if(flag2){
      flag2=0;
      count++;
      printf(2,"received signal from child\n");
      printf(2,"trying to send signal again. Then sleeping for 500\n");
      kill(child,2);
      sleep(500);
      if(flag2){
        printf(2,"Test failed. Shouldn't have received signal back.\n");
        kill(child,9);
        break;
      }
      printf(2,"did not receive signal from child, sending a different one.\n");
      kill(child,3);
      printf(2,"busy waiting for answer\n");
      while(!flag2);
      printf(2,"received signal from child.\n");
      wait();
      printf(2,"Mask change signal test passed\n");
      break;
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
  printf(2,"Communication test passed\n");
}

void multipleSignalsTest(){
  count=0;
  signal(2,&setflag);
  for(int i=3;i<9;++i){signal(i,&incCount);}
  int child;
  if((child=fork())==0){
    while(1){
      if(flag1){
        printf(2,"I'm printing number %d\n",count);
      }
      if(count==6){
        printf(2,"Bye!\n");
        count=0;
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
  printf(2,"Multiple signals test passed\n");
}

void stopContTest(){
  int child;
  signal(4,&setflag);
  if((child=fork())==0){
    while(1){
      printf(2,"Running!!! lalalalal\n");
      if(flag1){
        printf(2,"Received signal!\n");
        flag1=0;
      }
    }
  }
  sleep(5);
  printf(2,"Stopping child!\n");
  kill(child,17);
  while(!isStopped(child));
  printf(2,"Child stopped! Sending child a signal, sleep for 10 and then continuing the child.\n");
  kill(child,4);
  sleep(10);
  kill(child,19);
  while(isStopped(child));
  kill(child,9);
  wait();
  printf(2,"child killed!\n");
  printf(2,"stop cont test passed\n");
}

void sendSignalUsingKillProgram(){
  printf(2,"starting signal test using kill prog\n");
  signal(6,&setflag);
  signal(8,&setflag2);
  int pid=getpid();
  char* pidStr = char[20];
  char* signumStr = char[3];
  char* argv[] = {pidStr,signumStr,0};
  itoa(pid,pidStr);
  if(child==0){
    while(1){
      if(flag1){
        flag1=0;
        itoa(8,signumStr);
        if(fork()==0){
          if(exec(argv[0],argv)<0){
            printf(2,"couldn't exec. test failed\n");
            exit();
          }
        }
        exit();
      }
    }
  }
  itoa(child,pidStr);
  itoa(6,signumStr);
  if(fork()==0){
    if(exec(argv[0],argv)<0){
      printf(2,"couldn't exec. test failed\n");
      exit();
    }
  } 
  while(1){
    if(flag2){
      flag2=0;
      wait();
      break;
    }
  }
  printf(2,"signal test using kill prog passed\n");    
}

int main(int argc,char** argv){
  if(argc>1){
    if(strcmp(argv[1],"sigmask")==0){
      uint secMask = sigprocmask(0);
      if(secMask!=28){
        printf(2,"Mask was changed after exec, test failed.\n");
        exit();
      }
      printf(2,"Mask change test passed\n");
      exit();
    }
    if(strcmp(argv[1],"sighandlers")==0){
      if(signal(3,(void*)SIG_IGN)!=(void*)SIG_IGN){
        printf(2,"SIG_IGN wasn't kept after exec. test failed\n");
        exit();
      }
      if(signal(4,&setflag)==&setflag){
        printf(2,"Custom signal handler wasn't removed after exec. test failed\n");
        exit();
      }
      printf(2,"Handler change test passed\n");
      exit();
    }
    printf(2,"Unknown argument\n");
    exit();
  }
  multipleChildrenTest();
  killTest();
  stopContTest();
  communicationTest();
  maskChangeTest();
  handlerChange();
  maskChangeSignalTest();
  multipleSignalsTest();
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

void setflag3(int signum){
  flag3=1;
  return;
}

void incCount(int signum){
  count++;
}

void itoa(int num,char* str){
  
}
