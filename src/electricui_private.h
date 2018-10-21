// Private functions
callback_uint8_t
auto_output( void );

uint8_t
handle_packet_data( eui_interface_t *valid_packet,
                    eui_header_t *header,
                    eui_message_t *msgObjPtr );

void
handle_packet_response( eui_interface_t *valid_packet, 
                        eui_header_t *header,
                        eui_message_t *msgObjPtr );

#ifdef EUI_CONF_VARIABLE_CALLBACKS
    void
    handle_packet_callback( eui_message_t *msgObjPtr );
#endif

//application layer functionality
void
announce_board( void );

void
announce_dev_msg_readonly( void );

void
announce_dev_msg_writable( void );

void
announce_dev_vars_readonly( void );

void
announce_dev_vars_writable( void );

euiVariableCount_t
send_tracked_message_id_list( uint8_t read_only );

euiVariableCount_t
send_tracked_variables( uint8_t read_only );

//interface management
eui_interface_t     *interfaceArray;
uint8_t             numInterfaces;

//dev interface
eui_message_t       *devObjectArray;
euiVariableCount_t  numDevObjects;

// eUI variables accessible to developer
uint8_t     heartbeat;
uint16_t    board_identifier;
uint8_t     session_identifier;
uint8_t     active_interface;
