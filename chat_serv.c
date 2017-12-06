#include "unp.h"
#include <stdlib.h>
#include <string.h>
#define USER_NUM 3
#define MAX_PWD_FAIL_COUNT 3
#define MAX_CHAT_NUM 20

#define STATE_INIT '0'
#define STATE_USER_NAME_ALREADY '1'
#define STATE_LOGIN_SUCCESS '2'


char sendBuf[MAXLINE];
struct UserRecord{
    int sockfd;
    char username[20];
    char pwd[20];
    char state;
    int failNum; 
} records[] = {
    {-1, "user1", "123456", STATE_INIT, 0}, 
    {-1, "user2", "123456", STATE_INIT, 0},
    {-1, "user3", "123456", STATE_INIT, 0}
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

void sendWithCode(int sockfd, char* msg, char* code, char*buf){
    strcpy(buf, msg);
    strcat(buf, code);
    //int size = strlen(msg) + strlen(code);
    Writen(sockfd, buf, strlen(buf));
}   

/**
 * 将消息广播给已有的连接
 * 但是不广播给自己
 * 因为sockfd是发送这个消息到服务器的客户端，所以可以不用把消息回射给它，发送给其他用户即可
 */
void brodcastMsg(int sockfd, char* msg, int length){
    printf("broad msg: %s", msg);
    int from = -1;
    for(int i = 0; i < USER_NUM; i++){
        if(sockfd == records[i].sockfd){
            from = i;
            break;
        }
    }
    for(int i = 0; i < USER_NUM; i++){
        if(records[i].sockfd < 0 || records[i].sockfd == sockfd){
            printf("-1\n");
            continue;
        }
        printf("sendto: %d\n", records[i].sockfd);
        sendWithCode(records[i].sockfd, records[from].username, ": ", sendBuf);
        sendWithCode(records[i].sockfd, msg, "2\n", sendBuf); 
    }
}

/**
 * 验证用户名
 */
void vertifyName(int sockfd, char* username){
    for(int i = 0; i < USER_NUM; i++){
        if(strcmp(records[i].username, username) == 0){
            if(records[i].state != STATE_INIT || records[i].sockfd > 0){    //用户已经登录，不允许重复登录
                sendWithCode(sockfd, "User already login! try again!", "\n0\n", sendBuf);
                return; 
            } else {    //静态表里面有这个用户（可以认为这个用户名是已注册的，是有效的）
                records[i].sockfd = sockfd;
                records[i].state = STATE_USER_NAME_ALREADY;
                sendWithCode(sockfd, "Password: ", "\n1\n", sendBuf);
                return;
            }
        }
    }
    sendString(sockfd, "User not exist! Try again!\n0\n", sendBuf);
}

/**
 * 验证密码
 */
void vertifyPwd(int sockfd, char* password){
    for(int i = 0; i < USER_NUM; i++){
        if(sockfd == records[i].sockfd){
            if(strcmp(records[i].pwd, password) == 0){   //验证通过
                records[i].sockfd = sockfd;
                records[i].state = STATE_LOGIN_SUCCESS;
                sendWithCode(sockfd, "Login Success!!", "\n2\n", sendBuf);
            } else if(records[i].failNum < MAX_PWD_FAIL_COUNT){ //密码错误，但是错误次数还不多，允许再次输入
                records[i].failNum++;
                sendString(sockfd, "Password not correct, try again!\n1\n", sendBuf);
            } else {    //密码错了太多次，要求重新输入用户名
                records[i].failNum = 0;
                records[i].sockfd = -1;
                records[i].state = STATE_INIT;
                sendString(sockfd, "Password not correct too much time! Please input a new username\n0\n", sendBuf);
            }
        }
    }
}

/**
 * 用户断开连接后，清空该用户的信息
 */
void clearUser(int sockfd){
    for(int i = 0; i < USER_NUM; i++){
        if(records[i].sockfd == sockfd){
            records[i].state = STATE_INIT;
            records[i].failNum = 0;
            records[i].sockfd = -1;
        }
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

    int state;  //暂存用户的状态

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
          //  vertify(connfd);
            addSockfd(connfd);
            sendWithCode(connfd, "Username: ", "\n0\n", buf);
            FD_SET(connfd, &allset);
            if(connfd > maxfd)
                maxfd = connfd;
            if(--nready <= 0){
                continue;
            }
        }

        for(i = 0; i < MAX_CHAT_NUM; i++){
            if((sockfd = sockfds[i]) < 0)
                continue;
            if(FD_ISSET(sockfd, &rset)){
                printf("%d ready\n", sockfd);
                if((n = Read(sockfd, buf, MAXLINE)) == 0){
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    removeSockfd(sockfd);
                    clearUser(sockfd);
                } else {
                    //printf("code: %c", buf[n - 2]);
                    //printf("brodcast msg: %s", buf);
                    //brodcastMsg(sockfd, buf, strlen(buf));
                    state = buf[n - 2];
                    buf[n - 2] = '\0';
                    switch(state){
                    case STATE_INIT:
                        vertifyName(sockfd, buf);
                        break;
                    case STATE_USER_NAME_ALREADY:
                        vertifyPwd(sockfd, buf);
                        break;
                    case STATE_LOGIN_SUCCESS:
                        brodcastMsg(sockfd, buf, strlen(buf));
                        break;
                    }
                    memset(buf, 0, sizeof(buf));
                }
                if(--nready <= 0)
                    break;
            }
        }
	}

	
}
