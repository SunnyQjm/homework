
PROGS = operator_serv operator_cli \
		chat_cli chat_serv


all:	${PROGS}

operator_serv:	operator_serv.o
		cc -o $@ operator_serv.o -lunp 

operator_cli:	operator_cli.o
		cc -o $@ operator_cli.o -lunp 

chat_cli: chat_cli.o		
		cc -o $@ chat_cli.o -lunp 

chat_serv: chat_serv.o		
		cc -o $@ chat_serv.o -lunp 
clean:
		rm -f ${PROGS} ${CLEANFILES}
