#include "../lib/scheduler/scheduler.h"
#include "gtest/gtest.h"

namespace {
class SchedulerTest : public testing::Test {
  protected:
  void SetUp() override {
    schedule1 = new Scheduler("TEST", act_limit, exc_limit);
  }
  void TearDown() override {
    delete schedule1;
  }
  
  Scheduler *schedule1{};
  uint8_t exc_limit = 4;
  uint8_t act_limit = 10;
};

TEST_F(SchedulerTest, addExceptionAsIncorrectString) {
  // This test is named "addExceptionAsIncorrectString", and belongs to the "SchedulerTest"
  // test case.
  EXPECT_FALSE(schedule1->addException("E02:20-05-15,00-00-01,20-05-15,16-00-00"));
}

TEST_F(SchedulerTest, addActivityAsIncorrectString) {
  // This test is named "addActivityAsIncorrectString", and belongs to the "SchedulerTest"
  // test case.
  EXPECT_FALSE(schedule1->addActivity("A:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
}

TEST_F(SchedulerTest, addLineAsInCorrectString) {
  // This test is named "addLineAsCorrectString", and belongs to the "SchedulerTest"
  // test case.
  EXPECT_FALSE(schedule1->addLine("001:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
  EXPECT_FALSE(schedule1->addLine("E001:20-05-15,00-00-0120-05-15,16-00-00"));
}

TEST_F(SchedulerTest, addLineAsCorrectString) {
  // This test is named "addLineAsIncorrectString", and belongs to the "SchedulerTest"
  // test case.
  EXPECT_TRUE(schedule1->addLine("A001:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
  EXPECT_TRUE(schedule1->addLine("E001:20-05-15,00-00-01,20-05-15,16-00-00"));
}

TEST_F(SchedulerTest, addActivityAsStringWithLimitReached) {
  // This test is named "addActivityAsStringWithLimitReached", and belongs to the "SchedulerTest"
  // test case.
  int activity_cnt = 0;
  do {
    if (activity_cnt < schedule1->getActivitiesCountLimit()){
      EXPECT_TRUE(schedule1->addActivity("A002:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
    }
    else{
      EXPECT_FALSE(schedule1->addActivity("A002:20-05-01,20-09-30,MON,19-00-00,00-02-00"));
    }
  }while(++activity_cnt < (schedule1->getActivitiesCountLimit() + 2));
  EXPECT_EQ(schedule1->getActivitiesCountLimit(), schedule1->getActivitiesCount());
}

TEST_F(SchedulerTest, addExceptionAsStringWithLimitReached) {
  // This test is named "addExceptionAsStringWithLimitReached", and belongs to the "SchedulerTest"
  // test case.
  int exception_cnt = 0;
  do {
    if (exception_cnt < schedule1->getExceptionsCountLimit()){
      EXPECT_TRUE(schedule1->addException("E001:20-05-15,00-00-01,20-05-15,16-00-00"));
    }
    else{
      EXPECT_FALSE(schedule1->addException("E001:20-05-15,00-00-01,20-05-15,16-00-00"));
    }
  }while(++exception_cnt < (schedule1->getExceptionsCountLimit() + 2));
  EXPECT_EQ(schedule1->getExceptionsCountLimit(), schedule1->getExceptionsCount());
  }

}