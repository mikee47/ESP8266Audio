/*
  AudioGeneratorMP3
  Audio output generator using the Helix MP3 decoder
  
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
#include "libhelix-mp3/mp3dec.h"

class AudioGeneratorMP3a : public AudioGenerator
{
public:
	AudioGeneratorMP3a();
	~AudioGeneratorMP3a();

	bool begin(AudioFileSource* source, AudioOutput* output) override;
	bool loop() override;
	bool stop() override;

protected:
	// Helix MP3 decoder
	HMP3Decoder hMP3Decoder;

	// Input buffering
	uint8_t buff[1600] = {0}; ///< File buffer to store at least a whole compressed frame
	int16_t buffValid = 0;
	int16_t lastFrameEnd = 0;

	// Read until we get a valid syncword and min(feof, 2048) butes in the buffer
	bool FillBufferWithValidFrame();

	// Output buffering
	int16_t outSample[1152 * 2] = {0}; // Interleaved L/R
	int16_t validSamples = 0;
	int16_t curSample = 0;

	// Each frame may change this if they're very strange, I guess
	unsigned lastRate = 0;
	int lastChannels = 0;
};
