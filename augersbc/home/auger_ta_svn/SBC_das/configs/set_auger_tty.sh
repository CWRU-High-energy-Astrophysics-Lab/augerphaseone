stty -F /dev/ttyUSB2 38400 rows 0 columns 0 line 0
stty -F /dev/ttyUSB2 intr "undef" quit "undef" erase "undef" kill "undef" eof "undef"
stty -F /dev/ttyUSB2 eol "undef" eol2 "undef" swtch "undef" start "undef" stop "undef"
stty -F /dev/ttyUSB2 susp "undef" rprnt "undef" werase "undef" lnext "undef"
stty -F /dev/ttyUSB2 flush "undef" min 1 time 0
stty -F /dev/ttyUSB2 -parenb -parodd cs8 -hupcl -cstopb cread clocal -crtscts -ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel -iutf8 -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke
