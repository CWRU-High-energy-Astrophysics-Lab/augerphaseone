devid="1"

stty -F /dev/ttyUSB$devid 38400 cs8 -cstopb -parenb crtscts
