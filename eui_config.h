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
#define MESSAGEID_BITS      4	//size of the messageIDlen bitfield doesn't change regardless

#ifndef MESSAGEID_SIZE_MAX
	#define MESSAGEID_SIZE      ( 1 << MESSAGEID_BITS ) //max allowed bytes in msgID
#endif

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


// Configure internal message ID strings

#define EUI_INTERNAL_LIB_VER	"lv"
#define EUI_INTERNAL_BOARD_ID	"bi"
#define EUI_INTERNAL_SESSION_ID	"si"
#define EUI_INTERNAL_ERROR_ID	"er"
#define EUI_INTERNAL_HEARTBEAT	"hb"
#define EUI_INTERNAL_AM_RO		"dmr"
#define EUI_INTERNAL_AM_RO_LIST	"dmrl"
#define EUI_INTERNAL_AM_RO_END	"dmre"
#define EUI_INTERNAL_AM_RW		"dmw"
#define EUI_INTERNAL_AM_RW_LIST	"dmwl"
#define EUI_INTERNAL_AM_RW_END	"dmwe"
#define EUI_INTERNAL_AV_RO		"dvr"
#define EUI_INTERNAL_AV_RW		"dvw"
#define EUI_INTERNAL_SEARCH		"as"
