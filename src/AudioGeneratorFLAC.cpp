/*
  AudioGeneratorFLAC
  Audio output generator that plays FLAC audio files
    
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

#include <AudioGeneratorFLAC.h>

AudioGeneratorFLAC::~AudioGeneratorFLAC()
{
	FLAC__stream_decoder_delete(flac);
}

bool AudioGeneratorFLAC::begin(AudioFileSource* source, AudioOutput* output)
{
	if(!source) {
		return false;
	}
	file = source;
	if(!output) {
		return false;
	}
	this->output = output;
	if(!file->isOpen()) {
		return false; // Error
	}

	flac = FLAC__stream_decoder_new();
	if(!flac) {
		return false;
	}

	(void)FLAC__stream_decoder_set_md5_checking(flac, false);

#define SELF() static_cast<AudioGeneratorFLAC*>(client_data)

	// FLAC callbacks, need static functions to bounce into c++ from c
	auto _read_cb = [](const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes,
					   void* client_data) -> FLAC__StreamDecoderReadStatus {
		return SELF()->read_cb(decoder, buffer, bytes);
	};
	auto _seek_cb = [](const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset,
					   void* client_data) -> FLAC__StreamDecoderSeekStatus {
		return SELF()->seek_cb(decoder, absolute_byte_offset);
	};
	auto _tell_cb = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset,
					   void* client_data) -> FLAC__StreamDecoderTellStatus {
		return SELF()->tell_cb(decoder, absolute_byte_offset);
	};
	auto _length_cb = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* stream_length,
						 void* client_data) -> FLAC__StreamDecoderLengthStatus {
		return SELF()->length_cb(decoder, stream_length);
	};
	auto _eof_cb = [](const FLAC__StreamDecoder* decoder, void* client_data) -> FLAC__bool {
		return SELF()->eof_cb(decoder);
	};
	auto _write_cb = [](const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[],
						void* client_data) -> FLAC__StreamDecoderWriteStatus {
		return SELF()->write_cb(decoder, frame, buffer);
	};
	auto _metadata_cb = [](const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata,
						   void* client_data) { SELF()->metadata_cb(decoder, metadata); };
	auto _error_cb = [](const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data) {
		SELF()->error_cb(decoder, status);
	};

	FLAC__StreamDecoderInitStatus ret = FLAC__stream_decoder_init_stream(
		flac, _read_cb, _seek_cb, _tell_cb, _length_cb, _eof_cb, _write_cb, _metadata_cb, _error_cb, this);
	if(ret != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		FLAC__stream_decoder_delete(flac);
		flac = NULL;
		return false;
	}

	output->begin();
	running = true;
	return true;
}

bool AudioGeneratorFLAC::loop()
{
	FLAC__bool ret;

	if(!running)
		goto done;

	if(!output->ConsumeSample(lastSample))
		goto done; // Try and send last buffered sample

	while(running && output->ConsumeSample(lastSample)) {
		if(buffPtr == buffLen) {
			ret = FLAC__stream_decoder_process_single(flac);
			if(!ret) {
				running = false;
				goto done;
			} else {
				// We might be done...
				if(FLAC__stream_decoder_get_state(flac) == FLAC__STREAM_DECODER_END_OF_STREAM) {
					running = false;
					goto done;
				}

				unsigned newsr = FLAC__stream_decoder_get_sample_rate(flac);
				if(newsr != sampleRate) {
					sampleRate = newsr;
					output->SetRate(sampleRate);
				}

				unsigned newch = FLAC__stream_decoder_get_channels(flac);
				if(newch != channels) {
					channels = newch;
					output->SetChannels(channels);
				}

				unsigned newbps = FLAC__stream_decoder_get_bits_per_sample(flac);
				if(newbps != bitsPerSample) {
					bitsPerSample = newbps;
					output->SetBitsPerSample(bitsPerSample);
				}
			}
		}

		// Check for some weird case where above didn't give any data
		if(buffPtr == buffLen) {
			goto done; // At some point the flac better error and we'll retudn
		}
		if(bitsPerSample <= 16) {
			lastSample[AudioOutput::LEFTCHANNEL] = buff[0][buffPtr] & 0xffff;
			if(channels == 2)
				lastSample[AudioOutput::RIGHTCHANNEL] = buff[1][buffPtr] & 0xffff;
			else
				lastSample[AudioOutput::RIGHTCHANNEL] = lastSample[AudioOutput::LEFTCHANNEL];
		} else if(bitsPerSample <= 24) {
			lastSample[AudioOutput::LEFTCHANNEL] = (buff[0][buffPtr] >> 8) & 0xffff;
			if(channels == 2)
				lastSample[AudioOutput::RIGHTCHANNEL] = (buff[1][buffPtr] >> 8) & 0xffff;
			else
				lastSample[AudioOutput::RIGHTCHANNEL] = lastSample[AudioOutput::LEFTCHANNEL];
		} else {
			lastSample[AudioOutput::LEFTCHANNEL] = (buff[0][buffPtr] >> 16) & 0xffff;
			if(channels == 2)
				lastSample[AudioOutput::RIGHTCHANNEL] = (buff[1][buffPtr] >> 16) & 0xffff;
			else
				lastSample[AudioOutput::RIGHTCHANNEL] = lastSample[AudioOutput::LEFTCHANNEL];
		}
		buffPtr++;
	}

done:
	file->loop();
	output->loop();

	return running;
}

bool AudioGeneratorFLAC::stop()
{
	FLAC__stream_decoder_delete(flac);
	flac = NULL;
	running = false;
	output->stop();
	return true;
}

FLAC__StreamDecoderReadStatus AudioGeneratorFLAC::read_cb(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[],
														  size_t* bytes)
{
	(void)decoder;
	if(*bytes == 0)
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	*bytes = file->read(buffer, sizeof(FLAC__byte) * (*bytes));
	if(*bytes == 0)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}
FLAC__StreamDecoderSeekStatus AudioGeneratorFLAC::seek_cb(const FLAC__StreamDecoder* decoder,
														  FLAC__uint64 absolute_byte_offset)
{
	(void)decoder;
	if(!file->seek((int32_t)absolute_byte_offset, 0))
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}
FLAC__StreamDecoderTellStatus AudioGeneratorFLAC::tell_cb(const FLAC__StreamDecoder* decoder,
														  FLAC__uint64* absolute_byte_offset)
{
	(void)decoder;
	*absolute_byte_offset = file->getPos();
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus AudioGeneratorFLAC::length_cb(const FLAC__StreamDecoder* decoder,
															  FLAC__uint64* stream_length)
{
	(void)decoder;
	*stream_length = file->getSize();
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}
FLAC__bool AudioGeneratorFLAC::eof_cb(const FLAC__StreamDecoder* decoder)
{
	(void)decoder;
	if(file->getPos() >= file->getSize())
		return true;
	return false;
}
FLAC__StreamDecoderWriteStatus AudioGeneratorFLAC::write_cb(const FLAC__StreamDecoder* decoder,
															const FLAC__Frame* frame, const FLAC__int32* const buffer[])
{
	(void)decoder;
	// Hackish warning here.  FLAC sends the buffer but doesn't free it until the next call to decode_frame, so we stash
	// the pointers here and use it in our loop() instead of memcpy()'ing into yet another buffer.
	buffLen = frame->header.blocksize;
	buff[0] = buffer[0];
	if(frame->header.channels > 1)
		buff[1] = buffer[1];
	else
		buff[1] = buffer[0];
	buffPtr = 0;
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}
void AudioGeneratorFLAC::metadata_cb(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata)
{
	(void)decoder;
	(void)metadata;
	AUDIO_INFO("Metadata");
}
void AudioGeneratorFLAC::error_cb(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status)
{
	(void)decoder;
	cb.st((int)status, FLAC__StreamDecoderErrorStatusString[status]);
}
