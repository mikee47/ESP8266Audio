/*
  AudioGeneratorMP3
  Wrap libmad MP3 library to play audio
    
  Copyright (C) 2017  Earle F. Philhower, III

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "AudioGenerator.h"
#include "libmad/config.h"
#include "libmad/mad.h"

class AudioGeneratorMP3 : public AudioGenerator
{
public:
	AudioGeneratorMP3() = default;

	AudioGeneratorMP3(void* preallocateSpace, int preallocateSize)
		: preallocateSpace(preallocateSpace), preallocateSize(preallocateSize)
	{
	}

	~AudioGeneratorMP3()
	{
		freeBuffers();
	}

	bool begin(AudioFileSource* source, AudioOutput* output) override;
	bool loop() override;
	bool stop() override;

protected:
	void* preallocateSpace = nullptr;
	int preallocateSize = 0;

	const int buffLen = 0x600; // Slightly larger than largest MP3 frame
	uint8_t* buff = nullptr;
	int lastReadPos;
	unsigned int lastRate;
	int lastChannels;

	// Decoding bits
	bool madInitted = false;
	struct mad_stream* stream = nullptr;
	struct mad_frame* frame = nullptr;
	struct mad_synth* synth = nullptr;
	int samplePtr;
	int nsCount;
	int nsCountMax = 1152 / 32;

	// The internal helpers
	enum mad_flow ErrorToFlow();
	enum mad_flow Input();
	bool DecodeNextFrame();
	bool GetOneSample(int16_t sample[2]);

private:
	void freeBuffers();
};
