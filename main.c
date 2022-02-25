#include "trec.h"
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <libgen.h>
#include <wchar.h>
#define PROGNAME "trec"
#define NL       "\n"

struct trec_cmd {
    const char *name;
    int       (*func) (trec *_trec, const char cmd[], char args[]);
    bool        repeat;
};
struct trec_keys {
    const char *name;
    const char *data;
};


static int cmd_launch(trec *_trec, const char cmd[], char args[]);
static int cmd_kill(trec *_trec, const char cmd[], char args[]);
static int cmd_write(trec *_trec, const char cmd[], char args[]);
static int cmd_type(trec *_trec, const char cmd[], char args[]);
static int cmd_wait(trec *_trec, const char cmd[], char args[]);

static const char help_header_prog[] =
    "Usage: %s [FILENAME] [-q][-o TTYRECORD][-s SPEED]"  NL;
static const char help_header_file[] =
    "Usage: %s [-q][-o TTYRECORD][-s SPEED]"  NL;
static const char help_description[] =
    "This program helps creating demostrations for your terminal" NL
    "programs in a replicable way."                               NL;
static const char help_flags[] =
    "  -q           : Run everything in a hidden the terminal."        NL
    "  -o TTYRECORD : Save a recording for `seq2gif(1)`, `ttyplay(1)`" NL
    "  -o SPEED     : Fasten/slow demostration (1 to 10) (default 3)." NL;
static const char help_commands[] =
    "  launch [COMMAND...] : Launch command (at least one)."   NL
    "  kill                : Kill the launched command."       NL
    "  write CHARACTERS... : Type characters at once."         NL
    "  type CHARACTERS...  : Type characters."                 NL
    "  wait                : Wait a little."                   NL
    "  exit                : Quit."                            NL;
static const struct trec_cmd cmds[] = {
    { "write"      , cmd_write      , false },
    { "launch"     , cmd_launch     , false },
    { "kill"       , cmd_kill       , true  },
    { "type"       , cmd_type       , false },
    { "wait"       , cmd_wait       , true  },
    { NULL}
};
static const struct trec_keys keys[] = {
    { "enter", "\n"   },
    { "up"   , "\r[[A"},
    { "down" , "\r[[B"},
    { "right", "\r[[C"},
    { "left" , "\r[[D"},
    { NULL},
};



int main (int _argc, char *_argv[]) {
    
    int                     argc       = _argc;
    char                  **argv       = _argv;
    char                   *input_file = NULL;
    int                     retval     = 1;
    int                     err,opt,i;
    trec                   *trec         = NULL;
    char                    buffer[1024] = {0};
    char                   *line,*tok,*cmd,*arg,*rep;
    const struct trec_cmd  *trec_cmd     = NULL;

    /* Get input file. */
    if (argc > 1 && argv[1][0]!='-') {
        input_file = argv[1];
        argc--;
        argv++;
    }

    /* Print help. */
    if (argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
        if (input_file) {
            printf(help_header_file, basename(input_file));
            printf(NL);
            FILE *fp = fopen(input_file, "rb");
            if (fp) {
                bool extra_paragraph = false;
                while ((line=fgets(buffer, sizeof(buffer)-1, fp))) {
                    if (line[0]!='#') break;
                    if (line[1]=='!') continue;
                    if (line[1]!='#') break;
                    while (*line != '\0' && strchr("# \t\r", *line)) line++;
                    fputs(line, stdout);
                    extra_paragraph = true;
                }
                fclose(fp);
                if (extra_paragraph) {
                    fputs(NL, stdout);
                }
            }
            printf(help_flags);
            printf(NL);
            return 0;
        } else {
            printf(help_header_prog, PROGNAME);
            printf(NL);
            printf(help_description);
            printf(NL);
            printf("Command line flags:" NL NL);
            printf(help_flags);
            printf(NL);
            printf("Instructions:" NL NL);
            printf(help_commands);
            printf(NL);
            return 0;
        }
    }
    
    /* Configure logging. */
    openlog((input_file)?input_file:PROGNAME, LOG_PERROR, LOG_USER);

    /* Initialize trec. */
    err = trec_create(&trec);
    if (err<0) goto cleanup;

    /* Set input file. */
    if (input_file && !(trec->input_fp = fopen(input_file, "rb"))) {
        syslog(LOG_ERR, "%s: %s", input_file, strerror(errno));
        goto cleanup;
    }
    if (isatty(fileno(trec->input_fp))) {
        trec->interactive_p = true;
    }
    
    /* Get options. */
    while((opt = getopt (argc, argv, "qo:s:")) != -1) {
        switch (opt) {
        case 'q': trec->quiet_p = true; break;
        case 'o': trec->output_path_m = strdup(optarg); break;
        case 's':
            trec->typing_speed  = strtol(optarg, NULL, 0);
            if (trec->typing_speed < 1 || trec->typing_speed > 10) {
                syslog(LOG_ERR, "Invalid speed: %s", optarg);
                goto cleanup;
            }
            break;
        case '?':
        default:
            return 1;
        }
    }
    
    /* Execute commands. */
    while (1) {
        if (trec->interactive_p) {
            fputs("> ", stderr);
        }
        if (!(line=fgets(buffer, sizeof(buffer), trec->input_fp))) {
            break;
        }
        if (line[0]=='#') {
            continue;
        }
        rep = NULL;
        cmd = strtok_r(line, " \t\n\r", &tok);
        repeat:
        if (!cmd) {
            continue;
        }
        if (!strcasecmp(cmd, "exit") || !strcasecmp(cmd, "quit")) {
            break;
        }
        arg = strtok_r(NULL, "\n\r", &tok);
        trec_cmd = NULL;
        for (i=0; cmds[i].name; i++) {
            if (!strcasecmp(cmd, cmds[i].name)) {
                trec_cmd = &cmds[i];
                if (cmds[i].repeat) {
                    rep = arg;
                }
            }
        }
        if (!trec_cmd) {
            for (i=0; keys[i].name; i++) {
                if (!strcasecmp(cmd, keys[i].name)) {
                    trec_cmd = &cmds[0];
                    rep = arg;
                    arg = (char*)keys[i].data;
                }
            }
        }
        if (!trec_cmd) {
            syslog(LOG_ERR, "Unknown command: %s", cmd);
            retval = 2;
            if (trec->interactive_p) continue; else break;
        }
        i = trec_cmd->func(trec, cmd, arg);
        if (i<0) {
            retval = 3;
            if (trec->interactive_p) continue; else break;
        }
        if (rep) {
            cmd = strtok_r(rep, " \t\n\r", &tok);
            goto repeat;
        }
        
    }

    /* Wait child processes. */
    usleep(200000);
    trec_kill(trec);
    trec_wait(trec);

    /* Cleanup. */
    retval = 0;
    cleanup:
    trec_destroy(trec);
    return retval;
}
int cmd_launch(trec *_trec, const char cmd[], char args[]) {
    return trec_launch(_trec, args);
}
int cmd_kill(trec *_trec, const char cmd[], char args[]) {
    trec_kill(_trec);
    trec_wait(_trec);
    return 0;
}
int cmd_type(trec *_trec, const char cmd[], char args[]) {
    if (_trec->master_fd==-1 || _trec->ttyrec_pid==-1) {
        syslog(LOG_ERR, "type: The process is not running.");
        return -1;
    }
    for (const char *c = args; *c; c++) {
        usleep(_trec->typing_lag1/_trec->typing_speed);
        write(_trec->master_fd, c, 1);
    }
    return 0;
}
int cmd_write(trec *_trec, const char cmd[], char args[]) {
    if (_trec->master_fd==-1 || _trec->ttyrec_pid==-1) {
        syslog(LOG_ERR, "type: The process is not running.");
        return -1;
    }
    usleep(_trec->typing_lag2/_trec->typing_speed);
    write(_trec->master_fd, args, strlen(args));
    return 0;
}
int cmd_wait(trec *_trec, const char cmd[], char args[]) {
    usleep(_trec->typing_lag2/_trec->typing_speed);
    return 0;
}


