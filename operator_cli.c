#include "unp.h"


/**
 * 处理客户端的请求
 * 向服务器发送一个表达式（例如：1+1）
 * 然后服务器会发回表达式的结果，客户端接收它并显示出来（例如：2）
 * 
 * @Param fp        一个文件指针（如果传入stdin则表示从控制台读取）
 * @Param sockfd    一个成功连接到服务器的Socket 的描述符
 */
void operator_cli(FILE * fp, int sockfd){
	
    //定义两个char数组
    //sendline 用来存从控制台输入的字符串
    //recvline 用来存从服务器返回的字符串
    char sendline[MAXLINE], recvline[MAXLINE];

    /**
     * 从控制台循环输入表达式，并传送到服务器端
     * 接收从服务器端返回的结果并打印到控制台
     * 
     * ps: Fgets是书作者定义的一个包装函数，作用是从fp指向的文件中读取一行字符串，每次读取最多为MAXLINE
     */
	while(Fgets(sendline, MAXLINE, fp) != NULL){
        
        /**
         * 下面这个函数也是课本作者定义的一个包装函数，功能是向sockfd所指向的对端设备发送sendline中的字符串
         * 通过上面的Fgets从控制台读取到表达式（例如：1+1）
         * 然后通过下面的这个函数把读取到的表达式传到服务器端（连接成功以后sockfd所指向的对端设备指的就是服务器端）
         */
		Writen(sockfd, sendline, strlen(sendline));
		
        /**
         * 下面的Readline函数类似，是从sockfd所指向的对端设备中接收字符串到recvline中，一次最多读取MAXLINE
         * 如果返回值是0，表示服务器端断开，输出错误并推出程序
         */
        if(Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("operator_cli: server terminated prematurely");
	
        //将从服务器上接收到的字符串打印到控制台
        Fputs(recvline, stdout);
	}
}

int main(int argc, char **argv){
	//Socket 文件描述符
    int sockfd;
    //下面这个结构体用来存储一个IP地址
	struct sockaddr_in servaddr;

    /**
     * 对控制台输入参数的判断：调用格式如==> operator_cli 127.0.0.1
     */
	if(argc != 2)
		err_quit("usage: operator_cli <IPaddress>\n");
	
    //创建一个TCP Socket
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    //将servaddr置0， 可以认为置0以后，该结构体里面存储的ip地址为0.0.0.0
	bzero(&servaddr, sizeof(servaddr));

    //指定服务器地址的协议族为 AF_INET(TCP/IP协议族)
	servaddr.sin_family = AF_INET;
    //指定服务器地址的端口
	servaddr.sin_port = htons(9748);

    //下面这一行将控制台输入的第二个参数（即IP地址），存放到地址结构体当中
    //emm，里面可能涉及主机字节序转网络字节序什么的，但是不用关心
    //只要知道调用下面的函数可以把形如“127.0.0.1”这样的字符串存放到servaddr这个结构体当中
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    /**
     * 下面这一行是客户端请求连接servaddr所指向的服务器（根据结构体里面保存的地址和端口）
     */
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    
    /**
     * 下面是成功连接到服务器以后，处理具体的逻辑，这是一个自定义的函数
     * 即输入一个表达式传送到服务器端，
     * 再从服务器端读取这个表达式的结果打印出啦
     */
	operator_cli(stdin, sockfd);
	exit(0);
}
