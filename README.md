WSN-Sniffer-CLI
===============

This is a client for a transceiver used as a sniffer in IEEE 802.15.4 Wireless
Sensor Networks. This program reads frames from the transceiver with UART and
displays them live and decoded to the user. It can also write frames into a PCAP
file. The sniffer can be adapted to various transceiver as long as they can
write raw frames to UART. The transceiver can also send general informations
which will be printed on the terminal.

Each frame or information string needs to be prefixed by an information byte
which contains its size. This size will always be less than 127 bytes. The
highest bit specify wether the event contains a MAC frame or an information
string. However some functions are available in the wsn-sniffer directory to
abstract the underlying protocol between the firmware and the sniffer
client. The help for these functions is in the corresponding header file.

Take note that the line is configured automatically to 8N1 when you specify the
baud rate in the command line. If you do not want to setup the line to 8N1 you
have to remove the baud rate argument and set it up manually with stty.

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

Firmware example
----------------

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
