# electricui-embedded

ElectricUI communications and handling library for use on simplistic embedded microcontrollers. Communicates with a ElectricUI compatible UI or device.

See the docs or website for more information.

## Getting Started

### Arduino

1. Ensure the Arduino IDE is installed (version >1.6.X tested ok) and the standard Blink.ino sketch compiles/flashes to the dev board.

2. Clone or download the electricui-embedded repo onto your computer. Put it whereever you like and use symbolic links, or copy the contents into "*/arduino/libraries/electricui*" folder as typically done with Arduino libraries. 

Symbolic folder links are a nice way to separate the library from the Arduino IDE install if you are planning on editing the libraries.

Unix (your Arduino sketchbook location may vary by OS/install):

```
sudo ln -s ~/projects/electricui-embedded/ ~/Arduino/libraries/electricui
```

Windows/Arduino can demonstrate issues with symbolic links across drives.

If you don't plan on developing electricui-embedded, copy or clone it straight into your libraries folder.

3. In the Arduino IDE, one can simply click *File > Examples > electricui > Examples > Hello_Electric* to load an example sketch.

4. If using the ElectricUI helloboard, set the board with *Tools > Board > "Arduino Leonardo"*

5. Test building and flashing the firmware to the board!


### Other Microcontrollers 

1. Just clone the repo and use the electricui files. Import into your project as normal (use extern C { } if using C++) and ensure the minimum setup functions are called.

2. The library assumes that you will provide a pointer to the serial tx function which accepts a char, this could be putc() or similar single byte uart_tx_write(uint8) function.

3. For for more detail, follow the docs or example Arduino code which shows the minimum setup and usage examples.

___

# Protocol Implementation Notes

We want the library to be portable, stateless where possible, and easy to use.

We use a binary protocol, and pack the protocol as follows (w/o whitespace, optional offset value):

`SOH header messageID offset(optional) payload checksum EOT`

We use the non-printable ascii control characters for start of header and end of transmission (oscilloscopes and logic analysers can catch these nicely, amongst actual relevance in this usecase).

By using these control characters, rigidly defined lengths for all other fields included in the header, parsing a packet is reasonably trivial.

## Header Byte

The header field spans 3 bytes which contain the following information through use of a bitfield as listed below:

| Bit Number    | Purpose                    |
| ------------- | -------------------------- |
| 0             | 1 if developer message     |
| 1             | 1 if ack requested         |
| 2             | 1 if query requested       |
| 3             | 1 if offset address inc    |
| 4-7           | Payload type enum          |
| 8-18          | Length of payload data     |
| 19-23         | Length of msgID            |
| 23-24         | sequence number            |

This is defined with the custom structure in C

`
typedef struct {
  unsigned internal   : 1;
  unsigned ack        : 1;
  unsigned query      : 1;
  unsigned offset     : 1;
  unsigned type       : 4;
  unsigned data_len   : 10;
  unsigned id_len     : 4;
  unsigned seq        : 2;
} euiHeader_t;
`

### Ack Behaviour

If an ingested message has a high ack bit, we use the 2-bit seq field to provide some level of sequence numbering.

The library will then respond with a message (at minimum, header and msgID, null payload) with the same seq number we recieved.

### Query Behaviour

If the query bit is high, we send back a message for the same msgID, with the _current_ value of that variable. This allows the UI to request variables without writing, or to confirm the recently-set value.

### Payload Length

10-bits are allocated for payload length to provide payloads up to 1kB in size. As long running messages aren't best practice, restricting this to the size of the inbound message buffer size is recommended.

Any messages longer than the limit should be handled somewhat automatically by the offset functionality for large data-structures.

### Handling Types

As the embedded side is very likely to be running a strictly typed language, the protocol needs to maintain the concept of types for arbitary payloads.

The UI doesn't need to worry about this due to devilish Javascript tricks, but the UI promises that it won't send data to us without informing us of the correct type, and it will remember the type of messages sent to it so returns are formatted correctly..

We will assume that most competent developers use the std types, ie stdtypes.h as they are probably writing C or C++ for their micro.

Types are defined as part of the header byte, where types are as follows:

| Bit Number    | Purpose                 |
| ------------- | ----------------------- |
| callback      | Function callbacks      |
| byte			| A single 8-bit message  |
| char			| A single character      |
| int8          | 8-bit signed integer    |
| uint8         | 8-bit unsigned integer  |
| int16         | 16-bit signed integer   |
| uint16        | 16-bit unsigned integer |
| int32         | 32-bit signed integer   |
| uint32        | 32-bit unsigned integer |
| float         | Standard float (4-byte) |
| double        | Double precision float  |

If the developer is using custom types, they will create another enum to define their types internally, but set their enum's first element to TYPE_CUSTOM_MARKER, which is the end of the electricUI defined type enum.

This allows the custom typed developer messages to be passed around as normal, and the library/UI will check the type reference and handle it accordingly.

## messageID

This is the identifier used for the message. Currently 3 bytes are allocated for the use of this identifier, but the protocol supports up to 16.

While the protocol is binary in design, the developer will see these as a 1 to 16 char string which can be used as a human readable define.

`btn` or `switch_left` for example.

This is allowed to be any char/byte array with length ranging from 1 to the defined max message ID length in electricui.h. While the protocol has no reliance on null-terminated strings for the messageID and allows any byte value for these fields, the library will terminate on null characters for ascii based messageID's.

As the header provides a internal/developer bit, we don't need to worry about collisions with userspace message IDs.

## Address Offsets

For really large data types or arrays, sending the entire variable in one message isn't feasible due to buffer sizes and potential transfer duration.

As a result, handling larger arrays and 'very large variables' is accomplished by sharing an offset value representing the position in memory of the first value in the payload for a given messageID.

This, in conjunction with the payload length information, allows either end to know how much data to ingest, and if required, where in the variable's memory space that payload data should be read or written to.

Where possible, the payload data will align with the data structure's edge (mostly structs) to reduce any issues which could occur if a structure was split over two messages (mostly readability and conceptual issues).

## Payload

Payload is the variable contents. Ints, Floats, strings, structures of data, whatever the developer feels like...

We assume the length of this is known as part of the length sent earlier, and the C implementation terminates based on length as allocating more memory as message data is ingested is nasty.

### Arrays of data

Arrays are handled natively without special header information.

The payload length defines the total number of bytes being sent, and type defines the size of each element if needed for checks or casting. 

As an array (or structure) is just a series of consecutive bytes in memory, plainly reading these out and copying in is fine providing the correct formatting (UI is responsible for decoding and encoding correctly).

Generally there shouldn't be any issues with this providing the total payload size (arrayElements * elementSize) doesn't exceed the max payload length. Future offset based messaging should handle this down the track.

## Checksum

The checksum uses the CRC16 method defined at http://www.sal.wisc.edu/st5000/documents/tables/crc16.c and covers all data between (but not including) the preamble and checksum value/EOT.

The embedded library uses a running CRC approach, where each byte is computed on ingest and a copy of the CRC held across the encode or decode of a given packet.

___

# Application Layer Behaviour

The current msgID's for these internal behaviours is for temporary debug visibility.

I plan on converting to an enum of bytes eventually.

## Handshaking

The UI needs to handshake with devices in order to establish what the device is, and if there are incompatible versions of UI or microcontroller code.

To kick this process, the UI will send an internal callback message of "as".

The micro will then respond with:

- "as" - start of search response
- "lv" - library version
- "pv" - protocol version
- "bi" - board ID
- "si" - Session ID
- "ae" - end of search response

Additional information will be added to this process as multiple connection method functionality is added.

## Board and Session Identifiers

Used as part of the discovery and connection process, the board maintains unique device identifiers (UUID) which the UI can query to resolve the following situations:

1. Differentiate between multiple microcontrollers on the same computer.
2. Recognise multiple connection methods to the same eUI enabled product.
3. Identify if a board is currently being accessed by a different eUI application instance.

### Board Identifier

The board ID should typically be either a randomised number, or set from a unique value such as MAC address, chip ID or security key hardware.

This value is read-only by the UI.

This is a unsigned 8-bit integer.

### Session Identifier

The session UUID provides the ability for a microcontroller to hold the current 'session' and may be used to prevent UI's from connecting to a board currently in use by another eUI application. 

This may be set by the UI once a successful connection has been established. It is advised to set this back to 'idle' value when disconnecting from the board.

This is a unsigned 8-bit integer.

## Sharing messageID data with the host

When connecting to a micro, the UI will need to know what internal variables we are going to send it, and what their corresponding types/etc are going to be. This allows the UI to populate the UI on connection.

To accomplish this (after the UI has performed handshaking), the UI will send an internal callback message of "dm".

The micro will then respond with"

- "dms" - developer message start, uint8, payload is the number of developer variables internally tracked
- "dml" - developer message list, payload contains the msgID's with null character (\0 or 0x00) delimiter
- "dml" - more dml messages until all developer msgID's have been sent (definable max num per message)
- "dme" - developer message end, uint8, payload is the number of developer variables sent

## Heartbeats

In the current implementation, "hb" behaves as a uint8 internal variable.

The UI can set/query it, but there is no microcontroller side logic at this point.

## Dealing with "large" data structures and arrays

When people want to move lots of information with eUI the protocol needs to be able to handle multi-message payloads.

This is performed by essentially allowing direct memory manipulation of that variable, and the innate understanding of types, payload lengths and how memory is read and written to.

For a given packet, if the offset bit is high, a 16-bit (future expansion plan for 32-bit option) offset address takes the first bytes of the payload. 

This address indicates what part of the variable's memory the payload data is intended to consume. By using this offset address and the pointer to the variable as stored in the eUI object, randomised chunks of the larger data structure can be streamed in or out with a mostly stateless approach.

Initially, the variable will be streamed back-to-front, giving the UI the ability to allocate a sufficiently large buffer when processing the first message. When the offset address is 0x00, or equal to the variable's pointer, we are assuming (for now) that the transfer of the data has completed.

The UI is able to track gaps in memory and confirm data if anything was missed or corrupted during transit.

Using this approach, one is able to handle a minimum of 2^16 bytes for a given 'variable' pointer (structures or arrays etc).

___

# Usage

This section will be written once the end-user API is fully defined and written.
Follow the example code for reference.


___

# Overheads and Benchmarks

The baseline "starting consumption without eUI" is shown first, subsequent rows are adding something to the previous setup.

Uses the Arduino avr-g++ compiler with default settings (-0s, C++11)

|                       | Flash           | Global Vars       |
| --------------------- | --------------- | ----------------- |
| No eUI                | 5354            | 327               |
| eUI, 1 conn           | 7956            | 563               |
| +extra conn           | 8054      	  | 706               |
| +track the uint8      | 8084      	  | 716               |
| +track all vars       | 8296            | 908               |

Cost of adding eUI to project               = 2602 bytes of PROGMEM, 236 bytes of RAM
Cost of additional eUI connection methods   = 98 bytes of PROGMEM,   143 bytes of RAM each
Cost of tracking a variable with eUI        = 30 bytes of PROGMEM,   10 bytes of RAM.

These results will vary with quantity and type due to alignment of flash words/pages, compiler optimisation etc.