CC=gcc
CFLAGS=-Wall -O3

SRC=dberror.c expr.c btree_mgr.c bplustree.c record_mgr.c buffer_mgr.c buffer_mgr_stat.c \
	rm_serializer.c storage_mgr.c rs_fifo.c rs_clock.c rs_lfu.c rs_lru.c rs_lru_k.c linked_list.c

all: test_bplustree test_assign4_1

test_bplustree: test_bplustree.c bplustree.c
	$(CC) $(CFLAGS) -o test_bplustree.elf $^

test_assign4_1: test_assign4_1.c $(SRC)
	$(CC) $(CFLAGS) -o test_assign4_1.elf $^

clean:
	rm -f *.elf
