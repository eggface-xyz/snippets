SRC_UTIL  = sock_util.c
SRC_SELECT = select.c $(SRC_UTIL)
OBJ_SELECT = $(patsubst %.c, %.o, $(SRC_SELECT))

PROG_SELECT = select
CC = g++

$(PROG_SELECT): $(OBJ_SELECT)
	$(CC) -o $@ $^

.PHONY: clean

clean:
	$(RM) $(PROG_SELECT) $(OBJ_SELECT)
