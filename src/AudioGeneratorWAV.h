/*
  AudioGeneratorWAV
  Audio output generator that reads 8 and 16-bit WAV files
    
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

class AudioGeneratorWAV : public AudioGenerator
{
public:
	bool begin(AudioFileSource* source, AudioOutput* output) override;
	bool loop() override;
	bool stop() override;
	bool isRunning() override;

private:
	bool ReadU32(uint32_t* dest)
	{
		return file->read(reinterpret_cast<uint8_t*>(dest), 4);
	}

	bool ReadU16(uint16_t* dest)
	{
		return file->read(reinterpret_cast<uint8_t*>(dest), 2);
	}

	bool ReadU8(uint8_t* dest)
	{
		return file->read(reinterpret_cast<uint8_t*>(dest), 1);
	}

	bool GetBufferedData(int bytes, void* dest);

	bool ReadWAVInfo();

protected:
	// WAV info
	uint16_t channels = 0;
	uint32_t sampleRate = 0;
	uint16_t bitsPerSample = 0;

	uint32_t availBytes = 0;

	// We need to buffer some data in-RAM to avoid doing 1000s of small reads
	static constexpr uint32_t buffSize = 128;
	uint8_t buff[buffSize];
	uint16_t buffPtr = 0;
	uint16_t buffLen = 0;
};
