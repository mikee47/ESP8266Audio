/*
  AudioFileSourceID3Unsync
  Internal class to handle ID3 unsync operation
  
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

#include "AudioFileSource.h"

class AudioFileSourceID3Unsync : public AudioFileSource
{
public:
	AudioFileSourceID3Unsync(AudioFileSource* src, int len, bool unsync) : src(src), remaining(len), unsync(unsync)
	{
	}

	uint32_t read(void* data, uint32_t len) override;

	int getByte();

	bool eof()
	{
		return remaining <= 0;
	}

private:
	AudioFileSource* src = nullptr;
	int remaining = 0;
	bool unsync = false;
	int savedByte = -1;
};
