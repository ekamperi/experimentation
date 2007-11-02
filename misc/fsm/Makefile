objects_rmcom = test_rmcom.o fsm.o htable.o states.o
objects_stress = test_stress.o fsm.o htable.o states.o
files_rmcom = test_rmcom.c fsm.c htable.c states.c
files_stress = test_stress.c fsm.c htable.c states.c
dest = .
cc = gcc
cflags = -Wall -W -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith \
   -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes \
   -Wmissing-declarations -Winline -Wundef -Wnested-externs -Wcast-qual \
   -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal \
   -Wswitch -Wbad-function-cast -pedantic -ansi -g

test_rmcom : $(objects)
		$(cc) $(cflags) $(files_rmcom) -o $(dest)/test_rmcom

test_stress: $(objects_stress)
		$(cc) $(cflags) $(files_stress) -o $(dest)/test_stress

clean :
		rm *.o

rmcom.o: fsm.h states.h types.h
fsm.o: fsm.h types.h
htable.o: htable.h
states.o: states.h types.h

