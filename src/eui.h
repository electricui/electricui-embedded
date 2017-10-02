#ifndef EUI_H
#define EUI_H

#include "eui_types.h"

typedef enum
{
	//Board hardware
    _BTN,
    _STATUS_1,            /* Output: LED RED */
    _STATUS_0,            /* Output: LED GREEN */

	//AUX IO pins
	_AUX_GPIO_1,
	_AUX_GPIO_2,
	_AUX_GPIO_0,

    _NUMBER_OF_ENTRIES
} euiInternal_t;

#ifdef __cplusplus
}
#endif

#endif