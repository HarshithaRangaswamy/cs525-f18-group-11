CC=gcc
CFLAGS=-O3 -Wall

default: test_1

test_1: test_assign3_1.c buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o expr.o record_mgr.o rm_serializer.o
	$(CC) $(CFLAGS) -o test_1 test_assign3_1.c buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o expr.o record_mgr.o rm_serializer.o

test_expr: test_expr.c dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o
	$(CC) $(CFLAGS) -o test_expr test_expr.c dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o

record_mgr.o: record_mgr.c
	$(CC) $(CFLAGS) -c -o record_mgr.o record_mgr.c

expr.o: expr.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o expr.o expr.c

rm_serializer.o: rm_serializer.c
	$(CC) $(CFLAGS) -c rm_serializer.c -o rm_serializer.o

buffer_mgr.o: buffer_mgr.c
	$(CC) $(CFLAGS) -c -o buffer_mgr.o buffer_mgr.c

storage_mgr.o: storage_mgr.c
	$(CC) $(CFLAGS) -c -o storage_mgr.o storage_mgr.c

dberror.o: dberror.c dberror.h
	$(CC) $(CFLAGS) -c -o dberror.o dberror.c

buffer_mgr_stat.o: buffer_mgr_stat.c
	$(CC) $(CFLAGS) -c -o buffer_mgr_stat.o buffer_mgr_stat.c

clean:
	$(RM) test_1 test_expr *.o

run:
	./test_1

run_expr:
	./test_expr
