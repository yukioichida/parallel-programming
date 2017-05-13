EXECS=main
MPICC?=mpicc

all: ${EXECS}

main: main.c
	${MPICC} -o main main.c -lm

clean:
	rm -f ${EXECS}