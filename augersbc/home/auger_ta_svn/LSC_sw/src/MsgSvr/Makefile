EXEDIR = $(Services_Bin)
IDIR = $(Services_Inc)
LDIR = $(Services_Lib)
ILINCAN = $(LincanSrc)/include

MSGSVR = msgsvr.o handle_msg.o handle_can_msg.o tocdas.o handle_radio.o acklist.o

all: $(EXEDIR)/msgsvr
	echo "Done"

$(EXEDIR)/msgsvr: $(MSGSVR) #$(LDIR)/libservices.a
	$(MY_GCC) -o $@ $(MSGSVR) -L$(LDIR) -l services -lm

msgsvr.o: msgsvr.c msgsvr_version.h $(IDIR)/central_local.h $(IDIR)/msgsvr.h $(IDIR)/shmlib.h $(IDIR)/gpsstatus.h $(IDIR)/msgqueuelib.h handle_msg.h $(IDIR)/candefs.h
	$(MY_GCC) -W -Wall -c -o $@ $< -Wall -I$(IDIR) -I$(ILINCAN) --no-strict-aliasing

tocdas.o: tocdas.c tocdas.h msgsvr_version.h $(IDIR)/msgsvr.h $(IDIR)/shmlib.h $(IDIR)/gpsstatus.h $(IDIR)/msgqueuelib.h handle_msg.h $(IDIR)/candefs.h
	$(MY_GCC) -W -Wall -c -o $@ $< -Wall -I$(IDIR) -I$(ILINCAN) --no-strict-aliasing

handle_msg.o: handle_msg.c msgsvr_version.h $(IDIR)/msgsvr.h $(IDIR)/shmlib.h $(IDIR)/gpsstatus.h $(IDIR)/msgqueuelib.h handle_msg.h $(IDIR)/candefs.h
	$(MY_GCC) -c -o $@ $< -Wall -I$(IDIR) -I$(ILINCAN) --no-strict-aliasing

handle_can_msg.o: handle_can_msg.c msgsvr_version.h $(IDIR)/msgsvr.h $(IDIR)/shmlib.h $(IDIR)/gpsstatus.h $(IDIR)/msgqueuelib.h handle_can_msg.h $(IDIR)/candefs.h
	$(MY_GCC) -c -o $@ $< -Wall -I$(IDIR) -I$(ILINCAN)

handle_radio.o: handle_radio.c msgsvr_version.h  $(IDIR)/gpsstatus.h $(IDIR)/msgqueuelib.h handle_radio.h $(IDIR)/candefs.h
	$(MY_GCC) -c -o $@ $< -Wall -I$(IDIR) -I$(ILINCAN)

acklist.o: acklist.c acklist.h $(IDIR)/candefs.h msgsvr_version.h
	$(MY_GCC) -c -o $@ $< -Wall -I$(IDIR) -I$(ILINCAN)

clean:
	rm -f *.o $(EXEDIR)/msgsvr
