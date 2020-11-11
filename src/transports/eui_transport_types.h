/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_TRANSPORT_TYPES_H
#define EUI_TRANSPORT_TYPES_H

/**
 * @file eui_transport_types.h
 * @brief Data structures and defines used in transport layer implementations.
 *
 * Provides fundamental types to capture inbound data and operate the parsing statemachine.
 */

#include <stdint.h>
#include "../eui_types.h"

/**
 * @brief Base overhead of a packet encoding and header
 *
 * Header is 3-bytes, with a 2-byte checksum and 2-bytes of COBS framing data.
 * Used to size the inbound buffer.
 */
#define PACKET_BASE_SIZE    ( sizeof(eui_header_t) \
                            + sizeof(uint16_t) \
                            + sizeof(uint16_t) )

/**
 * @brief Default maximum inbound payload buffer size
 *
 * Sets a default inbound payload length maximum of 120 bytes. Developer configuration allows for different sizes.
 *
 * @see eui_config.h for override define notes.
 */
#ifndef PAYLOAD_SIZE_MAX
    #define PAYLOAD_SIZE_MAX  120   //default inbound buffer size
#endif


/**
 * @brief Inbound Parsing statemachine data
 *
 * Keeps track of parser state, and provides length tracking for the buffers being manipulated by the parser.
 */
typedef struct {
    unsigned state          : 4;    ///< parseStates statemachine control flag
    unsigned id_bytes_in    : 4;    ///< Count parsed identifier string bytes
    unsigned data_bytes_in  : 10;   ///< Count parsed payload data bytes
    uint8_t  frame_offset;          ///< Tracks bytes until the next inbound COBS framing byte
} eui_parser_state_t;

/**
 * @brief Inbound Parser buffer
 *
 * Holds buffers as the parser processes the incoming bytestream. Storage of message identifier and payload.
 * eui_parser_state_t packs the parser state and parsed field lengths, along with running CRC calculations.
 */
typedef struct {
    eui_parser_state_t parser;                  ///< Parser statemachine data to track ingest progress
    eui_header_t header;                        ///< Storage for inbound header bytes
    
    uint8_t     id_in[EUI_MAX_MSGID_SIZE];      ///< Storage buffer for the inbound message identifier string

#ifndef EUI_CONF_OFFSETS_DISABLED
    uint16_t    offset_in;                      ///< Payload offset information for split packets
#endif
    
    uint16_t    crc_in;                         ///< CRC value decoded from the packet
    uint8_t     data_in[PAYLOAD_SIZE_MAX];      ///< Storage buffer of ingested payload data
} eui_packet_t;

#endif //end EUI_TRANSPORT_TYPES_H
