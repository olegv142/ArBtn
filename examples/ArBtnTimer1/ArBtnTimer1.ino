#include "ArBtn.h"

/*
 * Polling button state using
 * Timer1 interrupts for polling with 1000usec period.
 */

#define BT_PIN 4

Btn g_btn(BT_PIN);

ISR(TIMER1_OVF_vect)
{
  g_btn.poll();
}

void timer_init(unsigned us_period)
{
  TCCR1A = 0;  // clear control register A 
  ICR1 = (F_CPU / 2000000) * us_period; // it will count up/down
  TCCR1B = _BV(WGM13) | _BV(CS10); // simmetric PWM, no prescaler
  TIMSK1 = _BV(TOIE1); // enable interrupt
}

void setup() {
  Serial.begin(9600);
  g_btn.begin();
  timer_init(1000);
}

void loop() {
  switch(g_btn.get_event()) {
    case bt_released:
      Serial.println("released");
      break;
    case bt_long_pressed:
      Serial.println("long pressed");
      break;
    default:
      ;
  }
  delay(500);
}
