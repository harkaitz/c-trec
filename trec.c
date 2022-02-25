#include "trec.h"
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pty.h>

const char *g_env_executable = "/usr/bin/env";
const char *g_ttyrec_prog    = "ttyrec";
const char *g_shell_prog     = "/bin/sh -i";
const char *g_cat_prog       = "cat";
const char *g_ttyrec_output  = "/dev/null";

int  trec_create(trec **_trec) {
    trec *trec;
    trec = calloc(1, sizeof(struct trec));
    if (!trec) {
        syslog(LOG_ERR, "Can't allocate memory: %s", strerror(errno));
        return -1;
    }
    trec->typing_speed  = 3;
    trec->typing_lag1   = 400000;
    trec->typing_lag2   = 800000;
    trec->ttyrec_pid    = -1;
    trec->cat_pid       = -1;
    trec->master_fd     = -1;
    trec->output_path_m = NULL;
    trec->input_fp      = stdin;
    trec->quiet_p       = false;
    trec->interactive_p = false;
    *_trec = trec;
    return 0;
}

int  trec_launch(trec *_trec, const char _opt_command[]) {

    int ttyrec_pid = -1;
    int cat_pid    = -1;
    int master_fd  = -1;
    
    if (_trec->ttyrec_pid != -1 ||
        _trec->cat_pid    != -1 ||
        _trec->master_fd  != -1) {
        syslog(LOG_ERR, "Only one process at a time.");
        return -1;
    }
    
    /* Fork ttyrec. */
    ttyrec_pid = forkpty(&master_fd, NULL, NULL, NULL);
    if (ttyrec_pid==0) {
        const char *cmd = (_opt_command)?_opt_command:g_shell_prog;
        execl(g_env_executable, g_env_executable,
              g_ttyrec_prog,
              "-e", cmd,
              (_trec->output_path_m)?_trec->output_path_m:g_ttyrec_output,
              NULL);
        syslog(LOG_ERR, "Can't execute ttyrec: %s\n", strerror(errno));
        exit(1);
    } else if (ttyrec_pid==-1) {
        syslog(LOG_ERR, "Can't fork ttyrec process: %s", strerror(errno));
        return -1;
    }
    
    /* Fork cat. */
    cat_pid = fork();
    if (cat_pid==0) {
        char    b[512];
        ssize_t r;
        while (1) {
            r = read(master_fd, b, sizeof(b));
            if (!r || r==-1) break;
            if (!_trec->quiet_p) write(1, b, r);
        }
        fsync(1);
        exit(0);
    } else if (ttyrec_pid==-1) {
        syslog(LOG_ERR, "Can't fork cat process: %s", strerror(errno));
        close(master_fd);
        kill(ttyrec_pid, SIGINT);
        waitpid(ttyrec_pid, NULL, 0);
        return -1;
    }

    /* Save. */
    _trec->ttyrec_pid = ttyrec_pid;
    _trec->cat_pid    = cat_pid;
    _trec->master_fd  = master_fd;
    return 0;
}
void trec_kill(trec *_trec) {
    if (_trec->ttyrec_pid!=-1) {
        kill(_trec->ttyrec_pid, SIGINT);
    }
}
void trec_wait(trec *_trec) {
    /* Wait ttyrec process (This will close slave_fd). */
    if (_trec->ttyrec_pid!=-1) {
        waitpid(_trec->ttyrec_pid, NULL, 0);
        _trec->ttyrec_pid = -1;
    }
    /* Close master_fd. Now the only process with master_fd is cat.*/
    if (_trec->master_fd!=-1) {
        close(_trec->master_fd);
        _trec->master_fd = -1;
    }
    /* Wait cat process. */
    if (_trec->cat_pid!=-1) {
        waitpid(_trec->cat_pid, NULL, 0);
        _trec->cat_pid = -1;
    }
}
void trec_destroy(trec *_trec) {
    if (_trec) {
        trec_kill(_trec);
        trec_wait(_trec);
        fclose(_trec->input_fp);
        free(_trec->output_path_m);
        free(_trec);
    }
}







