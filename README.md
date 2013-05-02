WSN-Tools
=========

This project contains tools for use in IEEE 802.15.4 Wireless Sensor Networks.
These tools allow the manipulation of MAC 802.15.4 frames, setting up a sniffer, 
injecting and replaying traffic directly from the command line. These act as
clients for transceivers which are connected through UART by means of a dedicated
protocol. They can be adapted to various transceiver as long as you can write
a firmware for them which read/write frames and commands on UART using the underlying 
protocol. 

Note that code has already been made to abstract this protocol in your firmware so
that it only cares about the transceiver manipulation (i.e. sending and receiving
frames) and UART interrupts. The repository only contains the abstraction code now.
Complete example of firmwares will be added in the future. The firmware used for
development uses a MRF24J40 transceiver driven by an AVR ATmega328P microcontroller.
However it is not clean enough to be included in the repository. Other examples are 
welcome.

TODO
----

* Firmware examples with Contiki, TinyOS, ...
* Debug the still very experimental pcap-selector.
* Add an ncurses user interface for pcap-selector.
* Better documentation
* Check with other architectures (FreeBSD, Mac, Windows ?)
* Add a command to replay a PCAP file.
* Do not print informations directly to stdout with mac display functions.

Protocol
--------

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

###_Usage examples

Display frame control information reading from ttyUSB1 at 115200 bauds.

> wsn-sniffer-cli -c -b 115200 /dev/ttyUSB1

Display all informations from the decoded frames and display a hexadecimal dump
of the payload.

> wsn-sniffer-cli -PA -b 115200 /dev/ttyUSB1

Display all informations and payload from the decoded frames and save them into
a PCAP file.

> wsn-sniffer-cli -p mac.pcap -PA -b 115200 /dev/ttyUSB1

WSN-Injector-CLI
----------------

WSN-Ping-CLI
------------

PCAP-Selector
-------------

Firmware example
----------------

FIXME: This example is obsolete.
The following is a basic example of how you may use the helper functions within
the sniffer firmware.

    #include "wsn-sniffer.h"

    /* Sends data on UART. */
    static void send_on_uart(const unsigned char *data, unsigned int size);

    void start()
    {
        (...)

        /* Sends an information message to the client. */
        send_info("Hello world!\n", send_on_uart);

        /* Sends a frame to the client. */
        send_frame(frame, frame_size, send_on_uart);
    }
