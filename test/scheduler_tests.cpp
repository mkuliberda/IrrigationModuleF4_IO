#include "../lib/scheduler/scheduler.h"
#include "scheduler_tests.h"
#include <unity.h>

Scheduler schedule1("TEST");

void test_function_scheduler_parse_activity_valid(void) {
     TEST_ASSERT_TRUE(schedule1.addActivity("A2:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
}
void test_function_scheduler_parse_activity_invalid(void) {
    TEST_ASSERT_FALSE(schedule1.addActivity("A:20-05-01,20-09-30,MON19-00-00,00-02-00"));
}
