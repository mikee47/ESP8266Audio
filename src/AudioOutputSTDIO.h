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

#pragma once

#include <WString.h>
#include <stdbool.h>
#include <stdio.h>

#include "AudioOutput.h"

class AudioOutputSTDIO : public AudioOutput
{
  public:
    bool begin() override;
    bool ConsumeSample(int16_t sample[2]) override;
    bool stop() override;

    void SetFilename(const char *name)
    {
    	filename = name;
    }

  private:
    FILE *f = nullptr;
    String filename;
};
