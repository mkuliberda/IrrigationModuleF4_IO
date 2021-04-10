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

TEST_F(SchedulerTest, parseActivityStringCorrect) {
  // This test is named "parseActivityStringCorrect", and belongs to the "SchedulerTest"
  // test case.
  activity_s activity = Scheduler::parseActivity("A001:20-05-01,20-09-30,MON,19-10-34,01-02-12");
  EXPECT_EQ(1, activity.id);
  EXPECT_EQ(20, activity.begin.year);
  EXPECT_EQ(5, activity.begin.month);
  EXPECT_EQ(1, activity.begin.day);
  EXPECT_EQ(20, activity.end.year);
  EXPECT_EQ(9, activity.end.month);
  EXPECT_EQ(30, activity.end.day);
  EXPECT_EQ(19, activity.start.hours);
  EXPECT_EQ(10, activity.start.minutes);
  EXPECT_EQ(34, activity.start.seconds);
  EXPECT_EQ(1, activity.duration.hours);
  EXPECT_EQ(2, activity.duration.minutes);
  EXPECT_EQ(12, activity.duration.seconds);
}

TEST_F(SchedulerTest, parseActivityStringIncorrect) {
  // This test is named "parseActivityStringIncorrect", and belongs to the "SchedulerTest"
  // test case.
  activity_s activity = Scheduler::parseActivity("A001:20-0520-09-30,MON,19-10-34,01-02-12");
  EXPECT_EQ(0xff, activity.id);
}

TEST_F(SchedulerTest, parseExceptionStringCorrect) {
  // This test is named "parseExceptionStringCorrect", and belongs to the "SchedulerTest"
  // test case.
  exception_s exception = Scheduler::parseException("E005:20-05-15,00-00-01,20-05-15,16-00-00");
  EXPECT_EQ(5, exception.id);
  EXPECT_EQ(20, exception.begin.year);
  EXPECT_EQ(5, exception.begin.month);
  EXPECT_EQ(15, exception.begin.day);
  EXPECT_EQ(0, exception.begin.hours);
  EXPECT_EQ(0, exception.begin.minutes);
  EXPECT_EQ(1, exception.begin.seconds);
  EXPECT_EQ(20, exception.end.year);
  EXPECT_EQ(5, exception.end.month);
  EXPECT_EQ(15, exception.end.day);
  EXPECT_EQ(16, exception.end.hours);
  EXPECT_EQ(0, exception.end.minutes);
  EXPECT_EQ(0, exception.end.seconds);
}

TEST_F(SchedulerTest, parseExceptionStringIncorrect) {
  // This test is named "parseExceptionStringIncorrect", and belongs to the "SchedulerTest"
  // test case.
  exception_s exception = Scheduler::parseException("A001:20-0520-09-30,MON,19-10-34,01-02-12");
  EXPECT_EQ(0xff, exception.id);
}


}