EXEDIR = $(Acq_Bin)
ISERVDIR = $(Services_Inc)
IACQDIR = $(Acq_Inc)
LACQDIR = $(Acq_Lib)
LSERVDIR = $(Services_Lib)

all: $(EXEDIR)/evtsvr
	echo "Done"


$(EXEDIR)/evtsvr: evtsvr.o docompress.o $(LSERVDIR)/libservices.a $(LACQDIR)/libacq.a
	$(MY_GCC) -o $@ evtsvr.o docompress.o -L$(LSERVDIR) -L$(LACQDIR) -lacq -lservices -lm

evtsvr.o: evtsvr.c evtsvr_version.h \
	$(ISERVDIR)/msgsvr.h $(ISERVDIR)/msgqueuelib.h $(ISERVDIR)/central_local.h \
	$(IACQDIR)/acq_msg.h $(ISERVDIR)/timestamp.h $(IACQDIR)/events.h
	$(MY_GCC) -W -Wall -c -o $@ $<  -I$(ISERVDIR) -I$(IACQDIR)

docompress.o: docompress.c
	$(MY_GCC) -W -Wall -c -o $@ $< -I$(ISERVDIR) -I$(IACQDIR)

clean:
	rm -f *.o $(EXEDIR)/evtsvr
