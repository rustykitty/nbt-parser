LDLIBS += -lz

# CFLAGS += -O3
# CFLAGS += -march=native
CFLAGS += -std=gnu99 -Wall -Wextra -pipe
CFLAGS += -O0 -g
# CFLAGS += -fsanitize=address,undefined,unreachable

main: nbt.o nbt_parse.o zpipe.o main.c
	$(CC) $(CFLAGS) -o main main.c nbt.o nbt_parse.o zpipe.o $(LDLIBS)

nbt.o: nbt.c nbt.h
	$(CC) $(CFLAGS) -c nbt.c

nbt_parse.o: nbt_parse.c nbt_parse.h nbt.o
	$(CC) $(CFLAGS) -c nbt_parse.c

zpipe.o: zpipe.c zpipe.h
	$(CC) $(CFLAGS) -Wimplicit-fallthrough=0 -c zpipe.c

.PHONY: clean

clean:
	rm -f main *.o