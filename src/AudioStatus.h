/*
  AudioStatus
  Base class for Audio* status/metadata reporting
  
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

#include "AudioLogger.h"

class AudioStatus
{
public:
	AudioStatus()
	{
		ClearCBs();
	}

	virtual ~AudioStatus()
	{
	}

	void ClearCBs()
	{
		metadataCallback = nullptr;
		statusCallback = nullptr;
	}

	typedef void (*MetadataCallback)(void* cbData, const char* type, bool isUnicode, const char* str);

	bool RegisterMetadataCB(MetadataCallback f, void* cbData)
	{
		metadataCallback = f;
		metadataCallbackData = cbData;
		return true;
	}

	// Returns a unique warning/error code, varying by the object.  The string may be a PSTR, use _P functions!
	typedef void (*StatusCallback)(void* cbData, int code, const char* string);

	bool RegisterStatusCB(StatusCallback f, void* cbData)
	{
		statusCallback = f;
		statusCallbackData = cbData;
		return true;
	}

	// Safely call the md function, if defined
	void md(const char* type, bool isUnicode, const char* string)
	{
		if(metadataCallback)
			metadataCallback(metadataCallbackData, type, isUnicode, string);
	}

	// Safely call the st function, if defined
	void st(int code, const char* string)
	{
		if(statusCallback)
			statusCallback(statusCallbackData, code, string);
	}

private:
	MetadataCallback metadataCallback;
	void* metadataCallbackData;
	StatusCallback statusCallback;
	void* statusCallbackData;
};
