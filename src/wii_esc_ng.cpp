/**
 * Wii-ESC NG 1.0 - 2012
 * Main program file.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <pt.h>
#include <pt-sem.h>
#include "global.h"
#include "config.h"
#include "core.h"
#include "debug.h"
#include "RX.h"
#include "power_stage.h"
#include "sdm.h"
#include "zc.h"
#include "start.h"
#include "run.h"

void setup() {
  cli();
  Board_Init();
  PowerStage_Init();
  RX_Init();
  Debug_Init();
  sei();
  pwr_stage.braking_enabled = 0;
  __delay_ms(250);
}

void beep(uint8_t khz, uint8_t len) {
 uint16_t off = 10000 / khz;
 uint16_t cnt = khz * len;
 for (uint16_t i = 0; i < cnt; i++) {
   set_pwm_on(pwr_stage.com_state);
   __delay_us(6);
   set_pwm_off(pwr_stage.com_state);
   __delay_us(off);
 }
}

void startup_sound() {
  pwr_stage.com_state = 0; set_comm_state();
  for (uint8_t i = 0; i < 5; i++) {
    pwr_stage.com_state = i;
    change_comm_state(i);
    beep(6 + i, 10);
    __delay_ms(5);
  }
}

void wait_for(uint16_t low, uint16_t high, uint8_t cnt) {
  uint8_t _cnt = cnt;
  while (_cnt) {
    uint16_t tmp = rx_get_frame();
    if ((tmp >= low) && (tmp <= high)) _cnt--; else _cnt = cnt;
  }
}

void wait_for_arm() {
  wait_for(US_TO_TICKS(RCP_MIN), US_TO_TICKS(RCP_START), 50);
}

void wait_for_power_on() {
  wait_for(US_TO_TICKS(RCP_START + RCP_DEADBAND), US_TO_TICKS(RCP_MAX), 15);
}

static void calibrate_osc() {
#if defined(RCP_CAL) && defined(INT_OSC)
  while ((rx_get_frame() > US_TO_TICKS(RCP_CAL)) && (OSCCAL > 0x00)) {
    OSCCAL--;
    rx_get_frame();
  }
  while ((rx_get_frame() < US_TO_TICKS(RCP_CAL)) && (OSCCAL < 0xFF)) {
    OSCCAL++;
    rx_get_frame();
  }
#endif
}

void loop() {
  startup_sound();
  wait_for_arm();
  calibrate_osc();
  beep(12, 50);
  for (;;) {
    free_spin(); sdm_reset();
    if (pwr_stage.braking_enabled) brake();
    wait_for_power_on();
    free_spin();
    if (start() == START_RES_OK) {
      if (run() != RUN_RES_OK) break;
    }
  };
}
