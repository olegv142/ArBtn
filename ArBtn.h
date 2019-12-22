#pragma once

/*
 * Arduino button class with denouncer and long press detection capability.
 */

#include <Arduino.h>

typedef enum {
	bt_none,
	bt_released,
	bt_long_pressed,	
} bt_event_t;

#define DEF_LONG_PRESS_TICKS 2500 // 2.5 sec with 1 msec tick

class Btn {
protected:
	uint8_t                 m_pin;
	bt_event_t              m_event;
	uint8_t                 m_evt_epoch;
	uint8_t                 m_get_epoch;
	uint32_t                m_state;
	uint16_t                m_ticks;
	uint16_t                m_lp_ticks;
	volatile uint8_t const *m_port;
	uint8_t                 m_bit;

public:
	Btn(uint8_t pin, uint16_t long_press_ticks = DEF_LONG_PRESS_TICKS)
		: m_pin(pin), m_event(bt_none), m_evt_epoch(0), m_get_epoch(0)
		, m_state(0), m_ticks(0), m_lp_ticks(long_press_ticks)
		// resolve pin to port for faster subsequent access
		, m_port(portInputRegister(digitalPinToPort(pin)))
		, m_bit(digitalPinToBitMask(pin))
		{}

	// Configure IO ports
	void begin()
	{
		pinMode(m_pin, INPUT_PULLUP);
	}

	// Poll button state. Typically called from timer ISR.
	void poll()
	{
		uint8_t v = *m_port & m_bit;
		bool was_pressed = m_state != 0;
		m_state = (m_state << 1) | (v ? 0 : 1);
		if (was_pressed) {
			if (!m_state) {
				/* Btn released */
				if (m_event != bt_long_pressed)
					++m_evt_epoch;
				/* Don't increment epoch in case long press was reported.
				 * The client is expected to handle either of them but not both.
				 */				
				m_event = bt_released;
			} else if (m_event != bt_long_pressed && ++m_ticks > m_lp_ticks) {
				m_event = bt_long_pressed;
				++m_evt_epoch;
			}
		} else if (m_state) {
			/* Just pressed */
			m_ticks = 0;
		}
	}

	// Returns true if the button is currently in pressed state.
	bool is_pressed() const {
		return m_state != 0;
	}

	// Returns last not yet reported event. The particular event will be returned only once.
	bt_event_t get_events()
	{
		if (m_get_epoch == m_evt_epoch)
			return bt_none;

		m_get_epoch = m_evt_epoch;
		return m_event;
	}
};
