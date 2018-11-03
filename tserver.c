#include<time.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h> 
#include<string.h>
#include<stdlib.h>

#define WAITNUM 5
#define MAXLINE 2048
#define MAXMEM 10
#define NAMELEN 20

int socket_fd;
int connect_fd[MAXMEM];
int a;
void quit();
void recv_send(void* arg);

int main()
{
	pthread_t thread;
	struct sockaddr_in servaddr, client_addr;
	socklen_t len;
    time_t ticks;
    char buff[MAXLINE];


    printf("Socket...\n");
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        printf("Socket created failed.\n");
        return -1;
    }


    printf("Bind...\n");
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(16666);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Bind failed.\n");
        return -1;
    }

    printf("listening...\n");
    
    if(listen(socket_fd, WAITNUM) < 0)
    {
	printf("listen failed.\n");
        return -1;
    }

    pthread_create(&thread, NULL, (void*)(&quit), NULL);
    printf("create_quit_pthread\n");
    
	int i;
    for(i=0;i<MAXMEM;i++)
    {
        connect_fd[i]=-1;
    }
    memset(&client_addr, 0, sizeof(client_addr));	
    while(1)
    {
        len = sizeof(client_addr);
        for(i=0; i < MAXMEM; i++)
        {
            if(connect_fd[i]==-1)
            {
                break;
            }
        }

        connect_fd[i] = accept(socket_fd, (struct sockaddr*)&client_addr, &len);

		if (connect_fd[i] == -1 )
		{
			printf("accept failed\n");
		}

        ticks = time(NULL);
        printf("%s Connect from: %s\n", ctime(&ticks), 
			   inet_ntoa(client_addr.sin_addr));
	    
        printf("user id is %d join\n",i);
        a = i;
        pthread_create(malloc(sizeof(pthread_t)), NULL, (void*)(&recv_send), (void*)(&a));

    }
    return 0;
}

void quit()
{
    char msg[10];
    while(1)
    {
        scanf("%s",msg);
        if(strcmp("quit",msg)==0)
        {
            printf("Byebye...\n");
            close(socket_fd);
            exit(0);
        }
    }
}

void recv_send(void *arg)
{
    int id = *(int *)arg;
    char* ask=">>>>>Your name please:";
    char buff[MAXLINE];
    char recv_buff[MAXLINE];
    char time_buff[MAXLINE];
    char name[NAMELEN];
    time_t ticks;
    
    int retval;
    printf("id is %d\n",id);
    write(connect_fd[id], ask, strlen(ask));
    int len;
    len = read(connect_fd[id], name, NAMELEN);
    if(len > 0)
    {
         name[len] = '\0';
    }

    strcpy(buff, name);
    strcat(buff, "join in\0");
    int i;
    for(i=0;i<MAXMEM;i++)
    {
        if(connect_fd[i]!=-1)
        {
            write(connect_fd[i],buff,strlen(buff));
        }
    }

    while(1)
    {
        if((len = read(connect_fd[id], recv_buff, MAXLINE))>0)
        {
            recv_buff[len]=0;

             if(strcmp("bye", recv_buff) == 0)
             {
                 close(connect_fd[id]);
                 connect_fd[id]=-1;
                 pthread_exit(&retval);
             }

             ticks=time(NULL);
             sprintf(time_buff,"%.24s\r\n",ctime(&ticks));
             write(connect_fd,time_buff,strlen(time_buff));

             strcpy(buff,name);
             strcat(buff,"---");
             strcat(buff,time_buff);
             strcat(buff,recv_buff);

            for(i=0;i<MAXMEM;i++)
            {
                 if(connect_fd[i]!=-1)
                 {
                      write(connect_fd[i],buff,strlen(buff));
                 }
            }
        }

    }
}

