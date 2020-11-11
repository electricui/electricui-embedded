/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_BINARY_TRANSPORT_H
#define EUI_BINARY_TRANSPORT_H

/**
 * @file eui_binary_transport.h
 * @brief Bundled implementation of a binary protocol
 *
 * Intended for use with serial connections, this included protocol implementation provides encoding and decoding
 * with checksums and COBS framing for reliability.
 */

#include <stdint.h>
#include "../eui_types.h"
#include "eui_transport_types.h"

/**
 * @brief Parser Statemachine States
 *
 * Used as human-readable state codes for the parser statemachine.
 *
 * @note States often represent individual bytes in the header/CRC, which allows per-byte header bitfield reconstruction, 
 * and allows CRC early exits when the first CRC byte isn't correct.
 */
enum parseStates {
    exp_frame_offset = 0,   ///< Starting COBS offset byte
    exp_header_b1,          ///< Header Byte 1
    exp_header_b2,          ///< Header Byte 2
    exp_header_b3,          ///< Header Byte 3
    exp_message_id,         ///< Message ID bytes - length specified in header
    exp_offset_b1,          ///< Offset Value - Byte 1
    exp_offset_b2,          ///< Offset Value - Byte 2
    exp_data,               ///< Payload bytes - length specified in header
    exp_crc_b1,             ///< Checksum Byte 1 - Performs early exit if CRC is partially incorrect
    exp_crc_b2,             ///< Checksum Byte 2 - Validates inbound packet and passes up to electricui.h
};

/**
 * @brief Generates the 24-bit header from structured header fields
 * 
 * @note Manual preparation of header arrangement is required for reliable cross-platform header results.
 *
 * @param header Pointer to fully formed header object
 * @param buffer Pointer to target buffer (min 3-bytes)
 * @return uint8_t Status encoded as number of header bytes encoded
 */
uint8_t
encode_header( eui_header_t *header, uint8_t *buffer );

/**
 * @brief Apply COBS framing to the outbound bytestream
 *
 * COBS framing is applied to the packet to provide 0x00 based frame synchronisation.
 * As most packets are under the 255 byte framing offset requirement, this implementation opts to mutate in place,
 * with the less common 'framing overhead' bytes being inserted by rippling the remaining data backwards in the array.
 *
 * @warning The input buffer must be sized to fit the entire packet, 
 * with additional room for the starting 0x00, framing byte, and up to 3 additional framing bytes
 *
 * @param buffer Pointer to data to encode
 * @param size Number of bytes to encode
 * @return uint8_t 
 */
uint8_t
encode_framing( uint8_t *buffer, uint16_t size );

/**
 * @brief Simpler packet output function
 *
 * Does the same thing as encode_packet(), but infers some header details automatically.
 * Requires less boilerplate for manual sends than the full encode_packet()
 *
 * @param output_function Function pointer to 'userspace' serial write callback
 * @param settings Pointer to the packet's settings structure
 * @param msg_id Pointer to the (null-delimited) message identifier string
 * @param payload_len Length of payload data in bytes
 * @param header payload Pointer to payload data
 * @return uint8_t Encoder/Output status codes - see eui_output_errors
 */
uint8_t
encode_packet_simple(   callback_data_out_t output_function,
                        eui_pkt_settings_t  *settings,
                        const char          *msg_id,
                        uint16_t            payload_len,
                        void*               payload);

/**
 * @brief Packet output function to send data
 *
 * Prepares a COBS encoded packet with header, msgID, optional offset, payload, and CRC.
 * When completed, the user's defined output callback function is called with pointers to the intermediate buffer, 
 * and then the bytestream can be copied/sent/mutated as needed in application-side code.
 *
 * @param out_char Function pointer to 'userspace' serial write callback
 * @param header Pointer to a fully formed header
 * @param msg_id Pointer to msgID character array. ID length is in the header.
 * @param offset Slice offset in bytes used for partial packets
 * @param payload Pointer to buffer of payload data. Payload length is in the header.
 * @return uint8_t Encoder/Output status codes
 * @see eui_output_errors for status return codes
 */
uint8_t
encode_packet(  callback_data_out_t out_char,
                eui_header_t        *header,
                const char          *msg_id,
                uint16_t            offset,
                void*               payload );

/**
 * @brief Decodes inbound bytestream to separate data from COBS framing
 *
 * @param inbound_byte The most recently received byte to ingest for parsing
 * @param p_link_in pointer to the transport link object
 * @return uint8_t Parser status codes
 * @see eui_parse_errors for status return codes
 */
uint8_t
decode_packet(uint8_t inbound_byte, eui_packet_t *p_link_in);

/**
 * @brief Processes inbound decoded bytestream into standard packet structure
 *
 * @param byte_in Incoming (de-COBS'ed) byte
 * @param p_link_in pointer to the transport link object
 * @return uint8_t Parser status codes
 * @see eui_parse_errors for status return codes
 */
uint8_t
parse_decoded_packet(uint8_t byte_in, eui_packet_t *p_link_in);

#endif //end EUI_BINARY_TRANSPORT_H