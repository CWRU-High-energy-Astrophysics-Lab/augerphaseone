LIBDIR = ../../libs
INCDIR = ../../include

all: $(LIBDIR)/logfile.o $(LIBDIR)/gpsutil.o $(LIBDIR)/linklib.o
	echo "Done"

$(LIBDIR)/logfile.o: logfile.c $(INCDIR)/logfile.h $(INCDIR)/logfile_p.h
	cc -c -o $@ $< -I$(INCDIR)

$(LIBDIR)/linklib.o: linklib.c $(INCDIR)/linklib.h
	cc -c -o $@ $< -I$(INCDIR)

$(LIBDIR)/gpsutil.o: gpsutil.c $(INCDIR)/gpsutil.h
	cc -c -o $@ $< -I$(INCDIR)

clean:
	rm -f $(LIBDIR)/*.o
