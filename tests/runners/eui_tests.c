#include "unity_fixture.h"

static void RunAllTests(void)
{    
    RUN_TEST_GROUP(SerialCRC16);
    RUN_TEST_GROUP(SerialDecoder);
    RUN_TEST_GROUP(SerialEncoder);

    RUN_TEST_GROUP(FindMessageObject);
    RUN_TEST_GROUP(SessionLayer);

}

int main(int argc, const char * argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
