CC =gcc
CFLAGS =-I$(IDIR)

IDIR =../include
ODIR =obj
LDIR =../lib

LIBS=-lm

_DEPS = benchmark.h myargs.h switch.h msgbuf.h openflow.h openflow131.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = benchmark.o  myargs.o switch.o msgbuf.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ofcB: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	clear
