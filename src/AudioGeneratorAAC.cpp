/*
  AudioGeneratorAAC
  Audio output generator using the Helix AAC decoder
  
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

#pragma GCC optimize("O3")

#include "AudioGeneratorAAC.h"

AudioGeneratorAAC::AudioGeneratorAAC()
{
	buff = new uint8_t[buffLen];
	outSample = new int16_t[1024 * 2];
	if(!buff || !outSample) {
		AUDIO_ERROR("Out of memory");
	}

	hAACDecoder = AACInitDecoder();
	if(hAACDecoder == nullptr) {
		AUDIO_ERROR("Out of memory (hAACDecoder is NULL)");
	}
}

AudioGeneratorAAC::AudioGeneratorAAC(void* preallocateData, int preallocateSz)
	: preallocateSpace(preallocateData), preallocateSize(preallocateSz)

{
	buff = static_cast<uint8_t*>(preallocateSpace);
	auto p = buff;
	p += (buffLen + 7) & ~7;
	outSample = reinterpret_cast<int16_t*>(p);
	p += (1024 * 2 * sizeof(int16_t) + 7) & ~7;
	int used = p - (uint8_t*)preallocateSpace;
	int availSpace = preallocateSize - used;
	if(availSpace < 0) {
		AUDIO_ERROR("Out of memory");
	}

	hAACDecoder = AACInitDecoderPre(p, availSpace);
	if(hAACDecoder == nullptr) {
		AUDIO_ERROR("Out of memory (hAACDecoder is NULL)");
	}
}

AudioGeneratorAAC::~AudioGeneratorAAC()
{
	if(!preallocateSpace) {
		AACFreeDecoder(hAACDecoder);
		free(buff);
		free(outSample);
	}
}

bool AudioGeneratorAAC::stop()
{
	running = false;
	output->stop();
	return file->close();
}

bool AudioGeneratorAAC::FillBufferWithValidFrame()
{
	buff[0] = 0; // Destroy any existing sync word @ 0
	int nextSync;
	do {
		nextSync = AACFindSyncWord(buff + lastFrameEnd, buffValid - lastFrameEnd);
		if(nextSync >= 0)
			nextSync += lastFrameEnd;
		lastFrameEnd = 0;
		if(nextSync == -1) {
			if(buffValid && buff[buffValid - 1] == 0xff) { // Could be 1st half of syncword, preserve it...
				buff[0] = 0xff;
				buffValid = file->read(buff + 1, buffLen - 1);
				if(buffValid == 0)
					return false; // No data available, EOF
			} else {			  // Try a whole new buffer
				buffValid = file->read(buff, buffLen - 1);
				if(buffValid == 0)
					return false; // No data available, EOF
			}
		}
	} while(nextSync == -1);

	// Move the frame to start at offset 0 in the buffer
	buffValid -= nextSync; // Throw out prior to nextSync
	memmove(buff, buff + nextSync, buffValid);

	// We have a sync word at 0 now, try and fill remainder of buffer
	buffValid += file->read(buff + buffValid, buffLen - buffValid);

	return true;
}

bool AudioGeneratorAAC::loop()
{
	if(!running)
		goto done; // Nothing to do here!

	// If we've got data, try and pump it out...
	while(validSamples) {
		lastSample[0] = outSample[curSample * 2];
		lastSample[1] = outSample[curSample * 2 + 1];
		if(!output->ConsumeSample(lastSample))
			goto done; // Can't send, but no error detected
		validSamples--;
		curSample++;
	}

	// No samples available, need to decode a new frame
	if(FillBufferWithValidFrame()) {
		// buff[0] start of frame, decode it...
		unsigned char* inBuff = reinterpret_cast<unsigned char*>(buff);
		int bytesLeft = buffValid;
		int ret = AACDecode(hAACDecoder, &inBuff, &bytesLeft, outSample);
		if(ret) {
			// Error, skip the frame...
			char buff[48];
			sprintf_P(buff, PSTR("AAC decode error %d"), ret);
			cb.st(ret, buff);
		} else {
			lastFrameEnd = buffValid - bytesLeft;
			AACFrameInfo fi;
			AACGetLastFrameInfo(hAACDecoder, &fi);
			if((int)fi.sampRateOut != (int)lastRate) {
				output->SetRate(fi.sampRateOut);
				lastRate = fi.sampRateOut;
			}
			if(fi.nChans != lastChannels) {
				output->SetChannels(fi.nChans);
				lastChannels = fi.nChans;
			}
			curSample = 0;
			validSamples = fi.outputSamps / lastChannels;
		}
	} else {
		running = false; // No more data, we're done here...
	}

done:
	file->loop();
	output->loop();

	return running;
}

bool AudioGeneratorAAC::begin(AudioFileSource* source, AudioOutput* output)
{
	if(!source)
		return false;
	file = source;
	if(!output)
		return false;
	this->output = output;
	if(!file->isOpen())
		return false; // Error

	output->begin();

	// AAC always comes out at 16 bits
	output->SetBitsPerSample(16);

	memset(buff, 0, buffLen);
	memset(outSample, 0, 1024 * 2 * sizeof(int16_t));

	running = true;

	return true;
}
