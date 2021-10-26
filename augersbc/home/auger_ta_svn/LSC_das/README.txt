Laurent Guglielmi - 2015/06/19
--------------------------

This directory tree contains the LSC "micro CDAS':
* Pm
    + pm: a version of Pm + some Cdas capabilities.
    + topm: an interactive interface to send commands to pm.
    + pmup,pmdown: upload and download files to/from LSC. Files are
    uploaded and downloaded in "slices". Slices are handled by pmup
    and pmdown.

* Concentrator
  + cs: the interface between pm and csradio,

* Csradio
  + csradio: emulator of the RDA local station radio (name LR in the RDA jargon)
     Receive messages from cs, transforms into CAN frames sent to LSC.
     Read CAN frames from LSC, build messages and passes to cs
     In additon, handles the radio protocole: answer to requests from LSC, send to LSC network status,
     and other types of messages.


NOTE: compilation of csradio for ARM linux and socketcan was a bit tricky (cross compilation on my
Mac, wrong version of linux (lincan instead of socketcan).
Some tuning in the Makefile and the source may be necessary.

Usage
-----

The first step is to launch csradio. The output sould be "CAN socket
created". It does not mean that everything is OK, but at least there
is a good chance.
Next step is to launch pm and Cs in a second ssh session (just to
avoid merging of output with csradio): startpm (a shell script in directory
bin).

In principle csradio sould print: "Concentrator Connected"

Now the operations can be started.

It is necessary to start a nwe ssh session.
The program to use to communicate with the LSC is "topm".

There are many commands that can be sent to the LSC with topm, type
'topm -?' to have a list. Note that topm can be used either
interactively (just type'topm' and type the commands at the prompt) or
the commands can be sent on the command line 'topm T' for exemple to
start triggering.

The first command to try could be: w (wakeup message); the LSC should
respond with its status. The answer appears in the PM/Cs ssh session
output.

If it's OK then the next step could be to start triggering (if not yet
started) with th command T (start Triggering). The T2 data should
appear in te PM/Cs output (if it's not the cse, try the command 'v' -
"toggle T2 print").
The command 'St' starts saving T2 data to a file ('st' to stop
saving). The file name is "t2_yyyymmdd_hhmmss.dat".

There is a posibility to retrieve some events (aka T3 events) with the
command 'T3'; the timestamp is the timestamp of the latest t2
received.
The event is written to a file "t3_iiiii.dat" where 'iiiii' is an
event number starting from 1 and incremented at each event.

Pm/Cs can be stopped cleanly with the command 'q'.

Testing123



