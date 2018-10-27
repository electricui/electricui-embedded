#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../src/electricui.h"

uint8_t   test_byte  = 10;
uint16_t  test_2byte  = 200;

eui_message_t afl_msg_store[] = 
{
    EUI_UINT8(  "u8",  test_byte ),
    EUI_UINT16( "u16", test_2byte ),
};

void afl_putc( uint8_t data );

int main() 
{
    printf("Test Harness for American Fuzzy Lop (AFL\n");

    // eUI Setup
    eui_interface_t afl_comms; 
    afl_comms.output_func = &afl_putc;
    setup_interface( &afl_comms, 1 );
    EUI_TRACK( afl_msg_store );
    setup_identifier( "fuzzy", 5 );


    char *buffer = NULL;
    size_t buf_cap = 0;
    
    ssize_t bytes_read = getline(&buffer, &buf_cap, stdin);
    
    if(bytes_read == -1) 
    {
        return -1;
    }

    for(size_t i = 0; i <= bytes_read; i++)
    {
        parse_packet( (uint8_t)buffer[i], &afl_comms );
    }
  
    free(buffer);
    return 0;
}

void afl_putc( uint8_t data )
{
    //we don't care about output
}