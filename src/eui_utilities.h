/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_UTILITIES_H
#define EUI_UTILITIES_H

/**
 * @file eui_utilities.h
 * @brief Helper functions for CRC, range validation, etc
 *
 */

#include <stdint.h>

/**
 * @brief CRC16-CCITT-FALSE calculation
 *
 * Provides a per-byte CRC16-CCITT-FALSE implementation which mutates a 'running CRC' intermediate variable.
 * CRC is seeded with 0xFFFF by default.
 *
 * This allows a temporary CRC value to be held, and data included as it's processed, rather than in a block.
 * Implemented byte-wise as the parser operates 'statelessly' on one-byte inputs.
 *
 * @param data One byte of data to be included in the CRC
 * @param crc Pointer to a copy of the running CRC value
 * @see https://crccalc.com/ for online encoder/decoder tooling
 */
void
crc16(uint8_t data, uint16_t *crc);


/**
 * @brief Calculates valid start and end address offsets for multi-packet transfers
 *
 * Start and endpoints are sent in a metadata message so the receiver knows the overall shape of the offset structure.
 * The startpoint/endpoint of the slice are type-aware, and will ensure that any standard types aren't split across packets.
 * The returned offsets are used when sending the stream of partial-payload packets. 
 *
 * @warning As TYPE_CUSTOM layouts are out of the library's control, there are no guarantees about splitting custom structures correctly.
 *
 * @param base User-specified starting address of the desired slice
 * @param offset User-specified end address of the desired slice
 * @param type_bytes Type of data being sliced
 * @param size Total size of the payload being sliced
 * @param start Pointer to the destination of the start offset value
 * @param end Pointer to the destination of the start offset value
 */
void
validate_offset_range(  uint16_t    base, 
                        uint16_t    offset, 
                        uint8_t     type_bytes,
                        uint16_t    size,
                        uint16_t    *start,
                        uint16_t    *end );

#endif //end EUI_UTILITIES_H