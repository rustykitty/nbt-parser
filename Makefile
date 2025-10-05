LDLIBS += -lz

CFLAGS += -std=gnu99 -Wall -Wextra -pipe
CFLAGS += -O0 -g
# CFLAGS += -fsanitize=address,undefined,unreachable
# CFLAGS += -O3 -g0
# CFLAGS += -march=native

main: nbt.o nbt_parse.o nbt_traverse.o main.c zpipe
	$(CC) $(CFLAGS) -o main main.c nbt.o nbt_parse.o nbt_traverse.o $(LDLIBS)

nbt.o: nbt.c nbt.h
	$(CC) $(CFLAGS) -c nbt.c

nbt_parse.o: nbt_parse.c nbt_parse.h nbt.o
	$(CC) $(CFLAGS) -c nbt_parse.c

zpipe: zpipe.c
	$(CC) $(CFLAGS) -Wimplicit-fallthrough=0 -o zpipe zpipe.c $(LDLIBS)

nbt_traverse.o: nbt_traverse.c nbt_traverse.h
	$(CC) $(CFLAGS) -c nbt_traverse.c

.PHONY: clean

clean:
	rm -f main zpipe *.o
