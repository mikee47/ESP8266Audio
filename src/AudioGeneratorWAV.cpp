/*
  AudioGeneratorWAV
  Audio output generator that reads 8 and 16-bit WAV files
  
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

#include "AudioGeneratorWAV.h"

#define FAILED(fmt, ...)                                                                                               \
	AUDIO_ERROR(fmt, ##__VA_ARGS__);                                                                                   \
	return false;

#define READ_FAILED(desc)                                                                                              \
	AUDIO_ERROR("Read failed (%s)", _F(desc));                                                                         \
	return false;

bool AudioGeneratorWAV::stop()
{
	if(!running) {
		return true;
	}
	running = false;
	output->stop();
	return file->close();
}

// Handle buffered reading, reload each time we run out of data
bool AudioGeneratorWAV::GetBufferedData(int bytes, void* dest)
{
	if(!running) {
		return false; // Nothing to do here
	}
	auto p = static_cast<uint8_t*>(dest);
	while(bytes--) {
		// Potentially load next batch of data...
		if(buffPtr >= buffLen) {
			buffPtr = 0;
			uint32_t toRead = std::min(availBytes, buffSize);
			buffLen = file->read(buff, toRead);
			availBytes -= buffLen;
		}
		if(buffPtr >= buffLen) {
			return false; // No data left
		}
		*p++ = buff[buffPtr++];
	}
	return true;
}

bool AudioGeneratorWAV::loop()
{
	while(running && output->ConsumeSample(lastSample)) {
		if(bitsPerSample == 8) {
			uint8_t l, r;
			if(!GetBufferedData(1, &l)) {
				stop();
			}
			if(channels == 2) {
				if(!GetBufferedData(1, &r)) {
					stop();
				}
			} else {
				r = 0;
			}
			lastSample[AudioOutput::LEFTCHANNEL] = l;
			lastSample[AudioOutput::RIGHTCHANNEL] = r;
		} else if(bitsPerSample == 16) {
			if(!GetBufferedData(2, &lastSample[AudioOutput::LEFTCHANNEL])) {
				stop();
			}
			if(channels == 2) {
				if(!GetBufferedData(2, &lastSample[AudioOutput::RIGHTCHANNEL])) {
					stop();
				}
			} else {
				lastSample[AudioOutput::RIGHTCHANNEL] = 0;
			}
		}
	}

	file->loop();
	output->loop();

	return running;
}

bool AudioGeneratorWAV::ReadWAVInfo()
{
	// WAV specification document:
	// https://www.aelius.com/njh/wavemetatools/doc/riffmci.pdf

	// Header == "RIFF"
	uint32_t u32;
	if(!ReadU32(&u32)) {
		READ_FAILED("RIFF");
	}
	if(u32 != 0x46464952) {
		FAILED("RIFF signature bad, got: %08X", u32);
	}

	// Skip ChunkSize
	if(!ReadU32(&u32)) {
		READ_FAILED("RIFF ChunkSize");
	};

	// Format == "WAVE"
	if(!ReadU32(&u32)) {
		READ_FAILED("WAVE");
	};
	if(u32 != 0x45564157) {
		FAILED("WAVE bad, got: %08X", u32);
	}

	// there might be JUNK or PAD - ignore it by continuing reading until we get to "fmt "
	while(true) {
		if(!ReadU32(&u32)) {
			READ_FAILED("Chunk ID");
		};
		if(u32 == 0x20746d66) {
			break; // 'fmt '
		}
	}

	// subchunk size
	if(!ReadU32(&u32)) {
		READ_FAILED("Subchunk size");
	};

	int toSkip;
	if(u32 == 16) {
		toSkip = 0;
	} else if(u32 == 18) {
		toSkip = 18 - 16;
	} else if(u32 == 40) {
		toSkip = 40 - 16;
	} else {
		FAILED("Subchunk size is non-standard PCM");
	} // we only do standard PCM

	// AudioFormat
	uint16_t u16;
	if(!ReadU16(&u16)) {
		READ_FAILED("AudioFormat");
	};
	if(u16 != 1) {
		FAILED("AudioFormat != 1 (non-standard PCM)");
	} // we only do standard PCM

	// NumChannels
	if(!ReadU16(&channels)) {
		READ_FAILED("NumChannels");
	};
	if((channels < 1) || (channels > 2)) {
		FAILED("Channels bad, %u read", channels);
	} // Mono or stereo support only

	// SampleRate
	if(!ReadU32(&sampleRate)) {
		READ_FAILED("SampleRate");
	};
	if(sampleRate == 0) {
		FAILED("SampleRate bad");
	} // Weird rate, punt.  Will need to check w/DAC to see if supported

	// Ignore byterate and blockalign
	if(!ReadU32(&u32)) {
		READ_FAILED("ByteRate");
	};
	if(!ReadU16(&u16)) {
		READ_FAILED("BlockAlign");
		return false;
	};

	// Bits per sample
	if(!ReadU16(&bitsPerSample)) {
		READ_FAILED("BitsPerSample");
	};
	if((bitsPerSample != 8) && (bitsPerSample != 16)) {
		FAILED("BitsPerSample bad, %u found", bitsPerSample);
	} // Only 8 or 16 bits

	// Skip any extra header
	while(toSkip != 0) {
		uint8_t ign;
		if(!ReadU8(&ign)) {
			READ_FAILED("Extra header");
		}
		--toSkip;
	}

	// look for data subchunk
	while(true) {
		// id == "data"
		if(!ReadU32(&u32)) {
			READ_FAILED("Subchunk ID");
		}
		if(u32 == 0x61746164) {
			break; // "data"
		}
		// Skip size, read until end of chunk
		if(!ReadU32(&u32)) {
			READ_FAILED("Subchunk size");
		}
		file->seek(u32, SEEK_CUR);
	}

	if(!file->isOpen()) {
		FAILED("Cannot read WAV, file not open");
	}

	// Skip size, read until end of file...
	if(!ReadU32(&u32)) {
		READ_FAILED("Data size");
	}
	availBytes = u32;

	// Now set up the buffer or fail
	buffPtr = 0;
	buffLen = 0;

	return true;
}

bool AudioGeneratorWAV::begin(AudioFileSource* source, AudioOutput* output)
{
	if(!source) {
		FAILED("Invalid source");
	}
	file = source;
	if(!output) {
		FAILED("Invalid output");
	}
	this->output = output;
	if(!file->isOpen()) {
		FAILED("File not open");
	} // Error

	if(!ReadWAVInfo()) {
		FAILED("ReadWAVInfo");
	}

	if(!output->SetRate(sampleRate)) {
		FAILED("output->SetRate()");
	}
	if(!output->SetBitsPerSample(bitsPerSample)) {
		FAILED("output->SetBitsPerSample()");
	}
	if(!output->SetChannels(channels)) {
		FAILED("output->SetChannels()");
	}
	if(!output->begin()) {
		FAILED("output->begin()");
	}

	running = true;

	return true;
}
