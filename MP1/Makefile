CC=gcc 
CFLAGS=-I.

OBJ = main.o input.o command.o prompt.o hop.o reveal.o log.o proclore.o seek.o mysignal.o

shell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o shell

.PHONY: clean
