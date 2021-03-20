#include "../lib/hysteresis/hysteresis.h"
#include "hysteresis_tests.h"
#include <unity.h>
#include "time.h"

Hysteresis hyst1;

void test_function_hysteresis(void) {
     bool state = hyst1.get_state();
     hyst1.set_hysteresis_time_from(false, 2000.0_msec);
	hyst1.set_hysteresis_time_from(true, 2000.0_msec);

     int time = 0;

     while (time<10){
          hyst1.update(time);
          if (time == 2) hyst1.set_state_and_update(true, time);
          if (time == 3) TEST_ASSERT_FALSE(hyst1.get_state());
          if (time == 4) TEST_ASSERT_TRUE(hyst1.get_state());
          if (time == 5) hyst1.set_state_and_update(false, time);
          if (time == 6) TEST_ASSERT_TRUE(hyst1.get_state());
          if (time == 7) TEST_ASSERT_FALSE(hyst1.get_state());
          time++;
     }
}