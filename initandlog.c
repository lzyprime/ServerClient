#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

void init_daemon(void)
{
    int pid;
    int i = 0;
    if (pid = fork())
    {
        exit(0); //if father,kill it
    }
    else if (pid < 0)
    {
        exit(1); //fork error ,quit
    }

    setsid(); //first subprocess become new session boss and process boss
        //leave with terminal

    if (pid = fork())
    {
        exit(0); //first sub,kill it
    }
    else if (pid < 0)
    {
        exit(1); //fork error,quit
    }

    for (i = 0; i < NOFILE; ++i)
    {
        close(i); //close file description
    }
    // chdir("/tmp");		//change work dir to /tmp
    umask(0); //reset file mask
    return;
}

static void printlog(const char *on_what)
{
	FILE *fp = NULL;
	time_t tt;

	if((fp = fopen("./socket.log", "a"))!=NULL)
	{
		tt = time(0);
		fprintf(fp, "server>  %s do: %s\n", asctime(localtime(&tt)), on_what);
	}

	fclose(fp);
}
