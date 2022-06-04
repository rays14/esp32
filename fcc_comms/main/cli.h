#ifndef SRC_CLI_H_
#define SRC_CLI_H_

#define CLI_TASK (1)
#define NUM_ARGVS (50)

typedef void (*cli_cmd_cb)(int argc, char *argv[]);

struct cli_cmds_t
{
	char *cmd;
	char *cmdDescription;
	char *cmdArguments;
    cli_cmd_cb cb;
};

void cliTaskBlocking(void);


#endif /* SRC_CLI_H_ */
