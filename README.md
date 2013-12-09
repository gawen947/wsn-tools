WSN-Tools
=========

This project contains tools for use in IEEE 802.15.4 Wireless Sensor Networks.
These tools allow the manipulation of MAC 802.15.4 frames, setting up a sniffer, 
injecting and replaying traffic directly from the command line. These act as
clients for transceivers which are connected through UART by means of a dedicated
protocol. They can be adapted to various transceiver as long as you can write
a firmware for them which read/write frames and commands on UART using the underlying 
protocol.

News
----

Main page    : http://www.hauweele.net/~gawen/wsn-tools.html

Mailing list (subscribe)   : wsn-tools+subscribe@googlegroups.com
             (unsubscribe) : wsn-tools+unsubscribe@googlegroups.com
             (post)        : wsn-tools@googlegroups.com
             (arhives)     : http://groups.google.com/d/forum/wsn-tools

Rewriting
---------

Note that we are currently working on the new version of WSN-Tools. In particular 
this new version will include a redesign of the protocol. This new protocol will not 
be compatible with the current version and requires an update. Discussions about the 
new protocol are still open and comments are welcome on the mailing list (see below).

TODO
----

* A firmware example.
* Debug the still very experimental pcap-selector.
* Add an ncurses user interface for pcap-selector.
* Better documentation.
* Check with other architectures (FreeBSD, Mac, Windows ?).
* Add a command to replay a PCAP file.
* Do not print informations directly to stdout with mac display functions.

Protocol
--------

Note that code has already been made to abstract the protocol in your firmware so
that it only cares about the transceiver manipulation (i.e. sending and receiving
frames) and UART interrupts. The repository only contains the abstraction code now.
Complete example of firmwares will be added in the future. The firmware used for
development uses a MRF24J40 transceiver driven by an AVR ATmega328P microcontroller.
Albeit it is not clean enough to be included in the repository, an archive of the
original firmwares is available on the main page. Other examples are welcome.

The transmission unit used in the protocol is called a message.
A message consist of an information byte followed by a payload of
0 to 127 bytes. The information byte specifies the type of the
message and its size.

A message can be either a frame message or a control message.
Frame messages are used to transmit a frame to/from the transceiver.
The entire payload is used to represent the raw frame.
Control messages are used to transmit commands or responses to/from
the transceiver. That is, they permit to configure the transceiver,
transmit informational textual messages, signal success and error.

Take note that the serial line is configured automatically to 8N1 when you specify 
the baud rate in the command line. If you do not want to setup the line to 8N1 you
have to remove the baud rate argument and set it up manually with stty.

WSN-Sniffer-CLI
---------------

This is a client for a transceiver used as a sniffer in IEEE 802.15.4 Wireless
Sensor Networks. This program reads frames from the transceiver with UART and
displays them live and decoded to the user. It can also write frames into a PCAP
file.

### Usage examples

Display frame control information reading from ttyUSB1 at 115200 bauds.
Configure the transceiver to use channel 11.

> wsn-sniffer-cli -C 11 -c -b 115200 /dev/ttyUSB1

Display all informations from the decoded frames and display a hexadecimal dump
of the payload.

> wsn-sniffer-cli -PA -b 115200 /dev/ttyUSB1

Display all informations and payload from the decoded frames and save them into
a PCAP file.

> wsn-sniffer-cli -p mac.pcap -PA -b 115200 /dev/ttyUSB1

WSN-Injector-CLI
----------------

This tool allows to craft and dissect IEEE 802.15.4 MAC frames. That is selecting 
and replacing the different parts of the frame (header, payload), and changing each
element of the MAC frame individually. It may subsequently inject these frames into
a lowpan, or save the frame or its different parts into files.

### Usage examples

Display a default frame. Do not send anything (dryrun).

> wsn-injector-cli -n -D

Change the sequence number and type of the frame.

> wsn-injector-cli -n -D --seqno 3 --type ACK

Load the frame from a file, change its sequence number, source address and destination
address. Then save the header, the payload and the complete frame into different files.

> wsn-injector-cli -n -D --frame base.frm --seqno 3 --saddr --saddr ABCD-0001 --daddr ABCD-0a:1b:2c:3d:4e:5f:6a:7b --write-header new.hdr --write-payload new.pkt --write-frame new.frame

Broadcast a frame on channel 11 with a random payload.

> wsn-injector-cli -D -C 11 --random-payload --daddr ABCD-FFFF -b 115200 /dev/ttyUSB1

WSN-Ping-CLI
------------

This tool allows to ping a firmware. This can be used to check that the firmware receive
and send messages correctly on UART. It can also be used to check the UART line itself.

### Usage examples

Ping the firmware on ttyUSB1.

> wsn-ping-cli -b 115200 /dev/ttyUSB1

Flood ping with an interval of ten milleseconds between each message.

> wsn-ping-cli -f -i 10 -b 115200 /dev/ttyUSB1 

PCAP-Selector
-------------

This tool is still very experimental. It allows to open a PCAP file containing 802.15.4
MAC frames and to interactively extract replace or insert frames into this PCAP. It was
made to easily extract frames from a PCAP containing 6LoWPAN payloads one want to reinject
into the lowpan. The PCAP selector was made to support multiple user interfaces. For now
only a textual user interface is available. However another possible user interface could
use ncurses and display the decoded frame as one navigate through the PCAP. This will
hopefully be the default user interface in the future.

