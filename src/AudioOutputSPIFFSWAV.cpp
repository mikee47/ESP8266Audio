/*
  AudioOutputSPIFFSWAV
  Writes a WAV file to the SPIFFS filesystem
  
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

#include "AudioOutputSPIFFSWAV.h"

bool AudioOutputSPIFFSWAV::begin()
{
	if(file >= 0) {
		return false; // Already open
	}

	file = fileOpen(filename, eFO_CreateNewAlways);
	if(file < 0) {
		return false;
	}
	return writeHeader();
}

bool AudioOutputSPIFFSWAV::stop()
{
	if(file < 0) {
		return true;
	}

	if(fileSeek(file, 0, eSO_FileStart) == 0) {
		writeHeader();
	}
	fileClose(file);
	file = -1;
	return true;
}

bool AudioOutputSPIFFSWAV::write(const void* src, size_t size)
{
	return fileWrite(file, src, size) == int(size);
}
