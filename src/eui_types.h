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
    cb_generic = 0,
    cb_untracked,
    cb_todo,
};

enum eui_status_codes {
    status_ok = 0,
    status_crc_err,
    status_parser_generic,
    status_unknown_id,
    status_missing_callback,
    status_offset_er,
    status_todo,
};

#ifdef EUI_CONF_MANY_VARIABLES
    typedef uint16_t eui_variable_count_t;
#else
    typedef uint8_t eui_variable_count_t;
#endif

// Callback without data
typedef void (*eui_cb_t)(void);

// Callback with byte
typedef void (*callback_uint8_t)(uint8_t);

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
    const char*   msgID;
    uint8_t       type;
    uint16_t      size;
    void          *payload;
#ifdef EUI_CONF_VARIABLE_CALLBACKS
    eui_cb_t callback;
#endif
} eui_message_t;

#endif //end EUI_TYPES