#ifndef EUI_OFS_VAL
#define EUI_OFS_VAL_H

#include <stdint.h>

void
validate_offset_range(  uint16_t base, 
                        uint16_t offset, 
                        uint16_t type_bytes,
                        uint16_t size,
                        uint16_t *start,
                        uint16_t *end );

#endif