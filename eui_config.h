// Add the ability to provide a callback pointer to each tracked variable
#ifdef EUI_CONF_VARIABLE_CALLBACKS

#endif

// Increase supported variable count to 65535
#ifdef EUI_CONF_MANY_VARIABLES
	typedef uint16_t eui_var_count_t;
#else
	typedef uint8_t eui_var_count_t;
#endif

// Disable offset messages
#ifdef EUI_CONF_OFFSETS_DISABLED
	#WARNING "ElectricUI will not handle data larger than PAYLOAD_SIZE_MAX"
#endif

// Configure inbound message buffer size
// TODO, and decide if worth implementing or not (minimal improvements?)


// Default payload size for the inbound parsing buffer
#ifndef PAYLOAD_SIZE_MAX
	#define PAYLOAD_SIZE_MAX	120
#endif

// Configure maximum outbound packet size
// TODO, or use symetric payload sizes

// Disable read-only messages
#define EUI_CONF_READ_ONLY_DISABLE

// Disable error reporting


// Disable outbound message queuing
#ifdef EUI_CONF_QUEUE_DISABLE
	#warning "ElectricUI may have issues with outbound buffer overruns or pre-emptive tasking"
#endif


// Configure outbound message batching


