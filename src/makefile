CC =gcc
CFLAGS =-I$(IDIR)

IDIR =../include
ODIR =obj
LDIR =../lib

LIBS=`net-snmp-config --libs` -lm -lrt -pthread

_DEPS =  myargs.h benchmark.h mysnmp.h myreport.h mymessages.h myserver.h myclient.h myswitch.h msgbuf.h openflow.h openflow131.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ =  myargs.o benchmark.o mysnmp.o myreport.o mymessages.o myserver.o myclient.o myswitch.o msgbuf.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ofcB: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) -fpermissive

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	clear
