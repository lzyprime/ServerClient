#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define WAITNUM 5
#define MAXLINE 2048
#define MAXMEM 10
#define NAMELEN 20
#define SERVERADDR "192.168.207.132"
#define SERVERPORT 16667

int socket_fd;
struct sockaddr_in servaddr;
int connect_fd[MAXMEM], connect_count;
int a;

void addlog(char *);
void *quit();
void *recv_send(void *arg);
void *sendcount();
int main()
{
    // creat socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        addlog("Socket created failed.");
        return -1;
    }

    // bind ip port
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVERPORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        addlog("Bind failed.");
        return -1;
    }

    // listen
    if (listen(socket_fd, WAITNUM) < 0)
    {
        addlog("listen failed.");
        return -1;
    }

    // creat listen quit() , sendcount()
    memset(connect_fd, -1, sizeof(connect_fd));
    pthread_create(malloc(sizeof(pthread_t)), NULL, quit, NULL);
    pthread_create(malloc(sizeof(pthread_t)), NULL, sendcount, NULL);
    // wait clients
    struct sockaddr_in client_addr;
    while (1)
    {
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        int newclient = accept(socket_fd, (struct sockaddr *)&client_addr, &len);
        if (newclient < 0)
            continue;
        else if (connect_count >= MAXMEM)
        {
            write(newclient, "server is full", 15);
            close(newclient);
        }
        else
        {
            for (a = 0; a < MAXMEM; a++)
                if (connect_fd[a] == -1)
                    break;
            if (a < MAXMEM)
            {
                pthread_create(malloc(sizeof(pthread_t)), NULL, recv_send, (void *)(&a));
                connect_count++;
            }
        }
    }
    return 0;
}

void *sendcount()
{
    while (1)
    {
        while (!connect_count)
            ;
        char log[100] = {0};
        sprintf(log, "server: online: %d", connect_count);
        addlog(log);
        for (int i = 0; i < MAXMEM; i++)
            if (connect_fd[i] != -1)
                write(connect_fd[i], log, strlen(log) + 1);
        sleep(5);
    }
}

void *quit()
{
    addlog("server start success");
    while (1)
    {
        char msg[MAXLINE] = {0};
        scanf("%s", msg);
        char log[MAXLINE] = "server: ";
        strcat(log, msg);
        if (strlen(msg))
        {
            addlog(log);
            for (int i = 0; i < MAXMEM; i++)
                if (connect_fd[i] != -1)
                    write(connect_fd[i], msg, strlen(log));
        }
        if (!strcmp("quit", msg))
        {
            sleep(1);
            close(socket_fd);
            exit(0);
        }
    }
}

void addlog(char text[])
{
    time_t tt;
    FILE *log_fd = 0;
    if ((log_fd = fopen("./socket.log", "a")) != 0)
        tt = time(0), fprintf(log_fd, "%s %s\n", ctime(&tt), text);
    printf("%s \n", text);
    fclose(log_fd);
}

void *recv_send(void *arg)
{
    int id = *(int *)arg;
    int client = connect_fd[id];
    char inputext[MAXLINE];
    while (1)
    {
        int len = read(client, inputext, MAXLINE);
        if (len)
        {
            inputext[len] = 0;
            char log[MAXLINE] = "client :";
            strcat(log, inputext);
            addlog(log);
            for (int i = 0; i < MAXMEM; i++)
                if (i != id && connect_fd[i] != -1)
                    write(connect_fd[i], inputext, strlen(inputext));
        }
    }
}
