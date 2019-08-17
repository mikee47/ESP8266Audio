/*
  AudioOutputWAV
  Outputs audio in WAV format

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

#include <WString.h>
#include <stdbool.h>
#include <stdio.h>

#include "AudioOutput.h"

class AudioOutputWAV : public AudioOutput
{
public:
	AudioOutputWAV() = default;

	AudioOutputWAV(const char* filename) : filename(filename)
	{
	}

	~AudioOutputWAV()
	{
		stop();
	}

	bool ConsumeSample(int16_t sample[2]) override;

	void SetFilename(const char* name)
	{
		filename = name;
	}

protected:
	virtual bool write(const void* src, size_t size) = 0;

	// Write header at current position
	bool writeHeader();

protected:
	String filename;
	size_t filePos = 0;
};
