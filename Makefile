
PROGS = operator_serv operator_cli


all:	${PROGS}

operator_serv:	operator_serv.o
		cc -o $@ operator_serv.o -lunp 

operator_cli:	operator_cli.o
		cc -o $@ operator_cli.o -lunp 

clean:
		rm -f ${PROGS} ${CLEANFILES}
