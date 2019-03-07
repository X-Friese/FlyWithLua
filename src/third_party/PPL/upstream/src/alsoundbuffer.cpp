// Copyright (c) 2017, Philipp Ringler philipp@x-plane.com
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those
// of the authors and should not be interpreted as representing official policies,
// either expressed or implied, of the FreeBSD Project.

#include <sstream>
#include <vector>
#include <cstdio>
#include <stdint.h>

#include "alsoundbuffer.h"

using namespace PPL;

ALSoundBuffer::ALSoundBuffer(const std::string& filename):
    m_name(filename)
{
    ALfloat source_position[] = { 0.0, 0.0, 0.0 };
    ALfloat source_velocity[] = { 0.0, 0.0, 0.0 };
    m_loop = AL_FALSE;
    // Clear present errors
    alGetError();
    m_buffer = LoadWav(filename);

    if( m_buffer == AL_NONE)
    {
        std::stringstream stream;
        stream << "ALUT: Buffer creation failed: "/*
                                    << alutGetErrorString(alutGetError())*/;
        throw SoundBufferError(stream.str());
    }
    else
    {
        if (alGetError() != AL_NO_ERROR)
        {
            throw SoundBufferError("Error in creating buffer");
        }
        alGenSources(1, &m_source);
        if (alGetError() != AL_NO_ERROR)
        {
            throw SoundSourceError("Error in creating source");
        }
        alSourcei (m_source, AL_BUFFER,   m_buffer   );
        alSourcef (m_source, AL_PITCH,    1.0      );
        alSourcef (m_source, AL_GAIN,     1.0      );
        alSourcefv(m_source, AL_POSITION, source_position);
        alSourcefv(m_source, AL_VELOCITY, source_velocity);
        alSourcei (m_source, AL_LOOPING,  m_loop     );
        if (alGetError() != AL_NO_ERROR)
        {
            throw SoundSourceError("Error in setting up source");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ALSoundBuffer::~ALSoundBuffer()
{
    alDeleteSources(1, &m_source);
    alDeleteBuffers(1, &m_buffer);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ALSoundBuffer::play(float volume)
{
    if (!(alIsSource( m_source ) == AL_TRUE))
    {
        std::stringstream stream;
        stream << "Error: " << m_name << " has no source";
        throw SoundSourceError(stream.str());
    }
    ALint buf;
    alGetSourcei(m_source, AL_BUFFER, &buf);
    if (!(alIsBuffer(buf) == AL_TRUE))
    {
        std::stringstream stream;
        stream << "Error: " << m_name << " has no buffer";
        throw SoundBufferError(stream.str());
    }

    if (alGetError() != AL_NO_ERROR)
    {
        std::stringstream stream;
        stream << "Error, cannot play " << m_name;
        throw SoundPlayingError(stream.str());
    }
    ALfloat listener_position[]= { 0.0, 0.0, 0.0 };
    ALfloat listener_velocity[] = { 0.0, 0.0, 0.0 };
    ALfloat listener_orientation[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
    alListenerfv(AL_POSITION,    listener_position);
    alListenerfv(AL_VELOCITY,    listener_velocity);
    alListenerfv(AL_ORIENTATION, listener_orientation);
    alSourcef(m_source, AL_GAIN, volume);
    if (alGetError() != AL_NO_ERROR)
    {
        std::stringstream stream;
        stream << "Error cannot play " << m_name <<
                  ". Setup of source and listener failed";
        throw SoundPlayingError(stream.str());
    }
    alSourcePlay( m_source );
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALSoundBuffer::stop()
{
    alSourceStop( m_source );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALSoundBuffer::rewind()
{
    alSourceRewind( m_source );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALSoundBuffer::setLoop(bool yes)
{
    ALboolean loop = yes ? AL_TRUE : AL_FALSE;
    alSourcei( m_source, AL_LOOPING, loop );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ALSoundBuffer::isPlaying() const
{
    ALint state;
    alGetSourcei( m_source, AL_SOURCE_STATE, &state );
    return (state == AL_PLAYING);
}

namespace PPL {
static uint16_t readByte16(const unsigned char buffer[2]) {
#if APL && defined(__ppc__)
    return (buffer[0] << 8) + buffer[1];
#else
    return (buffer[1] << 8) + buffer[0];
#endif
}
static uint32_t readByte32(const unsigned char buffer[4]) {
#if APL && defined(__ppc__)
    return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
#else
    return (buffer[3] << 24) + (buffer[2] << 16) + (buffer[1] << 8) + buffer[0];
#endif
}
}
//-----------------------------------------

//  References:
//  -  http://ccrma.stanford.edu/courses/422/projects/WaveFormat/
//  -  http://www.borg.com/~jglatt/tech/wave.htm
//  -  Alut source code: static BufferData *loadWavFile (InputStream *stream)
//     http://www.openal.org/repos/openal/tags/freealut_1_1_0/alut/alut/src/alutLoader.c
ALuint PPL::LoadWav(const std::string& fileName)
{
    constexpr std::size_t BUFFER_SIZE = 32768;     // 32 KB buffers
    std::size_t bytes;
    std::vector<char> data;
    ALenum format;
    ALsizei freq;

    // Local resources
    FILE *f = nullptr;
    ALuint buffer = AL_NONE;

    alGetError();

    // Main process
    try {

        // Open for binary reading
        f = fopen(fileName.c_str(), "rb");
        if (!f)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Could not load wav from " + fileName);

        // buffers
        char magic[5];
        magic[4] = '\0';
        unsigned char buffer32[4];
        unsigned char buffer16[2];

        // check magic
        if(fread(magic,4,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        if(std::string(magic) != "RIFF")
            throw ALSoundBuffer::SoundBufferError("LoadWav: Wrong wav file format. This file is not a .wav file (no RIFF magic): "+ fileName );

        // skip 4 bytes (file size)
        fseek(f,4,SEEK_CUR);

        // check file format
        if(fread(magic,4,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        if(std::string(magic) != "WAVE")
            throw ALSoundBuffer::SoundBufferError("LoadWav: Wrong wav file format. This file is not a .wav file (no WAVE format): "+ fileName );

        // check 'fmt ' sub chunk (1)
        if(fread(magic,4,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        if(std::string(magic) != "fmt ")
            throw ALSoundBuffer::SoundBufferError("LoadWav: Wrong wav file format. This file is not a .wav file (no 'fmt ' subchunk): "+ fileName );

        // read (1)'s size
        if(fread(buffer32,4,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        uint32_t subChunk1Size = readByte32(buffer32);
        if (subChunk1Size < 16)
            throw ALSoundBuffer::SoundBufferError("Wrong wav file format. This file is not a .wav file ('fmt ' chunk too small, truncated file?): "+ fileName );

        // check PCM audio format
        if(fread(buffer16,2,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        uint16_t audioFormat = readByte16(buffer16);
        if (audioFormat != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Wrong wav file format. This file is not a .wav file (audio format is not PCM): "+ fileName );

        // read number of channels
        if(fread(buffer16,2,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        uint16_t channels = readByte16(buffer16);

        // read frequency (sample rate)
        if (fread(buffer32,4,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        uint32_t frequency = readByte32(buffer32);

        // skip 6 bytes (Byte rate (4), Block align (2))
        fseek(f,6,SEEK_CUR);

        // read bits per sample
        if(fread(buffer16,2,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        uint16_t bps = readByte16(buffer16);

        if (channels == 1)
            format = (bps == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
        else
            format = (bps == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

        // check 'data' sub chunk (2)
        if(fread(magic,4,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        if(std::string(magic) != "data")
            throw ALSoundBuffer::SoundBufferError("LoadWav: Wrong wav file format. This file is not a .wav file (no data subchunk): "+ fileName );

        if(fread(buffer32,4,1,f) != 1)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Cannot read wav file "+ fileName );
        uint32_t subChunk2Size = readByte32(buffer32);

        // The frequency of the sampling rate
        freq = frequency;
        if (sizeof(freq) != sizeof(frequency))
            throw ALSoundBuffer::SoundBufferError("LoadWav: freq and frequency different sizes");

        auto array = std::vector<char>(BUFFER_SIZE);

        while (data.size() != subChunk2Size)
        {
            // Read up to a buffer's worth of decoded sound data
            bytes = fread(array.data(), 1, array.size(), f);

            if (bytes <= 0)
                break;

            if (data.size() + bytes > subChunk2Size)
                bytes = subChunk2Size - data.size();

            // Append to end of buffer
            data.insert(data.end(), array.begin(), array.begin() + bytes);
        };

        fclose(f);
        f = nullptr;

        alGenBuffers(1, &buffer);
        if(alGetError() != AL_NO_ERROR)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Could not generate buffer");
        if(AL_NONE == buffer)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Could not generate buffer");

        alBufferData(buffer, format, &data[0], data.size(), freq);
        if(alGetError() != AL_NO_ERROR)
            throw ALSoundBuffer::SoundBufferError("LoadWav: Could not load buffer data");

        return buffer;
    }
    catch (std::runtime_error& er)
    {
        if (buffer && alIsBuffer(buffer) == AL_TRUE)
            alDeleteBuffers(1, &buffer);

        if (f)
            fclose(f);
        throw;
    }
}
