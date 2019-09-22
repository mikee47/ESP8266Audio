/*
  AudioGeneratorMP3
  Wrap libmad MP3 library to play audio
  
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

#include "AudioGeneratorMP3.h"
#include <Arduino.h>

void AudioGeneratorMP3::freeBuffers()
{
	if(!preallocateSpace) {
		delete[] buff;
		delete synth;
		delete frame;
		delete stream;
	}

	buff = nullptr;
	synth = nullptr;
	frame = nullptr;
	stream = nullptr;
}

bool AudioGeneratorMP3::stop()
{
	if(madInitted) {
		mad_synth_finish(synth);
		mad_frame_finish(frame);
		mad_stream_finish(stream);
		madInitted = false;
	}

	freeBuffers();

	running = false;
	output->stop();
	return file->close();
}

enum mad_flow AudioGeneratorMP3::ErrorToFlow()
{
	char err[64];
	char errLine[128];

	// Special case - eat "lost sync @ byte 0" as it always occurs and is not really correct....it never had sync!
	if((lastReadPos == 0) && (stream->error == MAD_ERROR_LOSTSYNC))
		return MAD_FLOW_CONTINUE;

	strcpy_P(err, mad_stream_errorstr(stream));
	m_snprintf(errLine, sizeof(errLine), _F("Decoding error '%s' at byte offset %d"), err,
			   (stream->this_frame - buff) + lastReadPos);
	yield(); // Something bad happened anyway, ensure WiFi gets some time, too
	cb.st(stream->error, errLine);
	return MAD_FLOW_CONTINUE;
}

enum mad_flow AudioGeneratorMP3::Input()
{
	int unused = 0;

	if(stream->next_frame) {
		unused = buffLen - (stream->next_frame - buff);
		memmove(buff, stream->next_frame, unused);
		stream->next_frame = nullptr;
	}
	if(unused == buffLen) {
		// Something wicked this way came, throw it all out and try again
		unused = 0;
	}

	lastReadPos = file->getPos() - unused;
	int len = buffLen - unused;
	len = file->read(buff + unused, len);
	if(len == 0) {
		AUDIO_INFO("Stop, len==0");
		return MAD_FLOW_STOP;
	}

	mad_stream_buffer(stream, buff, len + unused);

	return MAD_FLOW_CONTINUE;
}

bool AudioGeneratorMP3::DecodeNextFrame()
{
	if(mad_frame_decode(frame, stream) == -1) {
		ErrorToFlow(); // Always returns CONTINUE
		return false;
	}
	nsCountMax = MAD_NSBSAMPLES(&frame->header);
	return true;
}

bool AudioGeneratorMP3::GetOneSample(int16_t sample[2])
{
	if(synth->pcm.samplerate != lastRate) {
		output->SetRate(synth->pcm.samplerate);
		lastRate = synth->pcm.samplerate;
	}
	if(synth->pcm.channels != lastChannels) {
		output->SetChannels(synth->pcm.channels);
		lastChannels = synth->pcm.channels;
	}

	// If we're here, we have one decoded frame and sent 0 or more samples out
	if(samplePtr < synth->pcm.length) {
		sample[AudioOutput::LEFTCHANNEL] = synth->pcm.samples[0][samplePtr];
		sample[AudioOutput::RIGHTCHANNEL] = synth->pcm.samples[1][samplePtr];
		samplePtr++;
	} else {
		samplePtr = 0;

		switch(mad_synth_frame_onens(synth, frame, nsCount++)) {
		case MAD_FLOW_STOP:
		case MAD_FLOW_BREAK:
			AUDIO_ERROR("msf1ns failed");
			return false; // Either way we're done
		default:
			break; // Do nothing
		}
		// for IGNORE and CONTINUE, just play what we have now
		sample[AudioOutput::LEFTCHANNEL] = synth->pcm.samples[0][samplePtr];
		sample[AudioOutput::RIGHTCHANNEL] = synth->pcm.samples[1][samplePtr];
		samplePtr++;
	}
	return true;
}

#include <Platform/Timers.h>

bool AudioGeneratorMP3::loop()
{
	OneShotFastMs timer(10);

	// First, try and push in the stored sample.  If we can't, then punt and try later
	while(running && output->ConsumeSample(lastSample) && !timer.expired()) {
		// Try and stuff the buffer one sample at a time
		// Decode next frame if we're beyond the existing generated data
		if((samplePtr >= synth->pcm.length) && (nsCount >= nsCountMax)) {
		retry:
			if(Input() == MAD_FLOW_STOP) {
				return false;
			}

			if(!DecodeNextFrame()) {
				goto retry;
			}
			samplePtr = 9999;
			nsCount = 0;
		}

		if(!GetOneSample(lastSample)) {
			AUDIO_ERROR("G1S failed");
			running = false;
			break;
		}
	}

	file->loop();
	output->loop();

	return running;
}

bool AudioGeneratorMP3::begin(AudioFileSource* source, AudioOutput* output)
{
	if(source == nullptr || output == nullptr) {
		return false;
	}
	file = source;
	this->output = output;
	if(!file->isOpen()) {
		AUDIO_ERROR("MP3 source file not open");
		return false; // Error
	}

	output->SetBitsPerSample(16); // Constant for MP3 decoder
	output->SetChannels(2);

	if(!output->begin()) {
		return false;
	}

	// Where we are in generating one frame's data, set to invalid so we will run loop on first getsample()
	samplePtr = 9999;
	nsCount = 9999;
	lastRate = 0;
	lastChannels = 0;
	lastReadPos = 0;

	// Allocate all large memory chunks
	if(preallocateSpace) {
		uint8_t* p = static_cast<uint8_t*>(preallocateSpace);
		buff = p;
		p += (buffLen + 7) & ~7;
		stream = reinterpret_cast<struct mad_stream*>(p);
		p += (sizeof(struct mad_stream) + 7) & ~7;
		frame = reinterpret_cast<struct mad_frame*>(p);
		p += (sizeof(struct mad_frame) + 7) & ~7;
		synth = reinterpret_cast<struct mad_synth*>(p);
		p += (sizeof(struct mad_synth) + 7) & ~7;
		int neededBytes = p - reinterpret_cast<uint8_t*>(preallocateSpace);
		if(neededBytes > preallocateSize) {
			AUDIO_ERROR("OOM: Want %u bytes, have %u bytes preallocated", neededBytes, preallocateSize);
			return false;
		}
	} else {
		buff = new uint8_t[buffLen];
		stream = new mad_stream;
		frame = new mad_frame;
		synth = new mad_synth;
		if(!buff || !stream || !frame || !synth) {
			freeBuffers();
			return false;
		}
	}

	mad_stream_init(stream);
	mad_frame_init(frame);
	mad_synth_init(synth);
	synth->pcm.length = 0;
	mad_stream_options(stream, 0); // TODO - add options support
	madInitted = true;

	running = true;
	return true;
}
