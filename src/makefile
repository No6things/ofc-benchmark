CC =gcc
CFLAGS =-I$(IDIR)

IDIR =../include
ODIR =obj
LDIR =../lib

LIBS=-lm -lloci

_DEPS = benchmark.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = benchmark.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(OBJ)
	gcc -L$(LDIR) -Wl,-rpath=$(LDIR) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
