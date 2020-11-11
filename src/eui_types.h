/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_TYPES_H
#define EUI_TYPES_H

/**
 * @file eui_types.h
 * @brief Public facing type and enum definitions
 *
 * Public facing types and enums used when interacting with the Electric UI embedded library and compatible transports.
 */

#include <stdint.h>

/**
 * @brief Default maximum message identifier length
 *
 * Default ID are up to 16 bytes in length. Can be reduced to save parser buffer bytes in RAM constrained systems.
 * 
 * @warning Extending msgID length beyond 16B requires rework of the header structure and codecs, as only 4-bits are assigned for ID length.
 */
#define EUI_MAX_MSGID_SIZE (16u)


/**
 * @brief Default Supported Message Types
 *
 *  Used to indicate the type of underlying payload data, or intended message functionality.
 *  Follows `stdint.h` style where applicable.
 */
enum eui_type {
    TYPE_CALLBACK = 0,      ///< Electric UI callback type - payload is a pointer to a C function(void)
    TYPE_CUSTOM,            ///< Custom data type - typically structures
    TYPE_OFFSET_METADATA,   ///< Internal metadata message - informs receiver of partial payload transmission (delta updates)
    TYPE_BYTE,              ///< Byte (1B) - expected representation as bitfield
    TYPE_CHAR,              ///< Char (1B) - C style character byte (singular)
    TYPE_INT8,              ///< Int8 (1B) - Signed 8-bit integer, -128 to +127
    TYPE_UINT8,             ///< UInt8 (1B) - Unsigned 8-bit integer, 0 to +255
    TYPE_INT16,             ///< Int16 (2B) - Signed 16-bit integer, -32,768 to 32,767
    TYPE_UINT16,            ///< UInt16 (2B) - Unsigned 16-bit integer, 0 to +65,535
    TYPE_INT32,             ///< Int32 (4B) - Signed 32-bit integer, -2,147,483,648 to +2,147,483,647
    TYPE_UINT32,            ///< UInt32 (4B) - Unsigned 32-bit integer, 0 to +4,294,967,295
    TYPE_FLOAT,             ///< Float32 (4B) - IEE754 Single precision floating point
    TYPE_DOUBLE,            ///< Float64 (8B) - IEE754 Double precision floating point
};

/**
 * @brief Interface Callback codes
 *
 * Used as a flag passed to the developer-space interface callback function to indicate specific callback source.
 */
enum eui_callback_codes {
    EUI_CB_GENERIC = 0, ///< Generic callback from library - unused
    EUI_CB_TRACKED,     ///< Represents callback after a tracked message has been handled
    EUI_CB_UNTRACKED,   ///< Represents callback after an untracked message has been handled
    EUI_CB_PARSE_FAIL,  ///< Represents a failure in parsing (CRC errors, etc)
    EUI_CB_LAST_ENUM,   ///< Used to denote the number of callback flags
};

/**
 * @brief Error data type
 *
 * Contains bitfields representing the states of Electric UI internals.
 * Matching member values against relevant enums allows for state/error checks against specific operations.
 */
typedef struct {
    unsigned parser     : 2;    ///< Refer to eui_parse_errors
    unsigned untracked  : 1;    ///< Refer
    unsigned action     : 2;    ///< Refer to eui_action_errors
    unsigned ack        : 1;    ///< Refer to eui_ack_errors
    unsigned query      : 2;    ///< Refer to eui_query_errors
    unsigned reserved   : 1;    ///< Unused bit - reserved for future use.
} eui_errors_t;

enum eui_parse_errors {
    EUI_PARSER_OK = 0,
    EUI_PARSER_IDLE,
    EUI_PARSER_ERROR,
};

enum eui_action_errors {
    EUI_ACTION_OK = 0,
    EUI_ACTION_CALLBACK_ERROR,
    EUI_ACTION_WRITE_ERROR,
    EUI_ACTION_TYPE_MISMATCH_ERROR,
};

enum eui_ack_errors {
    EUI_ACK_OK = 0,
    EUI_ACK_ERROR,
};

enum eui_query_errors {
    EUI_QUERY_OK = 0,
    EUI_QUERY_SEND_ERROR,
    EUI_QUERY_SEND_OFFSET_ERROR,
};

enum eui_output_errors {
    EUI_OUTPUT_OK = 0,
    EUI_OUTPUT_ERROR,
};

#ifdef EUI_CONF_MANY_VARIABLES
    #define TYPE_MANY_VARIABLES_SIZED TYPE_UINT8
    typedef uint16_t eui_variable_count_t;
#else
    #define TYPE_MANY_VARIABLES_SIZED TYPE_UINT8
    typedef uint8_t eui_variable_count_t;
#endif

/**
 * @brief Function pointer used as user-space callback type
 *
 * C-style callbacks from the UI are made against this type when used as a tracked variable
 * @see eui_type TYPE_CALLBACK
 */
typedef void (*eui_cb_t)(void);

/**
 * @brief Function callback which accepts a single uint8_t argument
 *
 * Used to provide user-space interface callbacks with a flag describing where the callback was generated
 * @see eui_callback_codes
 */
typedef void (*callback_uint8_t)(uint8_t);

/**
 * @brief Function callback to pass output data to user-space to be written over serial links etc
 *
 * This powers the output data callback which Electric UI uses to send data.
 * The developer should provide their callback function pointer matching this signature and is provided with the buffer and length.
 * It is expected that the buffer of bytes will then be written to the relevant output communications interface.
 * @param 1 Pointer to a uint8_t array
 * @param 2 Length (uint16_t) of the data array in bytes
 */
typedef void (*callback_data_out_t)(uint8_t*, uint16_t);

/**
 * @brief Packet Header Data structure.
 *
 * Contains data used during parsing of packets including sizes, type and intent of the packet
 */
typedef struct {
    unsigned data_len   : 10;   ///< Length of the payload in bytes. Max 1024 byte payloads supported.
    unsigned type       : 4;    ///< Type of the payload - see the eui_type enum.
    unsigned internal   : 1;    ///< True when message is for the internal namespace (heartbeats etc).
    unsigned offset     : 1;    ///< True when packet has offset indicator bytes included.
    unsigned id_len     : 4;    ///< Length of the message identifier in bytes. Max 15 byte id supported.
    unsigned response   : 1;    ///< True when the packet requires a response
    unsigned acknum     : 3;    ///< Value used to distinguish otherwise identical packets when returning an ack
} eui_header_t;

/**
 * @brief Shorthand header data structure.
 *
 * Used internally to pass _useful_ packet settings to the transport layer, transport layer is expected to generate
 * a full eui_header_t using information from this data + inferred data from id and payload lengths.
 */
typedef struct {
    unsigned internal   : 1;    ///< True when message is for the internal namespace
    unsigned response   : 1;    ///< True when the packet requires a response
    unsigned type       : 4;    ///< Type of the payload - see the eui_type enum
} eui_pkt_settings_t;

/**
 * @brief Foundational structure for data interacting with Electric UI.
 *
 * Contains the identifier and metadata which allows for safe handling of pointed payload data.
 */
typedef struct {
    const char*   id;           ///< Identifier, normally 1-16 byte human readable ASCII such as "motor_config".
    uint8_t       type;         ///< Payload type indication as a eui_type enum value
    uint16_t      size;         ///< Size of the payload in bytes

    union {
        void        *data;      ///< Pointer to user-owned data which is viewed/modified by Electric UI.
        eui_cb_t    callback;   ///< Function pointer which will be called when the message is received.
    } ptr;                      ///< Union provides the pointer to either data, or a callback in a typesafe manner.
} eui_message_t;

#endif //end EUI_TYPES_H