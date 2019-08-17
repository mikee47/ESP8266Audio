/*
  AudioGenerator
  Base class of an audio output generator
  
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

#include "AudioStatus.h"
#include "AudioFileSource.h"
#include "AudioOutput.h"

class AudioGenerator
{
public:
	virtual ~AudioGenerator()
	{
		stop();
	}

	virtual bool begin(AudioFileSource* source, AudioOutput* output)
	{
		(void)source;
		(void)output;
		return false;
	}

	virtual bool loop()
	{
		return false;
	}

	virtual bool stop()
	{
		return false;
	}

	virtual bool isRunning()
	{
		return running;
	}

public:
	virtual bool RegisterMetadataCB(AudioStatus::MetadataCallback fn, void* data)
	{
		return cb.RegisterMetadataCB(fn, data);
	}

	virtual bool RegisterStatusCB(AudioStatus::StatusCallback fn, void* data)
	{
		return cb.RegisterStatusCB(fn, data);
	}

protected:
	bool running = false;
	AudioFileSource* file = nullptr;
	AudioOutput* output = nullptr;
	int16_t lastSample[2] = {0};

protected:
	AudioStatus cb;
};
