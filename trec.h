#ifndef TREC_H
#define TREC_H

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct trec trec;

struct trec {
    long       typing_speed;
    useconds_t typing_lag1;
    useconds_t typing_lag2;
    pid_t      ttyrec_pid;
    pid_t      cat_pid;
    int        master_fd;
    char      *output_path_m;
    FILE      *input_fp;
    bool       quiet_p;
    bool       interactive_p;
};

int  trec_create  (trec **_trec);
int  trec_launch  (trec  *_trec, const char _command[]);
void trec_kill    (trec *_trec);
void trec_wait    (trec  *_trec);
void trec_destroy (trec  *_trec);

#endif
