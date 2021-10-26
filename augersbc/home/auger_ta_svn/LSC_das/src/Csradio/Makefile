IDIR = ../../include
LDIR = ../../libs
BINDIR = ../../bin

PROGRAMS = $(BINDIR)/csradio $(BINDIR)/tocr

all: $(PROGRAMS)
	echo "finished"

$(BINDIR)/csradio: csradio.o canlib.o $(LDIR)/logfile.o $(LDIR)/gpsutil.o
	cc -o $@ csradio.o $(LDIR)/logfile.o $(LDIR)/gpsutil.o canlib.o 

$(BINDIR)/tocr: tocr.o msgqueuelib.o
	cc -o $@ tocr.o msgqueuelib.o

tocr.o: tocr.c
	cc -c -o $@ $< -I$(IDIR)

msgqueuelib.o: msgqueuelib.c
	cc -c -o $@ $< -I$(IDIR)

csradio.o: csradio.c csradio_version.h csdefs.h $(IDIR)/candefs.h
	cc -c -o $@ $< -I$(IDIR)

canlib.o: canlib.c canlib_version.h
	cc -c -o $@ $< -I$(IDIR)

clean:
	rm -f *.o
