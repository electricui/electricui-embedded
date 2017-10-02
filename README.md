# electricui-embedded

# Implementation Notes

We want the library to be portable, stateless, and easy to use.

We use a binary protocol, and load the protocol as following

`preample header messageID payloadLen payload checksum`

The preamble is a one-byte identifier that indicates that a message should belong to eUI (and we can start parsing data after this point)

## Header Byte

The header field is a single byte which contains the following information through use of a bitfield as listed below:

| Bit Number    | Purpose                 |
| ------------- | ----------------------- |
| 0             | 1 if developer message  |
| 1             | 1 if type is custom     |
| 2             | 1 if ack requested      |
| 3             | Reserved for future use |
| 4-7           | Type of payload         |

This is defined with the custom structure in C

`
typedef struct
{
	unsigned internal	: 1;
	unsigned customType : 1;
	unsigned reqACK		: 1;
	unsigned reserved	: 1;
	unsigned type		: 4;
} euiHeader_t:
`

## messageID

This is the identifier used for the message. 3 bytes are allocated for the use of this identifier.

While the protocol is binary in design, the developer will see these as a 3-char string which can be used as a human readable define.

`btn` or `sw1` for example.

In theory, the developer could truncate this to a single int that references the index in the eUI variable structure, but we get around this by simplifying things a bit. This might change down the track depending on tooling and user feedback.

As the header provides a internal/developer bit, we will likely just incrementally start building the internal messages and build sequentially.

## Payload Length

A single byte allocated for payload length. 

As long running messages aren't best practice anyway, restricting this to a single unsigned int allows payloads of 255 bytes in length.

Any messages longer than 255 bytes should be handled through a custom type or like an array.

## Payload

Payload is the variable contents. Ints, Floats, strings, structures of data, whatever the developer feels like...

We assume the length of this is known as part of the length sent earlier, and we terminate based on length.

## Checksum

The checksum is (for now) the XOR'ed contents of the entire message (not including preamble). At this point, a single byte is likely sufficient for error detection, but each end should be designed in a manner which can be later increased to support n-byte checksum information.

# Handling Types

As the embedded side is very likely to be running a strictly typed language, the protocol needs to maintain the concept of types for arbitary payloads.

The UI doesn't need to worry about this due to devilish Javascript tricks, but the UI promises that it won't send data to us without informing us of the correct type, and it will remember the type of messages sent to it so returns are formatted correctly..

We will assume that most competent developers use the std types, ie stdtypes.h as they are probably writing C or C++ for their micro.

Types are defined as part of the header byte, where types are as follows:

| Bit Number    | Purpose                 |
| ------------- | ----------------------- |
| byte			| A single 8-bit message  |
| char			| A single character      |
| int8          | 8-bit signed integer    |
| uint8         | 8-bit unsigned integer  |
| int16         | 16-bit signed integer   |
| uint16        | 16-bit unsigned integer |
| int32         | 32-bit signed integer   |
| uint32        | 32-bit unsigned integer |
| int64         | 64-bit signed integer   |
| uint64        | 64-bit unsigned integer |
| float         | Standard float (4-byte) |
| double        | Double precision float  |
|               |                         |

