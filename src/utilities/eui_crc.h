/* Copyright (c) 2016-2019 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#ifndef EUI_CRC_H
#define EUI_CRC_H

/**
 * @file eui_types.h
 * @brief CRC16 Helper function
 *
 */

#include <stdint.h>

/**
 * @brief CRC16-CCITT-FALSE calculation
 *
 * Provides a per-byte CRC16-CCITT-FALSE implementation which mutates a 'running CRC' intermedate variable.
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

#endif