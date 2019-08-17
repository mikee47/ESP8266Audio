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

#include "AudioFileSourceID3Unsync.h"

uint32_t AudioFileSourceID3Unsync::read(void* data, uint32_t len)
{
	uint32_t bytes = 0;
	uint8_t* ptr = reinterpret_cast<uint8_t*>(data);

	// This is only used during ID3 parsing, so no need to optimize here...
	while(len--) {
		int b = getByte();
		if(b >= 0) {
			*ptr++ = uint8_t(b);
			++bytes;
		}
	}
	return bytes;
}

int AudioFileSourceID3Unsync::getByte()
{
	// If we're not unsync, just read.
	if(!unsync) {
		uint8_t c;
		if(remaining == 0) {
			return -1;
		}
		--remaining;
		return src->read(&c, 1) == 1 ? c : -1;
	}

	// If we've saved a pre-read character, return it immediately
	if(savedByte >= 0) {
		int s = savedByte;
		savedByte = -1;
		return s;
	}

	if(remaining <= 0) {
		return -1;
	}

	if(remaining == 1) {
		remaining--;
		uint8_t c;
		return src->read(&c, 1) == 1 ? c : -1;
	}

	uint8_t c;
	remaining--;
	if(src->read(&c, 1) != 1) {
		return -1;
	}

	if(c != 0xff) {
		return c;
	}

	// Saw 0xff, check next byte.  If 0 then eat it, OTW return the 0xff
	uint8_t d;
	remaining--;
	if(src->read(&d, 1) != 1) {
		return c;
	}

	if(d != 0x00) {
		savedByte = d;
	}

	return c;
}
