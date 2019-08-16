/*
  AudioOutputSTDIO
  Writes a WAV file to the STDIO filesystem
  Only for host-based testing

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

#include "AudioOutputSTDIO.h"
#include <unistd.h>

bool AudioOutputSTDIO::open(const String& filename)
{
	if(f != nullptr) {
		return false; // Already open
	}
	unlink(filename.c_str());
	f = fopen(filename.c_str(), "wb+");
	return f != nullptr;
}

void AudioOutputSTDIO::close()
{
	if(f != nullptr) {
		fclose(f);
		f = nullptr;
	}
}

bool AudioOutputSTDIO::write(const void* src, size_t size)
{
	return fwrite(src, size, 1, f) == 1;
}

bool AudioOutputSTDIO::rewind()
{
	return fseek(f, 0, SEEK_SET) == 0;
}
