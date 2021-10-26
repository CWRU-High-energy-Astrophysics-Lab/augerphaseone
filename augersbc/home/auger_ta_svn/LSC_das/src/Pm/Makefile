BINDIR =../../bin
IDIR = ../../include
LDIR = ../../libs

all:  $(BINDIR)/pm $(BINDIR)/pmup $(BINDIR)/pmdown $(BINDIR)/topm
	echo "Done"


$(BINDIR)/pm: pm.o $(LDIR)/gpsutil.o $(LDIR)/logfile.o  $(LDIR)/linklib.o
	cc -o $@ pm.o $(LDIR)/gpsutil.o $(LDIR)/logfile.o $(LDIR)/linklib.o

pm.o: pm.c
	cc -c -o $@ $< -I$(IDIR)

$(BINDIR)/pmup: pmup.o
	cc -o $@ $<

pmup.o: pmup.c
	cc -c -o $@ $< -I$(IDIR)


$(BINDIR)/pmdown: pmdown.o
	cc -o $@ $<

pmdown.o: pmdown.c
	cc -c -o $@ $< -I$(IDIR)

$(BINDIR)/topm: topm.o
	cc -o $@ $<

topm.o: topm.c
	cc -c -o $@ $< -I$(IDIR)

clean:
	rm -f *.o
