#include "scheduler_tests.h"
#include "hysteresis_tests.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "gmock/gmock-matchers.h"
//#include "gmock/gmock-generated-matchers.h"


int main(int argc, char **argv) {
    //::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}