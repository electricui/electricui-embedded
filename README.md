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

We use a binary protocol, and load the protocol as following (w/o whitespace):

`SOH header messageID STX payloadLen payload ETX checksum EOT`

We use the non-printable ascii control characters for start of header, start of text, end of text, end of transmission (oscilloscopes and logic analysers can catch these nicely, amongst actual relevance in this usecase).

By using these control characters, the messageID can (in theory) be indeterminate length as the STX would trigger the end of the header.

## Header Byte

The header field is a single byte which contains the following information through use of a bitfield as listed below:

| Bit Number    | Purpose                 |
| ------------- | ----------------------- |
| 0             | 1 if developer message  |
| 1             | 1 if ack requested      |
| 2             | Reserved for future use |
| 3-7           | Type of payload         |

This is defined with the custom structure in C

`
typedef struct  
{  
	unsigned internal	: 1;  
	unsigned reqACK		: 1;  
	unsigned reserved	: 1;  
	unsigned type		: 4;  
} euiHeader_t;  
`

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

Due to this implementation and 5 header bits for type are allocated, the maximum number of custom types is 20.

## messageID

This is the identifier used for the message. Typically 3 bytes are allocated for the use of this identifier.

While the protocol is binary in design, the developer will see these as a 1 to 3 char string which can be used as a human readable define.

`btn` or `sw` for example.

This is allowed to be any char/byte array with length ranging from 1 to the defined max message ID length in electricui.h. The library relies on null-terminated strings if the length is less than max.

For this reason, null-termination characters are illegal in the actual messageID.
Additionally, due to the use of the STX delimiter after the messageID, STX value usage is not recommended, but will be supported for single byte messageID's. Using STX on the 2nd to nth bytes will result in lost messageID data.

As the header provides a internal/developer bit, we don't need to worry about collisions with userspace message IDs.

## Payload Length

A single byte allocated for payload length. 

As long running messages aren't best practice anyway, restricting this to a single unsigned int allows payloads of 255 bytes in length.

Any messages longer than 255 bytes should ~~be handled through a custom type or like an array~~.

## Payload

Payload is the variable contents. Ints, Floats, strings, structures of data, whatever the developer feels like...

We assume the length of this is known as part of the length sent earlier, and the C implementation terminates based on length as allocating more memory as message data is ingested is nasty.

### Arrays of data

Arrays are handled natively without special header information.

The payload length defines the total number of bytes being sent, and type defines the size of each element if needed for checks or casting. 

As an array (or structure) is just a series of consecutive bytes in memory, plainly reading these out and copying in is fine providing the correct formatting (UI is responsible for decoding and encoding correctly).

Generally there shouldn't be any issues with this providing the total payload size (arrayElements * elementSize) doesn't exceed the max payload length.

## Checksum

The checksum is (for now) the XOR'ed contents of the entire message (from SOH to ETX). At this point, a single byte is likely sufficient for error detection, but each end should be designed in a manner which can be later increased to support n-byte checksum information.

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

___

# Usage

This section will be written once the end-user API is fully defined and written.
Follow the example code for reference.