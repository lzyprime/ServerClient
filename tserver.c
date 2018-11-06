#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#define WAITNUM 5
#define MAXLINE 2048
#define MAXMEM 10
#define SERVERADDR "127.0.0.1"
#define SERVERPORT 16666

int socket_fd;
struct sockaddr_in servaddr;
int connect_fd[MAXMEM], connect_count;
int a;

void addlog(char *);
void *quit();
void *recv_send(void *arg);
void *sendcount();
int main(int argc, char const *argv[])
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
    if (argc == 3)
        servaddr.sin_port = htons(argv[2]);
    else
        servaddr.sin_port = htons(SERVERPORT);
    if (argc >= 2)
        servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    else
        servaddr.sin_addr.s_addr = inet_addr(SERVERADDR);

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

    // wait clients
    struct sockaddr_in client_addr;
    while (1)
    {
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        int *newcilent = (int *)malloc(sizeof(int));
        *newcilent = accept(socket_fd, (struct sockaddr *)&client_addr, &len);
        if (newcilent < 0)
            free(newcilent);
        else if (connect_count >= MAXMEM)
        {
            write(*newcilent, "server is full", 15);
            free(newcilent);
        }
        else
            pthread_create(malloc(sizeof(pthread_t)), NULL, recv_send, (void *)(newcilent));
    }
    return 0;
}

void closesock(int *a)
{
    connect_count--;
    close(*a);
    *a = -1;
}

void server_send(char msg[])
{
    char log[MAXLINE] = "server: ";
    strcat(log, msg);
    addlog(log);
    for (int i = 0; i < MAXMEM; i++)
        if (connect_fd[i] != -1)
            if (write(connect_fd[i], log, strlen(log)) < 0)
                closesock(&connect_fd[i]);
}

void *quit()
{
    addlog("server start success");
    while (1)
    {
        char msg[MAXLINE] = {0};
        fgets(msg, MAXLINE, stdin);
        msg[strlen(msg) - 1] = 0;
        server_send(msg);
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

int login(int *s, char *name)
{
    FILE *usr = 0;
    while ((usr = fopen("usrinfo", "r")) > 0)
    {
        char passwd[20] = {0}, hpasswd[20] = {0};
        int hasusr = 0;
        int len = read(*s, name, 10);
        name[len] = 0;
        char fbuffer[50] = {0};
        while (!hasusr && fgets(fbuffer, 50, usr))
            if (!strncmp(name, fbuffer, strlen(name)))
                hasusr = 1;
        fclose(usr);
        if (!hasusr)
        {
            write(*s, "NO", 3);
            continue;
        }
        write(*s, "OK", 3);
        strcpy(hpasswd, fbuffer + strlen(name) + 1);
        hpasswd[strlen(hpasswd) - 1] = 0;
        puts(hpasswd);
        hasusr = 3;
        while (hasusr-- && *s > 0)
        {
            len = read(*s, passwd, 20);
            passwd[len] = 0;
            if (strcmp(passwd, hpasswd))
                write(*s, "NO", 3);
            else
            {
                write(*s, "OK", 3);
                return 1;
            }
        }
        return 0;
    }
    write(*s, "server error", 13);
    return 0;
}

int sign_in(int *s, char *name)
{
    FILE *usr = 0;
    while ((usr = fopen("usrinfo", "r")) > 0 && *s > 0)
    {
        char fbuffer[50] = {0};
        int hasusr = 0;
        int len = read(*s, name, 10);
        name[len] = 0;
        while (!hasusr && fgets(fbuffer, 50, usr))
            if (!strncmp(name, fbuffer, strlen(name)))
                hasusr = 1;
        fclose(usr);
        if (!hasusr)
        {
            write(*s, "OK", 3);
            break;
        }
        else
            write(*s, "NO", 3);
    }
    char passwd[20] = {0};
    int len = read(*s, passwd, 20);
    passwd[len] = 0;
    if ((usr = fopen("usrinfo", "a")) > 0)
    {
        fprintf(usr, "%s %s\n", name, passwd);
        write(*s, "OK", 3);
        fclose(usr);
        return 1;
    }
    else
    {
        write(*s, "NO", 3);
        return 0;
    }
}

void *recv_send(void *asg)
{
    int client = *(int *)asg;
    char name[10] = {0};
    char inputext[MAXLINE] = {0};
    int flag = 0;

    write(client, "OK", 3);
    read(client, inputext, MAXLINE);
    if (!strcmp(inputext, "login"))
        flag = login(&client, name);
    else if (!strcmp(inputext, "register"))
        flag = sign_in(&client, name);
    else
    {
        close(client);
        return 0;
    }

    if (!flag || connect_count >= MAXMEM)
    {
        close(client);
        return 0;
    }

    int id = 0;
    for (id; id < MAXMEM; id++)
        if (connect_fd[id] == -1)
            break;
    if (id >= MAXMEM)
    {
        close(client);
        return 0;
    }
    connect_fd[id] = client;
    connect_count++;

    char sendtext[MAXLINE] = {0};
    sprintf(sendtext, "%s join, now online: %d", name, connect_count);
    server_send(sendtext);

    while (1)
    {
        memset(inputext, 0, sizeof(inputext));
        int len = read(connect_fd[id], inputext, MAXLINE);
        inputext[len] = 0;
        if (!strcmp(inputext, "quit"))
        {
            closesock(&connect_fd[id]);
            sprintf(sendtext, "%s offline, now online: %d", name, connect_count);
            server_send(sendtext);
            return;
        }
        else if (!strcmp(inputext, "count"))
        {
            memset(sendtext, 0, sizeof(sendtext));
            sprintf(sendtext, "now online: %d", connect_count);
            if (write(client, sendtext, strlen(sendtext)) < 0)
                closesock(&connect_fd[id]);
        }
        else if (len)
        {
            memset(sendtext, 0, sizeof(sendtext));
            sprintf(sendtext, "%s : %s", name, inputext);
            addlog(sendtext);
            for (int i = 0; i < MAXMEM; i++)
                if (connect_fd[i] > 2 && connect_fd[i]!=connect_fd[id])
                    if (write(connect_fd[i], sendtext, strlen(sendtext)) < 0)
                        closesock(&connect_fd[id]);
        }
        if (connect_fd[id] < 0)
            return 0;
    }
}
