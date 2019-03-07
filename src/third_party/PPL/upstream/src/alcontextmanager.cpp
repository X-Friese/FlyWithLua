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
#include <memory>

#include "XPLMUtilities.h"
#include "alcontextmanager.h"
#include "alcontextchanger.h"

using namespace PPL;

ALContextManager::ALContextManager():
    m_internal_counter(0),
    m_device(0),
    m_my_context(0),
    m_current_context(0)
{
    ALCcontext * old_ctx = alcGetCurrentContext();

    m_device = alcOpenDevice(0);

    std::ostringstream os;
    if (alcIsExtensionPresent (NULL, (const ALCchar *) "ALC_ENUMERATION_EXT") == AL_TRUE)
    {
        const char *s = (const char *) alcGetString(NULL, ALC_DEVICE_SPECIFIER);
        while (*s != '\0')
        {
            os << "OpenAL available device: " << s << std::endl;
            while (*s++ != '\0')
                ;
        }
    }
    else
    {
        os << "OpenAL device enumeration isn't available." << std::endl;
    }

    // Print default device name
    os << "OpenAL default device: "
       << (const char *)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER)
       << std::endl;

    // Print current device name
    if (m_device)
    {
        os << "OpenAL current device: "
           << (const char *)alcGetString(m_device, ALC_DEVICE_SPECIFIER)
           << std::endl;
    }

    XPLMDebugString(os.str().c_str());

    if (!m_device || alGetError() != AL_NO_ERROR)
        throw ALContextManager::SoundLoadError("Can't open device");

    m_my_context = alcCreateContext(m_device, 0);

    if(m_my_context == NULL)
    {
        if(old_ctx)
            alcMakeContextCurrent(old_ctx);
        alcCloseDevice(m_device);
        m_device = NULL;
        throw ALContextManager::SoundLoadError("Could not create a context.");
    }


    if (alGetError() != AL_NO_ERROR)
        throw ALContextManager::SoundLoadError("Error after creating context");

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ALContextManager::~ALContextManager()
{
    deleteAllSounds();

    m_current_context = alcGetCurrentContext();
    if (m_current_context == m_my_context)
        alcMakeContextCurrent(NULL);
    alcSuspendContext(m_my_context);
    /*
         * FIXME
         * Technically you're supposed to detach and destroy the
         * current context with these two lines, but it deadlocks.
         * As a not-quite-as-clean shortcut, if we skip this step
         * and just close the device, OpenAL theoretically
         * destroys the context associated with that device.
         *
         * alcDestroyContext(m_my_context);
         */
    /*
         * FIXME
         * Technically you're supposed to close the device, but
         * the OpenAL sound thread crashes if we do this manually.
         * The device seems to be closed automagically anyway.
         *
         *  alcCloseDevice(m_device);
         */
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ALSoundBuffer* ALContextManager::findSoundById(int id)
{
    auto it = m_sounds.find(id);
    if (it != m_sounds.end())
    {
        return it->second.get();
    }
    else
    {
        std::stringstream stream;
        stream << "Sound number " << id << " nonexistant.";
        throw SoundNotFoundError(stream.str());
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int ALContextManager::addSoundFromFile(const std::string& filename)
{
    ALContextChanger cc(m_my_context);
    // object cc is not used, but it changes context, and since cc is on the stack,
    // it is destroyed when function exits, which changes back the contexts!
    // this is known as the RAII idiom
    try
    {
        auto return_value = m_sounds.insert(std::make_pair(++m_internal_counter, std::make_unique<ALSoundBuffer>(filename)));

        if (return_value.second == false)
        {
            std::stringstream stream;
            stream << "Failure inserting soundfile " << filename;
            throw SoundLoadError(stream.str());
        }
    }
    catch (ALSoundBuffer::SoundPlayingError& ex)
    {
        std::stringstream stream;
        stream << "Failure inserting soundfile " << filename << " AL error: "
               << ex.what();
        throw SoundLoadError(stream.str());
    }
    return m_internal_counter;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ALContextManager::playSound(int id, float volume)
{
    ALSoundBuffer* sound = findSoundById(id);
    try
    {
        ALContextChanger cc(m_my_context);
        sound->play(volume);
        return true;
    }
    catch(ALSoundBuffer::SoundPlayingError& ex)
    {
        std::stringstream stream;
        stream << "Sound number " << id << " failed to play with message: "
               << ex.what();
        throw SoundPlayError(stream.str());
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALContextManager::stopSound(int id)
{
    ALSoundBuffer* sound = findSoundById(id);
    ALContextChanger cc(m_my_context);
    sound->stop();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALContextManager::rewindSound(int id)
{
    ALSoundBuffer* sound = findSoundById(id);
    ALContextChanger cc(m_my_context);
    sound->rewind();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALContextManager::loopSound(int id)
{
    ALSoundBuffer* sound = findSoundById(id);
    ALContextChanger cc(m_my_context);
    sound->setLoop(true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALContextManager::unLoopSound(int id)
{
    ALSoundBuffer* sound = findSoundById(id);
    ALContextChanger cc(m_my_context);
    sound->setLoop(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ALContextManager::isPlayingSound(int id)
{
    ALSoundBuffer* sound = findSoundById(id);
    ALContextChanger cc(m_my_context);
    return sound->isPlaying();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALContextManager::removeSound(int id)
{
    auto it = m_sounds.find(id);
    if (it != m_sounds.end())
    {
        ALContextChanger cc(m_my_context);
        m_sounds.erase(it);
    }
    else
    {
        std::stringstream stream;
        stream << "Sound number " << id << " nonexistant.";
        throw SoundNotFoundError(stream.str());
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ALContextManager::deleteAllSounds()
{
    ALContextChanger cc(m_my_context);
    for (auto& it: m_sounds)
        it.second->stop();
    m_sounds.clear();
}
