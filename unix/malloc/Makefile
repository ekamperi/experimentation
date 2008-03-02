objects_common = mpool.o
files_common = mpool.c
objects1 = test1.o $(objects_common)
objects2 = test2.o $(objects_common)
objects3 = test3.o mstat.o $(objects_common)
objects4 = test4.o mstat.o $(objects_common)

files1 = test1.c $(files_common)
files2 = test2.c $(files_common)
files3 = test3.c mstat.c $(files_common)
files4 = test4.c mstat.c $(files_common)

dest = .
cc = gcc
cflags = -Wall -W -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith \
   -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes \
   -Wmissing-declarations -Winline -Wundef -Wnested-externs -Wcast-qual \
   -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal \
   -Wswitch -Wbad-function-cast -pedantic -ansi -g

all: test1 test2 test3 test4

test1: $(objects1)
		$(cc) $(cflags) $(files1) -o $(dest)/test1

test2: $(objects2)
		$(cc) $(cflags) $(files2) -o $(dest)/test2

test3: $(objects3)
		$(cc) $(cflags) $(files3) -o $(dest)/test3

test4: $(objects4)
		$(cc) $(cflags) $(files4) -o $(dest)/test4

clean:
		rm -f *.o
		rm -f *~
		find . -type f -perm /111 -exec rm '{}' \;    # In NetBSD /111 should be -111

test1.o: mpool.h
test2.o: mpool.h
test3.o: mpool.h mstat.h
test4.o: mpool.h mstat.h
mstat.o: mstat.h



