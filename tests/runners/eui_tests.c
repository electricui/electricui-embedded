#include "unity_fixture.h"

static void RunAllTests(void)
{    
	//transport layer
    RUN_TEST_GROUP(SerialCRC16);
    RUN_TEST_GROUP(SerialDecoder);
    RUN_TEST_GROUP(SerialEncoder);

    //'session' layer
    RUN_TEST_GROUP(FindMessageObject);
    RUN_TEST_GROUP(MessageSend);
    RUN_TEST_GROUP(InternalEUICallbacks);

    //application layer
    RUN_TEST_GROUP(SessionLayer);

    RUN_TEST_GROUP(MacroValidation);

}

int main(int argc, const char * argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
