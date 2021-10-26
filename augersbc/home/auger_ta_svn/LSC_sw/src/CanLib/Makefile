LDIR = $(Services_Lib)
IDIR = $(Services_Inc)
# Lincan driver include directory
ILINCAN = $(LincanSrc)/include

all: $(LDIR)/canlib.o
	echo "Done"

$(LDIR)/canlib.o: canlib.c canlib_version.h $(IDIR)/candefs.h
	$(MY_GCC) -c -o $@ $< -I$(IDIR) -I$(ILINCAN)

clean:
	rm -f *.o $(LDIR)/canlib.o
