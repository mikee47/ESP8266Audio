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

#pragma once

#include <AudioGenerator.h>
extern "C" {
#include "libflac/FLAC/stream_decoder.h"
}

class AudioGeneratorFLAC : public AudioGenerator
{
public:
	AudioGeneratorFLAC();
	virtual ~AudioGeneratorFLAC() override;
	virtual bool begin(AudioFileSource* source, AudioOutput* output) override;
	virtual bool loop() override;
	virtual bool stop() override;
	virtual bool isRunning() override;

protected:
	// FLAC info
	uint16_t channels = 0;
	uint32_t sampleRate = 0;
	uint16_t bitsPerSample = 0;

	// We need to buffer some data in-RAM to avoid doing 1000s of small reads
	const int* buff[2] = {nullptr, nullptr};
	uint16_t buffPtr = 0;
	uint16_t buffLen = 0;
	FLAC__StreamDecoder* flac = nullptr;

	// FLAC callbacks
	FLAC__StreamDecoderReadStatus read_cb(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes);
	FLAC__StreamDecoderSeekStatus seek_cb(const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset);
	FLAC__StreamDecoderTellStatus tell_cb(const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset);
	FLAC__StreamDecoderLengthStatus length_cb(const FLAC__StreamDecoder* decoder, FLAC__uint64* stream_length);
	FLAC__bool eof_cb(const FLAC__StreamDecoder* decoder);
	FLAC__StreamDecoderWriteStatus write_cb(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame,
											const FLAC__int32* const buffer[]);
	void metadata_cb(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata);
	void error_cb(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status);
};
