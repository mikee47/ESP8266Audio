/*
  AudioOutput
  Base class of an audio output player
  
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

class AudioOutputNull : public AudioOutput
{
public:
	bool begin() override
	{
		samples = 0;
		startms = millis();
		return true;
	}

	bool ConsumeSample(int16_t sample[2]) override
	{
		(void)sample;
		samples++;
		return true;
	}

	bool stop() override
	{
		endms = millis();
		return true;
	};

	unsigned long GetMilliseconds()
	{
		return endms - startms;
	}

	int GetSamples()
	{
		return samples;
	}

	int GetFrequency()
	{
		return hertz;
	}

protected:
	unsigned long startms = 0;
	unsigned long endms = 0;
	int samples = 0;
};

#endif
