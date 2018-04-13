#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;

  if(argc < 3 || argc%2==0){
    printf(2, "usage: kill pid signum...\n");
    exit();
  }
  for(i=1; i<argc-1; i=i+2){
    printf(2,"sending %s signal:%s\n",argv[i],argv[i+1]);
    kill(atoi(argv[i]),atoi(argv[i+1]));
  }
  exit();
}
