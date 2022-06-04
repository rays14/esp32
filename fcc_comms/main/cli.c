#include <stdio.h>
#include <string.h>
#include "cli.h"

#define CMD_SIZE      (100)
#define CMD_HIST_SIZE (10)

static char cmdHistoryTable[CMD_HIST_SIZE][CMD_SIZE];
static uint32_t cmdHistoryTableCurrentIndex = CMD_HIST_SIZE - 1;

// ----------------------------------------------------------------------------
// Make changes to the command table.
// ----------------------------------------------------------------------------

// These callbacks and command table goes in its own file
// so this file is not updated when new commands are added.
// This file is not to be touched once complete.
static void menuCmdCb(int argc, char *argv[]);
static void helpCmdCb(int argc, char *argv[]);
static void historyCmdCb(int argc, char *argv[]);

volatile struct cli_cmds_t cmdTable[] = {
    {
        "menu",
        "Command menu.",
        NULL,
        menuCmdCb
    },
    {
        "help",
        "Command help.",
        NULL,
        helpCmdCb
    },
    {
        "history",
        "Command history.",
        NULL,
        historyCmdCb
    },
    {NULL, NULL, NULL, NULL}
};
#if 0
static void printArgs(int argc, char *argv[]) {
    printf("argc = %d ", argc);
    for (uint32_t i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\r\n");
}
#endif
static void historyCmdCb(int argc, char *argv[]) {
    for (uint32_t i = 0; i < CMD_HIST_SIZE; i++) {
        char *hs = cmdHistoryTable[i];
        if (hs && (strlen(hs) > 0)) {
            printf("%s\n", hs);
        }
    }
}
static void menuCmdCb(int argc, char *argv[]) {
    printf(" ----------------------------------\n");
    printf(" -------------- Menu --------------\n");
    printf(" ----------------------------------\n");
    for (uint32_t i = 0; cmdTable[i].cb; i++) {
        char *cmd  = cmdTable[i].cmd;
        char *cmdd = cmdTable[i].cmdDescription;
        char *cmda = cmdTable[i].cmdArguments;
        if (cmd) {
            //printf("   %d. %s %s\r\n\t\t%s \r\n", i, cmd, cmda ? cmda : " ", cmdd ? cmdd : " ");
            printf("   %d. %s %s\n", i, cmd, cmda ? cmda : " ");
        }
    }
    printf(" ----------------------------------\n\n");

}
static void helpCmdCb(int argc, char *argv[]) {
    //printf("\r\nhelp_cmd_cb\r\n");
    //printArgs(argc, argv);
    printf(" ----------------------------------\n");
    printf("Boo .. I am of no help!!\n");
    printf(" ----------------------------------\n");
}
// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Below this - DO NOT CHANGE.
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------



void cliInit(void) {
    // Initialize the command history table.
    for (uint32_t i = 0; i < CMD_HIST_SIZE; i++) {
        for (uint32_t j = 0; j < CMD_SIZE; j++) {
            cmdHistoryTable[i][j] = '0';
        }
    }
}

static void cliParseCmd(char *cmd) {
    static char *argv[NUM_ARGVS];
    int argc = 0;
    char *tok;
    char *tokStr = cmd;

    // Tokenize.
    while ((tok = strtok(tokStr, " ")) != NULL) {
        argv[argc] = tok;
        //printf("cliParseCmd : %s\n", tok);
        argc++;
        tokStr = NULL;
    }

    // Based on argv[0] call the command and pass in arguments.
    for (uint32_t i = 0; (cmdTable[i].cmd != NULL); i++) {
        if (argv[0]) {
            //printf("i = %d, cmd = %s, argv = %s\n", i, cmdTable[i].cmd, argv[0]);
            if (strcmp(cmdTable[i].cmd, argv[0]) == 0) {
                cmdTable[i].cb(argc, argv);
                goto cliParseCmdExit;
            }
        } else {
            //printf("i = %d, cmd = %s\n", i, cmdTable[i].cmd);
        }
    }
cliParseCmdExit:
   return;
}

char inbyte() {
    char c;
    while ((c = getc(stdin)) == 255);
    //printf("%d 0x%02x\n", c, c);
    return c;
}

void cliTaskBlocking(void) {
    static char cmd[CMD_SIZE];
    static uint32_t index = 0;
    char c;

    c = inbyte();
    if (c == '\n') {
        printf("\n");
        cmd[index] = '\0';
        index = 0;

        if (strlen(cmd) > 0) {
            // Add cmd to the command list. First shift all commands
            // up the table by 1. Then copy new command into table.
            for (uint32_t i = 0; i < CMD_HIST_SIZE - 1; i++) {
                strncpy(cmdHistoryTable[i], cmdHistoryTable[i + 1], CMD_SIZE);
            }
            strncpy(cmdHistoryTable[CMD_HIST_SIZE - 1], cmd, CMD_SIZE);
            cmdHistoryTableCurrentIndex = CMD_HIST_SIZE - 1;
        }

        cliParseCmd(cmd);
        printf("> ");

        goto cliTaskExit;
    } else if (c == '\b') {
        printf("\b ");
        //cmd[index] = (char8)' ';
        cmd[index] = ' ';
        if (index > 0)
            index--;
    } else if (c == 27) {
        //c = inbyte(); // Get the [ character.
        c = inbyte(); // Get the [ character.
        c = inbyte(); // Get up, down, left, right.
        if (c == 'A') { // up
            printf("\033[1K");
            for (uint32_t i = 0; i < CMD_SIZE; i++) {
                printf("\b");
            }

            printf("> ");
            printf("%s", cmdHistoryTable[cmdHistoryTableCurrentIndex]);
            strncpy(cmd, cmdHistoryTable[cmdHistoryTableCurrentIndex], CMD_SIZE);
            index = strlen(cmd);

            // Move to the past in history.
            if ((cmdHistoryTableCurrentIndex > 0) &&
                (strlen(cmdHistoryTable[cmdHistoryTableCurrentIndex - 1]) > 0)) {
                cmdHistoryTableCurrentIndex--;
            }
        } else if (c == 'B') { // down
            printf("\033[1K");
            for (uint32_t i = 0; i < CMD_SIZE; i++) {
                printf("\b");
            }
            printf("> ");
            printf("%s", cmdHistoryTable[cmdHistoryTableCurrentIndex]);
            strncpy(cmd, cmdHistoryTable[cmdHistoryTableCurrentIndex], CMD_SIZE);
            index = strlen(cmd);

            // Move to the past in history.
            if ((cmdHistoryTableCurrentIndex < CMD_HIST_SIZE - 1) &&
                (strlen(cmdHistoryTable[cmdHistoryTableCurrentIndex]) > 0)) {
                cmdHistoryTableCurrentIndex++;
            }
        } else if (c == 'C') { // Forward.
            printf("forward ");
        } else if (c == 'D') { // backward
            printf("backward ");
        }

        // Simulates a continue so the last \r is not printed again.
        goto cliTaskExit;
    } else {
        cmd[index] = c;
        if (index < 100)
            index++;
    }
    printf("%c", c);

cliTaskExit:
    return;
}
