objects_common = fsm.o htable.o states.o
objects_rmcom = test_rmcom.o $(objects_common)
objects_stress = test_stress.o $(objects_common)
objects_thread = test_thread.o $(objects_common)

files_common = fsm.c ../../netbsd/genstructs/htable/htable.c states.c
files_rmcom = test_rmcom.c $(files_common)
files_stress = test_stress.c $(files_common)
files_thread = test_thread.c $(files_common)

libs_thread = -lpthread

dest = .
cc = gcc
cflags = -Wall -W -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith \
   -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes \
   -Wmissing-declarations -Winline -Wundef -Wnested-externs -Wcast-qual \
   -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal \
   -Wswitch -Wbad-function-cast -pedantic -ansi -g

all: test_rmcom test_stress test_thread

test_rmcom: $(objects_rmcom)
		$(cc) $(cflags) $(files_rmcom) -o $(dest)/test_rmcom

test_stress: $(objects_stress)
		$(cc) $(cflags) $(files_stress) -o $(dest)/test_stress

test_thread: $(objects_thread)
		$(cc) $(cflags) $(files_thread) $(libs_thread) -o $(dest)/test_thread

clean:
		rm -f *.o
		rm -f *~
		find . -type f -perm -111 -exec rm '{}' \;    # In Linux -111 should be /111

test_rmcom.o: fsm.h states.h types.h
test_stress.o: fsm.h states.h types.h
test_thread.o: fsm.h states.h types.h
fsm.o: fsm.h types.h
htable.o: ../../netbsd/genstructs/htable/htable.h
states.o: states.h types.h

