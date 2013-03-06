WSN-Sniffer-CLI
===============

This is a client for a transceiver used as a sniffer in IEEE 802.15.4 Wireless
Sensor Networks. This program reads frames from the transceiver with UART and
displays them live and decoded to the user. It can also write frames into a PCAP
file.

The sniffer can be adapted to various transceiver as long as they can write raw
frames to UART. Each frame needs to be prefixed by the 0xff byte and separated
in time with a short inter-frame delay to ensure that the sniffer client parses
one frame at a time. The transceiver can also send general informations which
will be printed on the terminal by prefixing text messages with the 0xfe byte.

Take note that the serial port has to be configured manually. You can do that
with either STTY, Screen or Minicom. However this may change in the future as a
default serial port configuration will be integrated into the client.

Usage examples
--------------

Display frame control information reading from ttyUSB1 at 115200 bauds.

> wsn-sniffer-cli -c -b 115200 /dev/ttyUSB1

Display all informations from the decoded frames and display a hexadecimal dump
of the payload.

> wsn-sniffer-cli -PA -b 115200 /dev/ttyUSB1

Display all informations and payload from the decoded frames and save them into
a PCAP file.

> wsn-sniffer-cli -p mac.pcap -PA -b 115200 /dev/ttyUSB1




