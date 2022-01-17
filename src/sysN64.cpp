/* Raw - Another World Interpreter
 * Copyright (C) 2021 Christopher Bonhage
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <libdragon.h>
#include <system.h>

#include "sys.h"
#include "util.h"

// Display helpers
typedef uint16_t pixel_t; // 16-bit RGBA (5:5:5:1)
static const bitdepth_t FRAMEBUFFER_DEPTH = DEPTH_16_BPP;
static display_context_t now_drawing = 0;
extern void *__safe_buffer[];
#define __get_buffer( x ) (pixel_t *)__safe_buffer[(x)-1]

struct N64Stub : System {

	enum {
		// Display constants
		WINDOW_W = 320,
		WINDOW_H = 200,
		FRAMEBUFFER_W = 320,
		FRAMEBUFFER_H = 240,
		FRAMEBUFFER_PITCH = sizeof(pixel_t),
		FRAMEBUFFER_COUNT = 2,
		// Sound constants
		SOUND_SAMPLE_RATE = 22050,
		SOUND_BUFFER_COUNT = 4,
		// Timer constants
		TIMER_SLOTS_COUNT = 16,
		// Input constants
		KEY_RETURN = 13,
		KEY_C = 99,
		KEY_P = 112,
		KEY_RIGHT = 79,
		KEY_LEFT = 80,
		KEY_DOWN = 81,
		KEY_UP = 82,
	};

	pixel_t palette[NUM_COLORS];

	virtual ~N64Stub() {}
	virtual void init(const char *title);
	virtual void destroy();
	virtual void setPalette(const uint8_t *buf);
	virtual void updateDisplay(const uint8_t *src);
	virtual void processEvents();
	virtual void sleep(uint32_t duration);
	virtual uint32_t getTimeStamp();
	virtual void startAudio(AudioCallback callback, void *param);
	virtual void stopAudio();
	virtual uint32_t getOutputSampleRate();
	virtual void pollAudio();
	virtual int addTimer(uint32_t delay, TimerCallback callback, void *param);
	virtual void removeTimer(int timerId);
	virtual void *createMutex();
	virtual void destroyMutex(void *mutex);
	virtual void lockMutex(void *mutex);
	virtual void unlockMutex(void *mutex);

	TimerCallback timer_callback;
	void * timer_param;
	uint32_t timer_delay; 

	static inline int64_t SAMPLES_FROM_MS(uint32_t milliseconds) {
		return (((int64_t)milliseconds * SOUND_SAMPLE_RATE) / 1000);
	}

	static int MixerEventCallback(void *ctx) {
		N64Stub *stub = (N64Stub *)ctx;
		if (stub->timer_callback){
			stub->timer_delay = stub->timer_callback(
				stub->timer_delay,
				stub->timer_param
			);
		}
		return SAMPLES_FROM_MS(stub->timer_delay);
	}
};

void N64Stub::init(const char *title) {
	// Initialize libdragon subsystems
	controller_init();
	timer_init();
	debug_init_isviewer();

	// Initialize display buffers
	display_init(
		RESOLUTION_320x240,
		FRAMEBUFFER_DEPTH,
		FRAMEBUFFER_COUNT,
		GAMMA_NONE,
		ANTIALIAS_RESAMPLE_FETCH_ALWAYS
	);
	while (!(now_drawing = display_lock())) { /* Spinlock */ }

	int result;

	// Setup read-only filesystem
	result = dfs_init(DFS_DEFAULT_LOCATION);
	assertf(result == DFS_ESUCCESS, "dfs_init error=%d\n", result);

	// TODO Setup save slots
	// const eepfs_entry_t eeprom_files[] = {};
	// const size_t eeprom_file_count = sizeof(eeprom_files) / sizeof(eepfs_entry_t);
	// result = eepfs_init(eeprom_files, eeprom_file_count);
	// if (result == EEPFS_ESUCCESS) {
	// 	if (!eepfs_verify_signature()) eepfs_wipe();
	// } else {
	// 	debugf( "EEPROMFS init failed: %d\n", result );
	// }

	// Clear player input
	memset(&input, 0, sizeof(input));
	// Clear palette colors
	memset(&palette, 0, sizeof(palette));
}

void N64Stub::destroy() {
	// N64 does not actually support quitting!
}

void N64Stub::setPalette(const uint8_t *p) {
	uint8_t c1, c2, r, g, b, a = 0xFF;

	// The incoming palette is in 565 format.
	for (int i = 0; i < NUM_COLORS; ++i) {
		c1 = *p++;
		c2 = *p++;
		r = (((c1 & 0x0F) << 2) | ((c1 & 0x0F) >> 2)) << 2; // r
		g = (((c2 & 0xF0) >> 2) | ((c2 & 0xF0) >> 6)) << 2; // g
		b = (((c2 & 0x0F) >> 2) | ((c2 & 0x0F) << 2)) << 2; // b
		palette[i] = graphics_make_color(r, g, b, a);
	}
}

void N64Stub::updateDisplay(const uint8_t *src) {
	pixel_t * dst = __get_buffer(now_drawing);
	uint8_t src_byte;
	// One byte gives us two pixels, we only need to iterate w/2 times.
	const size_t half_width = WINDOW_W / 2;

	// Center the image vertically
	const size_t y_offset = (FRAMEBUFFER_H - WINDOW_H) / 4;
	dst += FRAMEBUFFER_W * FRAMEBUFFER_PITCH * y_offset;

	size_t height = WINDOW_H;
	while (height--) {
		for (size_t i = 0; i < half_width; ++i) {
			src_byte = *src++;
			*dst++ = palette[src_byte >> 4];
			*dst++ = palette[src_byte & 0x0F];
		}
	}

	display_show(now_drawing);
	while (!(now_drawing = display_lock())) { /* Spinlock */ }
}

void N64Stub::processEvents() {
	controller_scan();

	struct controller_data keys_pressed = get_keys_down();
	struct controller_data keys_released = get_keys_up();

	struct SI_condat p1_pressed = keys_pressed.c[0];
	if (p1_pressed.A) {
		input.lastChar = KEY_RETURN;
		input.button = true;
	}
	if (p1_pressed.C_up) {
		input.lastChar = KEY_C;
		input.code = true;
	}
	if (p1_pressed.up) {
		input.lastChar = KEY_UP;
		input.dirMask |= PlayerInput::DIR_UP;
	}
	if (p1_pressed.down) {
		input.lastChar = KEY_DOWN;
		input.dirMask |= PlayerInput::DIR_DOWN;
	}
	if (p1_pressed.left) {
		input.lastChar = KEY_LEFT;
		input.dirMask |= PlayerInput::DIR_LEFT;
	}
	if (p1_pressed.right) {
		input.lastChar = KEY_RIGHT;
		input.dirMask |= PlayerInput::DIR_RIGHT;
	}
	if (p1_pressed.start) {
		input.lastChar = KEY_P;
		input.pause = true;
	}

	struct SI_condat p1_released = keys_released.c[0];
	if (p1_released.A) {
		input.button = false;
	}
	if (p1_released.up) {
		input.dirMask &= ~PlayerInput::DIR_UP;
	}
	if (p1_released.down) {
		input.dirMask &= ~PlayerInput::DIR_DOWN;
	}
	if (p1_released.left) {
		input.dirMask &= ~PlayerInput::DIR_LEFT;
	}
	if (p1_released.right) {
		input.dirMask &= ~PlayerInput::DIR_RIGHT;
	}
}

void N64Stub::sleep(uint32_t duration) {
	while (duration--) {
		pollAudio();
		wait_ms(1);
	}
}

uint32_t N64Stub::getTimeStamp() {
	return timer_ticks() / (TICKS_PER_SECOND / 1000);
}

void N64Stub::startAudio(AudioCallback callback, void *param) {
	audio_init(SOUND_SAMPLE_RATE, SOUND_BUFFER_COUNT);
}

void N64Stub::stopAudio() {
	audio_close();
}

uint32_t N64Stub::getOutputSampleRate(void) {
	return SOUND_SAMPLE_RATE;
}

void N64Stub::pollAudio(void) {
	if (audio_can_write()) {
		short *buf = audio_write_begin();
		mixer_poll(buf, audio_get_buffer_length());
		audio_write_end();
	}
}

int N64Stub::addTimer(uint32_t delay, TimerCallback callback, void *param) {
	/**
	 * The N64 system implementation assumes that there will only ever be one
	 * timer added: the SfxPlayer eventsCallback, which must be synchronized
	 * with the LibDragon mixer. Instead of using a timer based on CPU ticks,
	 * register the callback as a mixer event based on the number of samples
	 * that have been mixed.
	 */
	timer_callback = callback;
	timer_param = param;
	timer_delay = delay;
	mixer_add_event(
		SAMPLES_FROM_MS(delay),
		&N64Stub::MixerEventCallback,
		this
	);
	return 0;
}

void N64Stub::removeTimer(int timerId) {
	mixer_remove_event(&N64Stub::MixerEventCallback, this);
}

void *N64Stub::createMutex() {
	// N64 implementation is single-threaded; mutex is not needed.
	return NULL;
}

void N64Stub::destroyMutex(void *mutex) {
	// N64 implementation is single-threaded; mutex is not needed.
	(void)mutex;
}

void N64Stub::lockMutex(void *mutex) {
	// N64 implementation is single-threaded; mutex is not needed.
	(void)mutex;
}

void N64Stub::unlockMutex(void *mutex) {
	// N64 implementation is single-threaded; mutex is not needed.
	(void)mutex;
}

N64Stub sysN64;
System *stub = &sysN64;
