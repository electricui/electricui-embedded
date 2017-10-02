#include "eui.h"
#include "eui_types.h"

const euiInternal_t internal[] =
{
	/* --- BUTTONS --- */
	[ _BTN             ] = { .mode = MODE_INPUT_PU, .port = PORT_D, .pin = PIN_3 },

    /* --- STATUS Indication --- */
    [ _STATUS_0        ] = { .mode = MODE_OUT_PP, .port = PORT_B, .pin = PIN_0, .initial = 0 },
    [ _STATUS_1        ] = { .mode = MODE_OUT_PP, .port = PORT_B, .pin = PIN_1, .initial = 0 },

    /* --- Aux IO --- */
    [ _AUX_GPIO_2      ] = { .mode = MODE_INPUT, .port = PORT_A, .pin = PIN_5 },
    [ _AUX_GPIO_0      ] = { .mode = MODE_INPUT, .port = PORT_D, .pin = PIN_1 },
    [ _AUX_GPIO_1      ] = { .mode = MODE_INPUT, .port = PORT_D, .pin = PIN_0 },

};


void thingo() 
{

}