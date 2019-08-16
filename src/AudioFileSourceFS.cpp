/*
  AudioFileSourceFS
  Input "file" to be used by AudioGenerator
  
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

#include "AudioFileSourceFS.h"
#ifdef ESP32
#include "SPIFFS.h"
#endif

AudioFileSourceFS::AudioFileSourceFS(const char* filename)
{
	open(filename);
}

AudioFileSourceFS::~AudioFileSourceFS()
{
	close();
}

bool AudioFileSourceFS::open(const char* filename)
{
	handle = fileOpen(filename, eFO_ReadOnly);
	return handle >= 0;
}

uint32_t AudioFileSourceFS::AudioFileSourceFS::read(void* data, uint32_t len)
{
	int res = fileRead(handle, data, len);
	return (res >= 0) ? res : 0;
}

bool AudioFileSourceFS::seek(int32_t pos, int dir)
{
	return fileSeek(pos, dir) == SPIFFS_OK;
}

bool AudioFileSourceFS::close()
{
	fileClose(handle);
	handle = -1;
	return true;
}

bool AudioFileSourceFS::isOpen()
{
	return file >= 0;
}

uint32_t AudioFileSourceFS::getSize()
{
	int pos = fileTell(file);
	if(pos < 0) {
		return 0;
	}
	int size = fileSeek(file, 0, eSO_FileEnd);
	fileSeek(file, pos, eSO_FileStart);
	return (size >= 0) ? size : 0;
}

uint32_t AudioFileSourceFS::getPos()
{
	int pos = fileTell(handle);
	return (pos >= 0) ? pos : 0;
}
