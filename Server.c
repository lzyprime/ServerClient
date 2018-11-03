#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

time_t tt;

void addlog(char text[])
{
    FILE *log_fd = 0;
    if ((log_fd = fopen("./socket.log", "a")) != 0)
        tt = time(0), fprintf(log_fd, "%s %s\n", ctime(&tt), text);
    printf("%s \n", text);
    fclose(log_fd);
}

int main()
{
    // int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockaddr < 0)
    for (int i = 0; i < 3; i++)
    {
        char a[2] = {0};
        a[0] = i + '0';
        addlog(a);
        sleep(2);
    }
    return 0;
}