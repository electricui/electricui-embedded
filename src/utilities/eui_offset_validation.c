#include "eui_offset_validation.h"
#include "eui_config.h"

void
validate_offset_range(  uint16_t base,
                        uint16_t offset,
                        uint16_t type_bytes,
                        uint16_t size,
                        uint16_t *start,
                        uint16_t *end )
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

    if( base >= offset)
    {
        *start = offset - type_size;
    }
}