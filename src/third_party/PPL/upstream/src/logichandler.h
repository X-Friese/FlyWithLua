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

#ifndef logichandler_h
#define logichandler_h

#include <vector>
#include <string>

namespace PPL {

/**
  * abstract class for all kinds of processing that takes values from XP,
  * do calculations in the flightloop and publish the result in a custom Dataref
  * @author (c) 2009-2011 by Philipp Muenzel
  * @version 0.4
  */
class LogicHandler {

 public:


    virtual ~LogicHandler() = default;

    /**
     * reimplement to do the setups that have to be done once when data is acessible
     */
    virtual bool initializeAtStart() = 0;


    /**
      * reimplement this function to re-initialize when a new user aircraft is loaded
      */
    virtual bool initializeOnAircraftChange() = 0;


    /**
     * reimplement to do the calculations that are done periodically (via the flightloop)
     */
    virtual bool processInTheLoop() = 0;


    /**
     * reimplement to process a signal from outside (e.g. interplugin message)
     */
    virtual bool processInput(long input, void* param) = 0;


    /**
     * reimplement to return the value for the next call of your processing- positive for seconds, negative for ticks
     */
    virtual float loopFrequency() = 0;


    /**
      * reimplement to suspend all processing by this handler if asked
      */
    virtual void suspend(bool yes) = 0;


    /**
     * reimplement to tell if this handler is currenty suspended
     */
    virtual bool isSuspended() const= 0;


    /**
      * name of this handler
      */
    virtual std::string name() const= 0;

    /**
      * enable processing for this handler and register in X-Planes flight-loop
      */
    void hookToSim();

    /**
      * stop processing and unregister from X-Planes flight-loop
      */
    void unhookFromSim();

private:

    /**
     * reimplement to register for the data needed for calculation
     */
    virtual bool aquireSimData() = 0;


    /**
     * reimplement to register the acessors to the owned data to form a custom dataref(s)
     */
    virtual bool publishData() = 0;


    /**
     * reimplement to unregister the custom dataref(s)
     */
    virtual bool withdrawPublishedData() = 0;

};

float HandlerCallbackInit(float, float, int, void* inRefCon);
float HandlerCallbackProcess(float, float, int, void* inRefCon);

}

#endif
