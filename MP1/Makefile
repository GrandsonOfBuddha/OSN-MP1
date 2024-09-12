CC=gcc 
CFLAGS=-I.

OBJ = main.o input.o command.o prompt.o hop.o reveal.o log.o proclore.o seek.o mysignal.o redirection.o pipe.o myshrc.o activities.o ping.o fg_bg.o neonate.o iman.o

shell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o shell

.PHONY: clean
