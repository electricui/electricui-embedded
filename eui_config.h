// Add the ability to provide a callback pointer to each tracked variable

// #define EUI_CONF_VARIABLE_CALLBACKS


// Increase supported variable count to 65535
// Default 255 message support is probably enough for most people

//#define EUI_CONF_MANY_VARIABLES


// Disable offset message functionality

// #define EUI_CONF_OFFSETS_DISABLED


// The protocol limits ID lengths to a maximum of 15 characters
// Manually define a maximum here to reduce buffer size usage

//#define MESSAGEID_SIZE_MAX 12


// Default payload size for the inbound parsing buffer is 120 bytes.
// Manually reduce or increase the size as shown below.

//#define PAYLOAD_SIZE_MAX 512


// Disable error reporting to the UI (saves a few bytes silences link)

//#define EUI_CONF_ERROR_DISABLE


// Disable outbound message queuing

// #define EUI_CONF_COOPERATIVE_DISABLE


// Configure internal message ID strings
#define EUI_INTERNAL_SEARCH		"as"
#define EUI_INTERNAL_LIB_VER	"lv"
#define EUI_INTERNAL_BOARD_ID	"bi"
#define EUI_INTERNAL_SESSION_ID	"si"
#define EUI_INTERNAL_ERROR_ID	"er"
#define EUI_INTERNAL_HEARTBEAT	"hb"
#define EUI_DEFAULT_INTERFACE	"dl"

#define EUI_INTERNAL_AM_RO		"dmr"
#define EUI_INTERNAL_AM_RO_LIST	"dmrl"
#define EUI_INTERNAL_AM_RO_END	"dmre"
#define EUI_INTERNAL_AM_RW		"dmw"
#define EUI_INTERNAL_AM_RW_LIST	"dmwl"
#define EUI_INTERNAL_AM_RW_END	"dmwe"
#define EUI_INTERNAL_AV_RO		"dvr"
#define EUI_INTERNAL_AV_RW		"dvw"
