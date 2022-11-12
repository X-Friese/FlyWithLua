#ifndef FMOD4XPDEFAULTBUS_H
#define FMOD4XPDEFAULTBUS_H

#include "fmod.h"

enum FMOD4XPDefaultBus
{
    FMOD4XPBusMaster,
    FMOD4XPBusRadios,
    FMOD4XPBusCopilot,
    FMOD4XPBusUI,
    FMOD4XPBusInterior,
    FMOD4XPBusExteriorUnprocessed,
    FMOD4XPBusExteriorProcessed,
    FMOD4XPBusAircraft,
    FMOD4XPBusEnvironment
};

class FMOD4XPSystem;

FMOD_CHANNELGROUP* getFmodDefaultXPChannelGroup(FMOD4XPDefaultBus defaultBus, FMOD4XPSystem* system);

#endif // FMOD4XPDEFAULTBUS_H
