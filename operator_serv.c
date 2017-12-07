#include "unp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * 下面函数就是处理两个操作数的+-* /
 * 例如 op1 = 2, opt = '+', op2 = 3
 * 则返回 2+3
 */ 
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

/*
 * 下面的函数是一个字符串处理函数，
 * 字符串包含：只有两个操作数，一个操作符，并且操作符在中间（例如： “1+2”）
 * 
 * @Param buf       存储了表达式的字符指针（例如：“1+2”）
 * @Param n         buf中字符串的长度
 * 
 * @return float    返回表达式的计算结果
 */
float dealString(char* buf, int n){
	char temp[10];
	ssize_t t = 0;
	float op1, op2;
	char opt = 0;
	if(n > 0){	//deal data
		int i = 0;

        /**
         * 下面这循环具体处理表达式
         * 我们从左往右逐字符的扫描
         * 因为我们默认表达式的结构是两个操作数，一个操作符，并且操作符在中间，即 op1 op op2
         * 在遇到操作符之前的所有字符都因该是第一个操作数，我们把第一个操作符之前是数字或者是小数点的字符存到一个数组temp中
         * 在遇到操作符是，把之前存储在temp中的字符串通过atof转换成一个浮点数，作为第一个操作数存到op1，同时保存操作符到opt
         * 又上面的假设可以知道，操作符之后的所有字符表示第二个操作数，所以我们直接将操作符后面的所有字符放到temp中
         * 再使用atof转换成浮点数作为第二个操作数op2
         * 
         * 最后直接调用operator(op1, opt, op2) 计算表达式的结果并返回
         */
		for(i = 0; i < n; i++){
			if((buf[i] >= '0' && buf[i] <= '9') || buf[i] == '.'){
				temp[t] = buf[i];
				t++;
			} else if(buf[i] == '+' || buf[i] == '-' || buf[i] == '*' || buf[i] == '/'){
				temp[t] = '\0';
                /**
                 * atof是头文件stdlib.h中包含的一个库函数
                 * 其作用是将一个字符串转化成浮点型
                 */
				op1 = atof(temp);
				opt = buf[i];
				t = 0;
				for(i = i + 1; i < n; i++, t++){
					temp[t] = buf[i];
				}
				temp[t] = '\0';
				op2 = atof(temp);
				
				return operator(op1, opt, op2);
			} else if(buf[i] == ' '){ //忽略检测到的空格
				continue;
			}
			else {
				return 0;
			}

		}
	}
	return 0;
}

/**
 * 服务器每接收到一个来自客户端的连接，就调用下面的函数处理它
 * 
 * 具体的职能就是从客户端读取一个表达式，本地解析并计算结果，最后将计算得到的结果发送给客户端
 */
void deal(int connfd) {
    
    // ssize_t 实际上就是一个int型
    ssize_t n;
	char buf[MAXLINE];
	float result;
again:
    while((n = read(connfd, buf, MAXLINE)) > 0){
		/**
         * buf中存储了从客户端读取到的表达式字符串，其长度为n
         * 调用dealString函数可以解析这个表达式字符串，并返回这个表达式的结果，放到result当中
         */
        result = dealString(buf, n);

        /** snprintf函数的功能类似printf，也是用来做格式化输出
         * 不同的是printf是将格式化的字符串输出到控制台
         * 而snprintf是将格式化的字符串输出到指定的数组当中
         * 
         * 下面就是将result格式化为一个保留两位小数的字符串，并存储在buf中
         */
		snprintf(buf, sizeof(buf), "%.2f\n", result);
		
        /**
         * 通过上面的函数，就将服务器计算的到的结果以字符串的形式存在了buf中
         * 此时调用下面的函数将该结果发送回客户端
         */
        Writen(connfd, buf, strlen(buf));
	}

    /**
     * 下面这个判断是为了应对部分机型的电脑会由于中断信号而打断socket的读取数据的过程，导致什么都没有读到
     * 这个情况下应该重新读取
     * 
     */
	if(n < 0 && errno == EINTR){
		goto again;
	} else if(n < 0){   //如果读取错误，则报错并退出
		err_quit("read error");
	}
		
}



/*
 * 下面这段代码基本上就是第5章的回射函数的服务端代码，唯一不同的就是，各个部分的功能书本上有详解（课本p98）
 * 在接收到客户端的连接以后的处理，我们调用deal来具体处理
 */
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

		if((childpid = Fork()) == 0){
			Close(listenfd);
			deal(connfd);
			exit(0);
    	}
		Close(connfd);
	}
	
}
