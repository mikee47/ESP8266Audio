/*
  AudioOutputMixer
  Simple mixer which can combine multiple inputs to a single output stream
  
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

#include "AudioOutput.h"

class AudioOutputMixerStub;

// Single mixer object per output
class AudioOutputMixer : public AudioOutput
{
public:
	AudioOutputMixer(int samples, AudioOutput* sink);
	~AudioOutputMixer();

	// Most "standard" interfaces should fail, only MixerStub should be able to talk to us
	bool SetRate(int hz) override
	{
		(void)hz;
		return false;
	}

	bool SetBitsPerSample(int bits) override
	{
		(void)bits;
		return false;
	}

	bool SetChannels(int channels) override
	{
		(void)channels;
		return false;
	}

	bool ConsumeSample(int16_t sample[2]) override
	{
		(void)sample;
		return false;
	}

	// Send all existing samples we can to output
	bool loop() override;

	AudioOutput* NewInput(); // Get a new stub to pass to a generator

	// Stub called functions
	friend class AudioOutputMixerStub;

private:
	void RemoveInput(int id);

	bool SetRate(int hz, int id);
	bool SetBitsPerSample(int bits, int id);
	bool SetChannels(int channels, int id);
	bool begin(int id);
	bool ConsumeSample(int16_t sample[2], int id);
	bool stop(int id);

protected:
	static constexpr unsigned maxStubs = 8;
	AudioOutput* sink = nullptr;
	bool sinkStarted = false;
	int16_t buffSize = 0;
	int32_t* leftAccum = nullptr;
	int32_t* rightAccum = nullptr;
	bool stubAllocated[maxStubs];
	bool stubRunning[maxStubs];
	int16_t writePtr[maxStubs]; // Array of pointers for allocated stubs
	int16_t readPtr;
};
