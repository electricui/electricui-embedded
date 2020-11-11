/* Copyright (c) 2016-2020 Electric UI
 * MIT Licenced - see LICENCE for details.
 *
 * 
 */

#include "eui_utilities.h"
#include "eui_types.h"


void
crc16(uint8_t data, uint16_t *crc)
{
    *crc  = (uint8_t)(*crc >> 8) | (*crc << 8);
    *crc ^= data;
    *crc ^= (uint8_t)(*crc & 0xff) >> 4;
    *crc ^= (*crc << 8) << 4;
    *crc ^= ((*crc & 0xff) << 4) << 1;
}


void
validate_offset_range(  uint16_t    base,
                        uint16_t    offset,
                        uint8_t     type_bytes,
                        uint16_t    size,
                        uint16_t    *start,
                        uint16_t    *end )
{
    uint8_t type_size = 0;

    switch( type_bytes )
    {
        case TYPE_INT16:
        case TYPE_UINT16:
            type_size = 2;
        break;

        case TYPE_INT32:
        case TYPE_UINT32:
        case TYPE_FLOAT:
            type_size = 4;
        break;

        case TYPE_DOUBLE:
            type_size = 8;
        break;

        default:  
            //single byte types and customs
            type_size = 1;
        break;
    }

    //shift the offset up to align with the type size
    *start  = ((base    + (type_size-1)) / type_size) * type_size;
    *end    = ((offset  + (type_size-1)) / type_size) * type_size;

    if( offset > size || !offset)
    {
        *end = size;
    }

    if( *start >= *end)
    {
        *start = *end - type_size;
    }
}