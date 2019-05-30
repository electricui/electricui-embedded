#ifndef EUI_TYPES_H
#define EUI_TYPES_H

#include <stdint.h>

// Default supported message types
enum eui_type {
    TYPE_CALLBACK = 0,
    TYPE_CUSTOM,
    TYPE_OFFSET_METADATA,
    TYPE_BYTE,
    TYPE_CHAR,
    TYPE_INT8,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_FLOAT,
    TYPE_DOUBLE,
};

enum eui_callback_codes {
    EUI_CB_GENERIC = 0,
    EUI_CB_TRACKED,
    EUI_CB_UNTRACKED,
    EUI_CB_PARSE_FAIL,
    EUI_CB_LAST_ENUM,
};

// Shorthand header
typedef struct {
    unsigned parser     : 2;
    unsigned untracked  : 1;
    unsigned action     : 2;
    unsigned ack        : 1;
    unsigned query      : 2;
    unsigned reserved   : 1;
} eui_errors_t;

enum eui_parse_errors {
    EUI_PARSER_OK = 0,
    EUI_PARSER_IDLE,
    EUI_PARSER_ERROR,
};

enum eui_action_errors {
    EUI_ACTION_OK = 0,
    EUI_ACTION_CALLBACK_ERROR,
    EUI_ACTION_WRITE_ERROR,
    EUI_ACTION_TYPE_MISMATCH_ERROR,
};

enum eui_ack_errors {
    EUI_ACK_OK = 0,
    EUI_ACK_ERROR,
};

enum eui_query_errors {
    EUI_QUERY_OK = 0,
    EUI_QUERY_SEND_ERROR,
    EUI_QUERY_SEND_OFFSET_ERROR,
};

enum eui_output_errors {
    EUI_OUTPUT_OK = 0,
    EUI_OUTPUT_ERROR,
};

#ifdef EUI_CONF_MANY_VARIABLES
    #define TYPE_MANY_VARIABLES_SIZED TYPE_UINT8
    typedef uint16_t eui_variable_count_t;
#else
    #define TYPE_MANY_VARIABLES_SIZED TYPE_UINT8
    typedef uint8_t eui_variable_count_t;
#endif

// Callback without data
typedef void (*eui_cb_t)(void);

// Callback with byte
typedef void (*callback_uint8_t)(uint8_t);

typedef void (*callback_data_out_t)(uint8_t*, uint16_t);

// 24-bit header
typedef struct {
    unsigned data_len   : 10;
    unsigned type       : 4;
    unsigned internal   : 1;
    unsigned offset     : 1;
    unsigned id_len     : 4;
    unsigned response   : 1;
    unsigned acknum     : 3;
} eui_header_t;

// Shorthand header
typedef struct {
    unsigned internal   : 1;
    unsigned response   : 1;
    unsigned type       : 4;
} eui_pkt_settings_t;

// Base 'object' for a variable
typedef struct {
    const char*   id;
    uint8_t       type;
    uint16_t      size;

    union {
        void        *payload;
        eui_cb_t    callback;
    } ptr;
} eui_message_t;

#endif //end EUI_TYPES