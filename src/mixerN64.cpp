/* Raw - Another World Interpreter
 * Copyright (C) 2004 Gregory Montoir
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

#include "mixer.h"
#include "serializer.h"
#include "sys.h"

static inline float volume_byte_to_float(uint8_t byte) {
	float vol = (float)byte / 255.0f;
	if (vol > 1.0f) return 1.0f;
	if (vol < 0.0f) return 0.0f;
	return vol;
}

struct MixerChannel {
	bool active;
	uint8_t volume;
	MixerChunk chunk;
	uint32_t chunkInc;
	waveform_t wave;

	static void read(void *ctx, samplebuffer_t *sbuf, int wpos, int wlen, bool seeking) {
		MixerChannel *ch = (MixerChannel*)ctx;
		uint8_t* ram_addr = (uint8_t*)samplebuffer_append(sbuf, wlen);
		memcpy(ram_addr, ch->chunk.data + wpos, wlen);
	}
};

static MixerChannel _channels[AUDIO_NUM_CHANNELS] = {0};

Mixer::Mixer(System *stub)
	: sys(stub) {
}

void Mixer::init() {
	memset(_channels, 0, sizeof(_channels));
	_mutex = sys->createMutex();
	sys->startAudio(NULL, NULL);
	mixer_init(AUDIO_NUM_CHANNELS);
}

void Mixer::free() {
	mixer_close();
	sys->stopAudio();
	sys->destroyMutex(_mutex);
}

void Mixer::playChannel(uint8_t channel, const MixerChunk *mc, uint16_t freq, uint8_t volume) {
	debug(DBG_SND, "Mixer::playChannel(%d, %d, %d)", channel, freq, volume);
	assert(channel < AUDIO_NUM_CHANNELS);

	// The mutex is acquired in the constructor
	MutexStack(sys, _mutex);
	MixerChannel *ch = &_channels[channel];
	ch->active = true;
	ch->chunk = *mc;
	ch->chunkInc = (freq << 8) / sys->getOutputSampleRate();
	ch->wave.channels = 1;
	ch->wave.bits = 8;
	ch->wave.frequency = freq;
	if (mc->loopLen) {
		ch->wave.len = mc->loopPos + mc->loopLen;
		ch->wave.loop_len = mc->loopLen;
	} else {
		ch->wave.len = mc->len;
		ch->wave.loop_len = 0;
	}
	ch->wave.read = &MixerChannel::read;
	ch->wave.ctx = ch;

	mixer_ch_play(channel, &ch->wave);

	if (ch->volume != volume) {
		ch->volume = volume;
		float vol_float = volume_byte_to_float(volume);
		mixer_ch_set_vol(channel, vol_float, vol_float);
	}
}

void Mixer::stopChannel(uint8_t channel) {
	debug(DBG_SND, "Mixer::stopChannel(%d)", channel);
	assert(channel < AUDIO_NUM_CHANNELS);
	MutexStack(sys, _mutex);
	mixer_ch_stop(channel);
	memset(&_channels[channel], 0, sizeof(MixerChannel));
}

void Mixer::setChannelVolume(uint8_t channel, uint8_t volume) {
	debug(DBG_SND, "Mixer::setChannelVolume(%d, %d)", channel, volume);
	assert(channel < AUDIO_NUM_CHANNELS);
	MutexStack(sys, _mutex);
	MixerChannel *ch = &_channels[channel];
	if (ch->volume != volume) {
		ch->volume = volume;
		float vol_float = volume_byte_to_float(volume);
		mixer_ch_set_vol(channel, vol_float, vol_float);
	}
}

void Mixer::stopAll() {
	debug(DBG_SND, "Mixer::stopAll()");
	MutexStack(sys, _mutex);
	for (uint8_t i = 0; i < AUDIO_NUM_CHANNELS; ++i) {
		mixer_ch_stop(i);
	}
	memset(&_channels, 0, sizeof(_channels));
}

void Mixer::mix(int8_t *buf, int len) {
	// Intentionally left blank
}

void Mixer::mixCallback(void *param, uint8_t *buf, int len) {
	// Intentionally left blank
}

void Mixer::saveOrLoad(Serializer &ser) {
	sys->lockMutex(_mutex);
	for (int i = 0; i < AUDIO_NUM_CHANNELS; ++i) {
		MixerChannel *ch = &_channels[i];
		uint32_t chunkPos;
		if (ser._mode == Serializer::SM_SAVE && ch->active) {
			chunkPos = mixer_ch_get_pos(i);
		}
		Serializer::Entry entries[] = {
			SE_INT(&ch->active, Serializer::SES_BOOL, VER(2)),
			SE_INT(&ch->volume, Serializer::SES_INT8, VER(2)),
			SE_INT(&chunkPos, Serializer::SES_INT32, VER(2)),
			SE_INT(&ch->chunkInc, Serializer::SES_INT32, VER(2)),
			SE_PTR(&ch->chunk.data, VER(2)),
			SE_INT(&ch->chunk.len, Serializer::SES_INT16, VER(2)),
			SE_INT(&ch->chunk.loopPos, Serializer::SES_INT16, VER(2)),
			SE_INT(&ch->chunk.loopLen, Serializer::SES_INT16, VER(2)),
			SE_END()
		};
		ser.saveOrLoadEntries(entries);
		if (ser._mode == Serializer::SM_LOAD && ch->active) {
			mixer_ch_play(i, &ch->wave);
			mixer_ch_set_pos(i, chunkPos);
			float vol_float = volume_byte_to_float(ch->volume);
			mixer_ch_set_vol(i, vol_float, vol_float);
		}
	}
	sys->unlockMutex(_mutex);
};
