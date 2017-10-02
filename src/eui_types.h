typedef struct
{
	unsigned internal	: 1;
	unsigned customType : 1;
	unsigned reqACK		: 1;
	unsigned reserved	: 1;
	unsigned type		: 4;
} euiHeader_t:

typedef enum
{
    TYPE_BYTE = 0,
    TYPE_CHAR,
    TYPE_INT8,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_INT64,
    TYPE_UINT64,
    TYPE_FLOAT,
    TYPE_DOUBLE
    TYPE_CALLBACK,
    TYPE_ARRAY
} euiType_t;

typedef struct
{
    euiHeader_t 	header;
    char 			msgID[3];
    unsigned int	payloadLen;
    void			payload;
    unsigned int 	checksum;
} euiMessage_t;