#include "hysteresis_tests.h"
#include "scheduler_tests.h"
#include <unity.h>

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_function_scheduler_parse_activity_valid);
    RUN_TEST(test_function_scheduler_parse_activity_invalid);
    RUN_TEST(test_function_hysteresis);
    UNITY_END();

    return 0;
}