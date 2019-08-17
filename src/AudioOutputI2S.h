/*
  AudioOutputI2S
  Base class for an I2S output port
  
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

class AudioOutputI2S : public AudioOutput
{
public:
	AudioOutputI2S(int port = 0, int output_mode = EXTERNAL_I2S, int dma_buf_count = 8, int use_apll = APLL_DISABLE);
	~AudioOutputI2S();

	bool SetRate(int hz) override;
	bool SetBitsPerSample(int bits) override;
	bool SetChannels(int channels) override;
	bool begin() override;
	bool ConsumeSample(int16_t sample[2]) override;
	bool stop() override;

	bool SetPinout(int bclkPin, int wclkPin, int doutPin);

	// Force mono output no matter the input
	bool SetOutputModeMono(bool mono)
	{
		this->mono = mono;
		return true;
	}

	enum : int {
		APLL_AUTO = -1,
		APLL_ENABLE = 1,
		APLL_DISABLE = 0,
	};

	enum : int { EXTERNAL_I2S = 0, INTERNAL_DAC, INTERNAL_PDM, OUTPUT_MODE_MAX };

protected:
	virtual unsigned AdjustI2SRate(unsigned hz)
	{
		return hz;
	}

private:
	bool install();

	// Initialised in constructor
	uint8_t portNo;
	uint8_t output_mode;
	uint8_t dma_buf_count;
	int8_t use_apll;

	//
	bool mono = false;
	bool i2sOn = false;
};
