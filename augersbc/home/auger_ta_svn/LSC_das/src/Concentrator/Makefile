LDIR = ../../libs
BINDIR = ../../bin
IDIR = ../../include

all: $(BINDIR)/cs
	echo "Done"

$(BINDIR)/cs: concentrator.o pmconnect.o $(LDIR)/gpsutil.o $(LDIR)/logfile.o
	cc -o $@ $^

concentrator.o: concentrator.c $(IDIR)/central_local.h $(IDIR)/gpsutil.h $(IDIR)/lsx_defs.h pmconnect.h
	cc -W -Wall -o $@ -c $< -I$(IDIR)


pmconnect.o: pmconnect.c pmconnect.h
	cc -o $@ -c $< -I$(IDIR)

clean:
	rm -f *.o
