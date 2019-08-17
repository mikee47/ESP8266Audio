/*
  AudioFileSourceBuffer
  Double-buffered input file using system RAM
  
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

class AudioFileSourceBuffer : public AudioFileSource
{
public:
	AudioFileSourceBuffer(AudioFileSource* source, uint32_t bufferBytes);

	// Pre-allocated buffer by app
	AudioFileSourceBuffer(AudioFileSource* source, void* buffer, uint32_t bufferBytes)
		: src(source), buffer(static_cast<uint8_t*>(buffer)), buffSize(bufferBytes), deallocateBuffer(false)
	{
	}

	~AudioFileSourceBuffer()
	{
		freeBuffer();
	}

	uint32_t read(void* data, uint32_t len) override;
	bool seek(int32_t pos, int dir) override;
	bool close() override;
	bool isOpen() override;
	uint32_t getSize() override;
	uint32_t getPos() override;
	bool loop() override;

	virtual uint32_t getFillLevel();

	enum {
		STATUS_FILLING = 2,
		STATUS_UNDERFLOW,
	};

private:
	virtual void fill();
	void freeBuffer();

private:
	// Assigned in constructor
	AudioFileSource* src;
	uint8_t* buffer;
	uint32_t buffSize;
	bool deallocateBuffer;

	uint32_t writePtr = 0;
	uint32_t readPtr = 0;
	uint32_t length = 0;
	bool filled = false;
};
