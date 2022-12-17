#ifndef FMOD4XPSYSTEM_H
#define FMOD4XPSYSTEM_H

#include <string>

#include "fmod.h"
#include "fmod_studio.h"

using namespace std;

class FMOD4XPChannelGroup;

class FMOD4XPSystem
{
private:
    FMOD_STUDIO_SYSTEM* fmodStudioSystem = nullptr;
    FMOD_SYSTEM* fmodCoreSystem = nullptr;
public:
    FMOD4XPSystem(int fmodStudioHi, int fmodStudioLo);
    FMOD_SOUND* loadSoundFile(string filePath);

    FMOD_STUDIO_SYSTEM* getFmodStudioSystem() const;
    FMOD_SYSTEM* getFmodCoreSystem() const;
};

#endif // FMOD4XPSYSTEM_H
