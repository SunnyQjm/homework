#include "unp.h"


/**
 * 处理客户端的请求
 * 向服务器发送一个表达式（例如：1+1）
 * 然后服务器会发回表达式的结果，客户端接收它并显示出来（例如：2）
 */
void operator_cli(FILE * fp, int sockfd){
	char sendline[MAXLINE], recvline[MAXLINE];
	while(Fgets(sendline, MAXLINE, fp) != NULL){
		Writen(sockfd, sendline, strlen(sendline));
		if(Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("operator_cli: server terminated prematurely");
		Fputs(recvline, stdout);
	}
}

int main(int argc, char **argv){
	int sockfd;
	struct sockaddr_in servaddr;

	if(argc != 2)
		err_quit("usage: operator_cli <IPaddress>\n");
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9748);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	operator_cli(stdin, sockfd);
	exit(0);

}
