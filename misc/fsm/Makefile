objects_rmcom = test_rmcom.o fsm.o htable.o states.o
objects_stress = test_stress.o fsm.o htable.o states.o
objects_thread = test_thread.o fsm.o htable.o states.o


files_rmcom = test_rmcom.c fsm.c htable.c states.c
files_stress = test_stress.c fsm.c htable.c states.c
files_thread = test_thread.c fsm.c htable.c states.c

libs_thread = -lpthread

dest = .
cc = gcc
cflags = -Wall -W -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith \
   -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes \
   -Wmissing-declarations -Winline -Wundef -Wnested-externs -Wcast-qual \
   -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal \
   -Wswitch -Wbad-function-cast -pedantic -ansi -g

test_rmcom : $(objects_rmcom)
		$(cc) $(cflags) $(files_rmcom) -o $(dest)/test_rmcom

test_stress: $(objects_stress)
		$(cc) $(cflags) $(files_stress) -o $(dest)/test_stress

test_thread: $(objects_thread)
		$(cc) $(cflags) $(files_thread) $(libs_thread) -o $(dest)/test_thread

clean:
		rm *.o

test_rmcom.o: fsm.h states.h types.h
test_stress.o: fsm.h states.h types.h
test_thread.o: fsm.h states.h types.h
fsm.o: fsm.h types.h
htable.o: htable.h
states.o: states.h types.h

