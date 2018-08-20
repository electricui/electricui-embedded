#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER( MacroValidation )
{
    RUN_TEST_CASE( MacroValidation, Array_Element_Count )
    RUN_TEST_CASE( MacroValidation, EUIObject_Macro_Test_Standard )
    RUN_TEST_CASE( MacroValidation, EUIObject_Macro_Test_Read_Only)
}

