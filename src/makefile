CC =gcc
CFLAGS =-I$(IDIR)

IDIR =../include
ODIR =obj
LDIR =../lib

LIBS=-lloci

_DEPS = benchmark.h myargs.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = benchmark.o  myargs.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ofcB: $(OBJ)
	gcc -L$(LDIR) -Wl,-rpath=$(LDIR) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
