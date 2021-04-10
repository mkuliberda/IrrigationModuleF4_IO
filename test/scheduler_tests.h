#include "../lib/scheduler/scheduler.h"
#include "gtest/gtest.h"

namespace {
class SchedulerTest : public testing::Test {
  protected:
  void SetUp() override {
    schedule1 = new Scheduler("TEST");
  }
  void TearDown() override {
    delete schedule1;
  }
  
  Scheduler *schedule1;
};

TEST_F(SchedulerTest, addActivityAsStringWithLimitReached) {
  // This test is named "addActivityAsStringWithLimitReached, and belongs to the "SchedulerTest"
  // test case.
  int activity_cnt = 0;
  do {
    if (activity_cnt <= 255){
      EXPECT_TRUE(schedule1->addActivity("A2:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
    }
    else{
      EXPECT_FALSE(schedule1->addActivity("A2:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
    }
  }while(++activity_cnt < 257);
}

TEST_F(SchedulerTest, addExceptionAsStringWithLimitReached) {
  // This test is named "addExceptionAsStringWithLimitReached, and belongs to the "SchedulerTest"
  // test case.
  int exception_cnt = 0;
  do {
    if (exception_cnt <= 40){
      EXPECT_TRUE(schedule1->addException("E1:20-05-15,00-00-01,20-05-15,16-00-00"));
    }
    else{
      EXPECT_FALSE(schedule1->addException("E1:20-05-15,00-00-01,20-05-15,16-00-00"));
    }
  }while(++exception_cnt < 43);
}
}