#include "unp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

float operator(float op1, char opt, float op2){
	switch(opt){
		case '+':
			return op1 + op2;
		case '-':
			return op1 - op2;
		case '*':
			return op1 * op2;
		case '/':
			if((fabs(op2 - 0)) < 10e-6){
				return -1;
			}
			return op1 / op2;
		default:
			return 0;
	}
}

float dealString(char* buf, int n){
	char temp[10];
	ssize_t t = 0;
	float op1, op2;
	char opt = 0;
	if(n > 0){	//deal data
		int i = 0;
		for(i = 0; i < n; i++){
			if((buf[i] >= '0' && buf[i] <= '9') || buf[i] == '.'){
				temp[t] = buf[i];
				t++;
			} else if(buf[i] == '+' || buf[i] == '-' || buf[i] == '*' || buf[i] == '/'){
				temp[t] = '\0';
				op1 = atof(&temp);
				opt = buf[i];
				t = 0;
				for(i = i + 1; i < n; i++, t++){
					temp[t] = buf[i];
				}
				temp[t] = '\0';
				op2 = atof(&temp);
				
				return operator(op1, opt, op2);
			} else if(buf[i] == ' '){
				continue;
			}
			else {
				return 0;
			}

		}
	}
	return 0;
}

void deal(int connfd) {
	ssize_t n;
	char buf[MAXLINE];
	float result;
again:
    while((n = read(connfd, buf, MAXLINE)) > 0){
		result = dealString(buf, n);
		snprintf(buf, sizeof(buf), "%.2f\n", result);
		Writen(connfd, buf, strlen(buf));
	}
	if(n < 0 && errno == EINTR){
		goto again;
	} else if(n < 0){
		err_quit("read error");
	}
		
}
mywrite(int connfd, char* ptr){
	Writen(connfd, ptr, strlen(ptr));
}

/*int main(int argc, char** argv){
	printf("%f\n", dealString(argv[1], strlen(argv[1])));
}*/

int main(){
	int listenfd, connfd;
	pid_t childpid;
    socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(9748);

	Bind(listenfd, (SA*) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for(; ;){
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA*) &cliaddr, &clilen);

	/*	if((childpid = Fork()) == 0){
			Close(listenfd);*/
			deal(connfd);
			/*exit(0);*/
	/*	}*/
		Close(connfd);
	}
	
}
