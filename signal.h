#ifndef SIGNAL_H_
#define SIGNAL_H_

#define NUM_OF_SIGS 32
#define SIG_DFL 0
#define SIG_IGN 1

#define SIGKILL 9
#define SIGSTOP 17
#define SIGCONT 19

typedef void (*sighandler_t)(int);

#endif