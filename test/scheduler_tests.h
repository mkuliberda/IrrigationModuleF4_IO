#include "../lib/scheduler/scheduler.h"
#include "gtest/gtest.h"

namespace {
Scheduler schedule1("TEST");
TEST(SchedulerTest, addActivityTrue) {
  // This test is named "addActivityTrue, and belongs to the "SchedulerTest"
  // test case.
    ASSERT_TRUE(schedule1.addActivity("A2:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
}
}