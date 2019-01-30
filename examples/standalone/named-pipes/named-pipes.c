#include "named-pipes.h"

#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "electricui.h"

#define MAX_BUF 1024

char * tx_pipe_name = "/tmp/eui-pipe-out";
char * rx_pipe_name = "/tmp/eui-pipe-in";

int tx_fd;
int rx_fd;

/* Set to 0 when ctrl-c caught */
static volatile uint8_t programActive = 1;
static void signal_handler(int dummy);

uint8_t last_byte = 0;

int main( void )
{
    signal(SIGINT, signal_handler);

    char rx_buf[ MAX_BUF ];

    /* Create the named pipe - FIFO */
    mkfifo(tx_pipe_name, 0666);
    mkfifo(rx_pipe_name, 0666);

    printf("Named pipes \"%s\" and \"%s\" set as endpoints...\n", tx_pipe_name, rx_pipe_name);

    /* open a read-write connection to the pipe */
    rx_fd = open(rx_pipe_name, O_RDONLY);
    tx_fd = open(tx_pipe_name, O_WRONLY);

    while( programActive )
    {
        uint8_t rx = read_from_pipe();
        write_to_pipe( rx+1 );
        last_byte = rx;
        print_gui();
        // printf("\33[2K\rConverted:%i", last_byte);


        // sleep(1);
    }

    printf("Shutting down\n");

    close( rx_fd );
    close( tx_fd );

    unlink(rx_pipe_name);

    return 0;
}

static void signal_handler(int dummy) {
    programActive = 0;
}

uint8_t read_from_pipe( void )
{
    char byte_in;
    read( rx_fd, &byte_in, 1 );

    return (uint8_t)byte_in;
}

void write_to_pipe( uint8_t *data, uint16_t len )
{
    write( tx_fd, &data, len );
}

void print_gui( void )
{
    // printf("Some stuff");

    // Print a boolean LED on/off style value
    // Print a integer
    // Print a float
    // Print a string?

}