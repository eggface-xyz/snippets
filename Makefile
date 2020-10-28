SRC = select.c
PROG = select
CC = g++

$(PROG): $(SRC)
	$(CC) -o $@ $^

.PHONY: clean

clean:
	$(RM) $(PROG)
