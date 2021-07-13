#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../src/electricui.h"

uint8_t   test_byte  = 10;
uint16_t  test_2byte = 300;
uint32_t  test_4byte = 1234567890;
float     test_float = 3.1415927f;
double    test_double = 3.1415926535897931;

eui_message_t afl_msg_store[] = 
{
    EUI_UINT8(  "u8",  test_byte ),
    EUI_UINT16( "u16", test_2byte ),
    EUI_UINT32( "u32", test_4byte ),
    EUI_FLOAT(  "fp",  test_float ),
    EUI_DOUBLE( "dfp", test_double ),

};

void afl_putc( uint8_t *data, uint16_t len );

int main() 
{
    printf("Test Harness for American Fuzzy Lop (AFL\n");

    // eUI Setup
    eui_interface_t afl_comms; 
    afl_comms.output_cb = &afl_putc;
    eui_setup_interface( &afl_comms );
    EUI_TRACK( afl_msg_store );
    eui_setup_identifier( "fuzzy", 5 );


    char *buffer = NULL;
    size_t buf_cap = 0;
    
    ssize_t bytes_read = getline(&buffer, &buf_cap, stdin);
    
    if(bytes_read == -1) 
    {
        return -1;
    }

    for(size_t i = 0; i <= bytes_read; i++)
    {
        eui_parse( (uint8_t)buffer[i], &afl_comms );
    }
  
    free(buffer);
    return 0;
}

void afl_putc( uint8_t *data, uint16_t len )
{
    //we don't care about output, but print it as hex
    data[len] = 0;  //make sure the last byte is 0, then print as string
    printf("%s",(char*)data);
}
