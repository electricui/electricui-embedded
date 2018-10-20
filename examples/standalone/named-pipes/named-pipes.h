#include <stdint.h>

// Use named pipes for data transfer
uint8_t read_from_pipe( void );
void write_to_pipe( uint8_t byte );
void print_gui( void );