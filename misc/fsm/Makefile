objects = main.o fsm.o htable.o states.o
files = main.c fsm.c htable.c states.c
dest = .
cc = gcc
cflags = -Wall -W -Wformat-nonliteral -Wcast-align -Wpointer-arith \
   -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes \
   -Wmissing-declarations -Winline -Wundef -Wnested-externs -Wcast-qual \
   -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal -pedantic -ansi

fsm : $(objects)
		$(cc) $(cflags) $(files) -o $(dest)/fsm

clean :
		rm *.o

main.o: fsm.h states.h types.h
fsm.o: fsm.h types.h
htable.o: htable.h
states.o: states.h types.h

