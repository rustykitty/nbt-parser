LDLIBS += -lz

# CFLAGS += -O3
# CFLAGS += -march=native
CFLAGS += -std=gnu99 -Wall -Wextra
CFLAGS += -O0 -g
# CFLAGS += -fsanitize=address,undefined,unreachable

main: nbt.o nbt_parser.o main.c
	$(CC) $(CFLAGS) -o main main.c nbt.o nbt_parser.o $(LDLIBS)

nbt.o: nbt.c nbt.h
	$(CC) $(CFLAGS) -c nbt.c

nbt_parser.o: nbt_parser.c nbt_parser.h nbt.o
	$(CC) $(CFLAGS) -c nbt_parser.c

.PHONY: clean test

clean:
	rm -f main *.o

test: main
	./main nbt/bigtest.nbt