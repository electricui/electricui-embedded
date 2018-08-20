#include "../../electricui.h"
#include "../../eui_macro.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP( MacroValidation );

void		macro_callback(void);

void macro_callback(void)
{
	//this is a test function...
}

char      	macro_char    	= 'a';
int8_t    	macro_int8    	= -10;
uint8_t   	macro_uint8		= 10;
int16_t    	macro_int16    	= -10;
uint16_t   	macro_uint16   	= 10;
int32_t    	macro_int32    	= -10;
uint32_t	macro_uint32   	= 10;
float 		macro_float		= 10.0f;
double 		macro_double 	= 10.0f;

typedef struct {
  float x;
  float y;
  float z;
} macro_test_struct_t;

macro_test_struct_t	macro_trifloat = { 10.0f, 100.0f, 1000.0f };

euiMessage_t macro_object_array[] = {
	EUI_FUNC(	"func",		macro_callback ),
	EUI_CHAR(	"char",		macro_char ),
	EUI_INT8(	"int8",		macro_int8 ),
	EUI_INT16(	"int16",	macro_int16 ),
	EUI_INT32(	"int32",	macro_int32 ),
	EUI_UINT8(	"uint8",	macro_uint8 ),
	EUI_UINT16(	"uint16",	macro_uint16 ),
	EUI_UINT32(	"uint32",	macro_uint32 ),
	EUI_FLOAT(	"float",	macro_float ),
	EUI_DOUBLE(	"double",	macro_double ),
	EUI_CUSTOM(	"custom",	macro_trifloat ),
};

euiMessage_t macro_object_array_read_only[] = {
	EUI_RO_CHAR(	"char",		macro_char ),
	EUI_RO_INT8(	"int8",		macro_int8 ),
	EUI_RO_INT16(	"int16",	macro_int16 ),
	EUI_RO_INT32(	"int32",	macro_int32 ),
	EUI_RO_UINT8(	"uint8",	macro_uint8 ),
	EUI_RO_UINT16(	"uint16",	macro_uint16 ),
	EUI_RO_UINT32(	"uint32",	macro_uint32 ),
	EUI_RO_FLOAT(	"float",	macro_float ),
	EUI_RO_DOUBLE(	"double",	macro_double ),
	EUI_RO_CUSTOM(	"custom",	macro_trifloat ),
};

euiMessage_t expected_object_array[] = {
    { .msgID = "func",   .type = TYPE_CALLBACK|READ_ONLY_MASK,	.size = 1, 		.payload = &macro_callback 	},
    { .msgID = "char",   .type = TYPE_CHAR, 	.size = sizeof(macro_char), 	.payload = &macro_char 		},
    { .msgID = "int8",   .type = TYPE_INT8, 	.size = sizeof(macro_int8), 	.payload = &macro_int8 		},
    { .msgID = "int16",  .type = TYPE_INT16, 	.size = sizeof(macro_int16), 	.payload = &macro_int16 	},
    { .msgID = "int32",  .type = TYPE_INT32, 	.size = sizeof(macro_int32), 	.payload = &macro_int32 	},
    { .msgID = "uint8",  .type = TYPE_UINT8, 	.size = sizeof(macro_uint8), 	.payload = &macro_uint8 	},
    { .msgID = "uint16", .type = TYPE_UINT16, 	.size = sizeof(macro_uint16), 	.payload = &macro_uint16 	},
    { .msgID = "uint32", .type = TYPE_UINT32, 	.size = sizeof(macro_uint32), 	.payload = &macro_uint32 	},
    { .msgID = "float",  .type = TYPE_FLOAT, 	.size = sizeof(macro_float), 	.payload = &macro_float 	},
    { .msgID = "double", .type = TYPE_DOUBLE, 	.size = sizeof(macro_double), 	.payload = &macro_double 	},
    { .msgID = "custom", .type = TYPE_CUSTOM, 	.size = sizeof(macro_trifloat), .payload = &macro_trifloat 	},
};

euiMessage_t expected_object_array_read_only[] = {
    { .msgID = "char",   .type = TYPE_CHAR|READ_ONLY_MASK, 		.size = sizeof(macro_char), 	.payload = &macro_char 		},
    { .msgID = "int8",   .type = TYPE_INT8|READ_ONLY_MASK, 		.size = sizeof(macro_int8), 	.payload = &macro_int8 		},
    { .msgID = "int16",  .type = TYPE_INT16|READ_ONLY_MASK, 	.size = sizeof(macro_int16), 	.payload = &macro_int16 	},
    { .msgID = "int32",  .type = TYPE_INT32|READ_ONLY_MASK, 	.size = sizeof(macro_int32), 	.payload = &macro_int32 	},
    { .msgID = "uint8",  .type = TYPE_UINT8|READ_ONLY_MASK, 	.size = sizeof(macro_uint8), 	.payload = &macro_uint8 	},
    { .msgID = "uint16", .type = TYPE_UINT16|READ_ONLY_MASK, 	.size = sizeof(macro_uint16), 	.payload = &macro_uint16 	},
    { .msgID = "uint32", .type = TYPE_UINT32|READ_ONLY_MASK, 	.size = sizeof(macro_uint32), 	.payload = &macro_uint32 	},
    { .msgID = "float",  .type = TYPE_FLOAT|READ_ONLY_MASK, 	.size = sizeof(macro_float), 	.payload = &macro_float 	},
    { .msgID = "double", .type = TYPE_DOUBLE|READ_ONLY_MASK, 	.size = sizeof(macro_double), 	.payload = &macro_double 	},
    { .msgID = "custom", .type = TYPE_CUSTOM|READ_ONLY_MASK, 	.size = sizeof(macro_trifloat), .payload = &macro_trifloat 	},
};

//test arrays for the array sizing macro
uint8_t no_array[0] 		=	{   };
uint8_t small_array[1] 		=	{ 0 };
uint8_t standard_array[40]	=	{ 0 };
uint8_t large_array[350]	=	{ 0 };

TEST_SETUP( MacroValidation )
{

}

TEST_TEAR_DOWN( MacroValidation )
{

}

TEST( MacroValidation, Array_Element_Count )
{
	TEST_ASSERT_EQUAL_INT_MESSAGE( 0, 	ARR_ELEM(no_array), 		"Null array elements incorrectly counted." );
	TEST_ASSERT_EQUAL_INT_MESSAGE( 1, 	ARR_ELEM(small_array), 		"Small array elements incorrectly counted." );
	TEST_ASSERT_EQUAL_INT_MESSAGE( 40, 	ARR_ELEM(standard_array), 	"Standard array elements incorrectly counted." );
	TEST_ASSERT_EQUAL_INT_MESSAGE( 350, ARR_ELEM(large_array), 		"Large array elements incorrectly counted." );

	TEST_ASSERT_EQUAL_INT_MESSAGE( 11, ARR_ELEM(macro_object_array), 	"Macro array elements incorrectly counted." );
	TEST_ASSERT_EQUAL_INT_MESSAGE( 11, ARR_ELEM(expected_object_array), "Ground-truth array elements incorrectly counted." );
}

TEST( MacroValidation, EUIObject_Macro_Test_Standard )
{
    TEST_ASSERT_EQUAL_MEMORY_ARRAY_MESSAGE(expected_object_array, macro_object_array, sizeof(euiMessage_t), ARR_ELEM(expected_object_array), "Macro populated array != ground truth array"); 
}

TEST( MacroValidation, EUIObject_Macro_Test_Read_Only )
{
    TEST_ASSERT_EQUAL_MEMORY_ARRAY_MESSAGE(expected_object_array_read_only, macro_object_array_read_only, sizeof(euiMessage_t), ARR_ELEM(expected_object_array_read_only), "Read only macros != ground truth array"); 
}