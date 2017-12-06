#include "unp.h"
#include <stdlib.h>
#include <string.h>
#define USER_NUM 3
#define MAX_PWD_FAIL_COUNT 3
#define MAX_CHAT_NUM 20

char users[USER_NUM][20] = {
    "user1\n", 
    "user2\n",
    "user3\n"
};

char pwd[USER_NUM][20] = {
    "123456\n", 
    "123456\n", 
    "123456\n"
};



int sockfds[MAX_CHAT_NUM] = {0};

/**
 * 添加一个sockfd
 * @return >0 add success
 *          =0 already exist
 *          <0 add_fail, the buf already full
 */
int addSockfd(int sockfd){
    int flag = -1;
    int exist = 0;
    for(int i = 0; i < MAX_CHAT_NUM; i++){
        if(sockfds[i] == sockfd){
            exist = 1;
        }
        if(flag < 0 && sockfds[i] < 0){
            flag = i;
        }
    }
    if(exist)
        return 0;
    if(flag == -1)
        return -1;
    printf("add sockfd success!\n");
    sockfds[flag] = sockfd;
    return 1;
}

/**
 * remove sockfd from socklfds
 * @return >0 delete success
 *          <=0 delete fail, sockfds did not contain this sockfd
 */
int removeSockfd(int sockfd){
    int exist = 0;
    if(sockfd < 0)
        return -1;
    for(int i =0; i < MAX_CHAT_NUM; i++){
        if(sockfds[i] == sockfd){
            sockfds[i] = -1;
            exist = 1;    
        }
    }
    if(exist){
        printf("remove sockfd success!!\n");
        return sockfd;
    }
    return -1;
}

/**
 * 向指定的已连接的socket描述符，发送一个字符串
 */
void sendString(int sockfd, char* msg, char* buf){
    strcpy(buf, msg);
    Writen(sockfd, buf, strlen(buf));
}

/**
 * 验证用户的用户名和密码
 */
void vertify(int sockfd){
    char sendline[MAXLINE], recvline[MAXLINE];
    int flag = -1;
    int pwdFailCount = 0;
    sendString(sockfd, "Username: \n", sendline);
    while(flag < 0){
        if(Readline(sockfd, recvline, MAXLINE) == 0){
           err_quit("Client socket closed!!");
        }
        for(int i = 0; i < USER_NUM; i++){
            if(strcmp(users[i], recvline) == 0){
                flag = i;
                break;
            } 
        }
        if(flag < 0){
            sendString(sockfd, "User not exist, try again!!\n", sendline);
        }
    }

    sendString(sockfd, "Password: \n", sendline);
    while(pwdFailCount <= MAX_PWD_FAIL_COUNT){
        if(Readline(sockfd, recvline, MAXLINE) == 0){
            err_quit("Client socket closed!!");
        }
        if(strcmp(pwd[flag], recvline) == 0){   //if the pwd is correct
            //add sockfd to buf 
            sendString(sockfd, "Login Success!\n", sendline);
            addSockfd(sockfd);
            return;
        }
        // if the pwd is not correct
        sendString(sockfd, "Password not correct, try again!!\n", sendline);
    }

    err_quit("password not correct!!");
}


/**
 * 将消息广播给已有的连接
 * 除了sockfd所指示的
 * 因为sockfd是发送这个消息到服务器的客户端，所以可以不用把消息回射给它，发送给其他用户即可
 */
void brodcastMsg(int sockfd, char* msg, int length){
    for(int i = 0; i < MAX_CHAT_NUM; i++){
        if(sockfds[i] > 0 && sockfds[i] != sockfd){
            Writen(sockfds[i], msg, length);
        }
    }
}

/**
 * 处理服务器的聊天逻辑，没收到一个客户端发送的消息就转发给其他的已经连接的客户端
 * 达到群聊的效果
 */
void chat(int sockfd){
    char recvline[MAXLINE];

    for(; ;){
        if(Readline(sockfd, recvline, MAXLINE) == 0){
            removeSockfd(sockfd);
            err_quit("server socket closed!!");
        }
        brodcastMsg(sockfd, recvline, strlen(recvline));
    }
}
/*
 * 下面这段代码基本上就是第5章的回射函数的服务端代码，唯一不同的就是，各个array type has incomplete element type ‘char[]部分的功能书本上有详解（课本p98）
 * 在接收到客户端的连接以后的处理，我们调用deal来具体处理
 */
int main(){
	int listenfd, connfd;
	pid_t childpid;
    socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

    int i, maxi, maxfd, sockfd;
    int nready;
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];



	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(9748);

	Bind(listenfd, (SA*) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);


    //init sockfds
    for(int i = 0; i < MAX_CHAT_NUM; i++){
        sockfds[i] = -1;
    }

    maxfd = listenfd;
    maxi = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

	for(; ;){

        rset = allset;
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
        printf("nready: %d\n", nready);
        if(FD_ISSET(listenfd, &rset)){
            clilen = sizeof(cliaddr);
    		connfd = Accept(listenfd, (SA*) &cliaddr, &clilen);
            vertify(connfd);
            addSockfd(connfd);
            FD_SET(connfd, &allset);
            if(connfd > maxfd)
                maxfd = connfd;
            if(--nready <= 0){
                continue;
            }
        }

        for(i = 0; i < MAX_CHAT_NUM; i++){
            printf("search!\n");
            if((sockfd = sockfds[i]) < 0)
                continue;
            if(FD_ISSET(sockfd, &rset)){
                printf("%d ready\n", sockfd);
                if((Readline(sockfd, buf, MAXLINE)) == 0){
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    removeSockfd(sockfd);
                } else {
                    printf("brodcast msg: %s", buf);
                    brodcastMsg(sockfd, buf, strlen(buf));
                }
                if(--nready <= 0)
                    break;
            }
        }
	}

	
}
