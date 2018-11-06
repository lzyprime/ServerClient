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

#define MAXLINE 2048
#define SERVERPORT 16666
#define SERVERADDR "127.0.0.1"

int socket_fd;

int login();
int sign_in();
void *toserver();

int main(int argc, char const *argv[])
{
    struct sockaddr_in servaddr;

    // creat socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket create failed\n");
        return -1;
    }

    // server addr
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if (argc == 3)
        servaddr.sin_port = htons(argv[2]);
    else
        servaddr.sin_port = htons(SERVERPORT);
    if (argc >= 2 && inet_aton(argv[1], &servaddr.sin_addr) < 0 || argc < 2 && inet_aton(SERVERADDR, &servaddr.sin_addr) < 0)
    {
        printf("inet_aton error.\n");
        return -1;
    }

    // connect()
    printf("Connecting...\n");
    if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Connect server failed.\n");
        return -1;
    }

    // --- login or register --- //
    char inputext[MAXLINE] = {0};
    read(socket_fd, inputext, MAXLINE);
    if (strcmp(inputext, "OK"))
    {
        puts(inputext);
        close(socket_fd);
        exit(0);
    }

    printf("1.login\n2.register\n");
    int flag = 0;
    while (~scanf("%d", &flag))
    {
        printf("%d", flag);
        if (flag == 1 || flag == 2)
            break;
        puts("1.login\n2.register");
    }

    if (flag != 1 && flag != 2)
    {
        close(socket_fd);
        exit(0);
    }

    if (flag == 1)
    {
        write(socket_fd, "login", 6);
        puts("start login");
        flag = login();
    }
    else
    {
        write(socket_fd, "register", 9);
        flag = sign_in();
    }

    if (!flag)
    {
        close(socket_fd);
        exit(0);
    }

    // listen server
    pthread_create(malloc(sizeof(pthread_t)), NULL, toserver, NULL);

    char server_in[MAXLINE] = {0};
    while (1)
    {
        memset(inputext, 0, sizeof(inputext));
        int len = read(socket_fd, server_in, MAXLINE);
        server_in[len] = 0;
        if (!strlen(inputext))
            inputext[len] = 0, puts(server_in);
        if (!strcmp(server_in, "server: quit"))
        {
            close(socket_fd);
            exit(0);
        }
    }
    return 0;
}

void *toserver()
{
    char inputext[MAXLINE];
    while (1)
    {
        memset(inputext, 0, sizeof(inputext));
        fgets(inputext, MAXLINE, stdin);
        inputext[strlen(inputext) - 1] = 0;
        if (strlen(inputext))
            write(socket_fd, inputext, strlen(inputext));
        if (!strcmp(inputext, "quit"))
        {
            close(socket_fd);
            exit(0);
        }
    }
    exit(0);
}

int login()
{
    char input[20] = {0};
    char server[4] = {0};
    int flg = 2;
    while (socket_fd > 0 && flg)
    {
        if (flg == 2)
            printf("usrname: ");
        else
            printf("passwd: ");
        scanf("%s", input);
        puts(input);
        write(socket_fd, input, strlen(input));
        read(socket_fd, server, 3);
        puts(server);
        if (!strcmp(server, "OK"))
            flg--;
    }
    if (!flg)
        return 1;
    return 0;
}

int sign_in()
{
    char input[20] = {0};
    char server[4] = {0};
    int flag = 2;
    while (flag && socket_fd > 0)
    {
        if (flag == 2)
            printf("usrname: ");
        else
            printf("passwd: ");
        scanf("%s", input);
        write(socket_fd, input, strlen(input));
        read(socket_fd, server, 4);
        if (!strcmp(server, "OK"))
            flag--;
    }
    if (!flag)
        return 1;
    else
        return 0;
}