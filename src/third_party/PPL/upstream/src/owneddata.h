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

#ifndef OWNEDDATA_H
#define OWNEDDATA_H

#include <string>
#include <vector>
#include <stdexcept>

#include "XPLMDataAccess.h"

#include "dataref.h"

namespace PPL {

template <typename T>
T readFunc(void*);
template <typename T>
void writeFunc(void*, T);

int readFuncStr(void*, void*, int, int);
void writeFuncStr(void*, void*, int, int);

int readFuncVF(void*, float*, int, int);
void writeFuncVF(void*, float*, int, int);

class DataRefNotPublishedException : public std::runtime_error {
public:
    DataRefNotPublishedException(const std::string& msg):
        std::runtime_error(msg)
    {}
};

/**
  * Data owned by the plugin and published as a XPLM DataRef.
  * create an instance and call a register function to make data accessible to other plugins
  * @author (c) 2009-2011 by Philipp Muenzel
  * @version 0.6
  */
template <typename T>
class OwnedData{
public:

    typedef void (*DataCallback_f)(const T&);

    /**
      * create owned date for sharing.
      * the identifier provided is the string identifier later used for the dataref.
      * the dataref is then registered in X-Plane's plugin system
      * Please consult the XP SDK naming conventions on how to name things right.
      * @param std::string& identifier
      * @param RWType set the dataref writeable to OTHER plugins
      * @param publish_in_dre by default, a message is sent to the DataRefEditor plugin that exposes the dataref
      * @param callback optional callback function pointer to be called when other plugins write to the dataref
      * @exception throws DataRefNotPublishedException if publishing in X-Plane fails
      */
    OwnedData(const std::string& identifier,
              RWType read_write = ReadOnly,
              bool publish_in_dre = false,
              DataCallback_f callback = 0):
        m_data_ref_identifier(identifier),
        m_data_ref(nullptr),
        m_value(T()),
        m_callback(callback)
    {
        switch(read_write)
        {
        case ReadOnly :
            registerRead();
            break;
        case WriteOnly :
            registerWrite();
            break;
        case ReadWrite:
            registerReadWrite();
            break;
        }
        if (publish_in_dre)
        {
            XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
            if (PluginID != XPLM_NO_PLUGIN_ID)
                XPLMSendMessageToPlugin(PluginID, DRE_MSG_ADD_DATAREF, (void*)m_data_ref_identifier.c_str());
        }
    }

    /**
      * upon destruction, the dataref is de-registered from X-Plane
      */
    ~OwnedData() { unregister(); }

    OwnedData(const OwnedData&) = delete;
    OwnedData& operator=(const OwnedData&) = delete;

    /**
      * acces the currently stored value
      * @return value
      */
    T value() const { return m_value; }

    /**
      * acces the currently stored value
      * @return value
      */
    operator T() const { return m_value; }

    /**
      * set the value so all other monitors of the dataref get it
      * @param val
      */
    const OwnedData& operator=(const T& val) { m_value = val; return *this; }

    /**
      * set the value so all other monitors of the dataref get it
      * @param val
      */
    void setValue(const T& val)
    {
        m_value = val;
        if (m_callback)
            m_callback(val);
    }


private:

    void registerRead();

    void registerWrite();

    void registerReadWrite();

    void unregister()
    {
        if (m_data_ref)
        {
            XPLMUnregisterDataAccessor(m_data_ref);
            m_data_ref = 0;
        }
    }

private:

    std::string m_data_ref_identifier;
    XPLMDataRef m_data_ref;
    T m_value;
    DataCallback_f m_callback;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename T>
T readFunc(void* inRefCon)
{
    OwnedData<T>* p_owned_data = static_cast<OwnedData<T>*>(inRefCon);
    return p_owned_data->value();
}

template <typename T>
void writeFunc(void* inRefCon, T data)
{
    OwnedData<T>* p_owned_data = static_cast<OwnedData<T>*>(inRefCon);
    p_owned_data->setValue(data);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<int>::registerRead();
template <>
void OwnedData<float>::registerRead();
template <>
void OwnedData<double>::registerRead();
template <>
void OwnedData<std::string>::registerRead();
template <>
void OwnedData<std::vector<float> >::registerRead();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<int>::registerWrite();
template <>
void OwnedData<float>::registerWrite();
template <>
void OwnedData<double>::registerWrite();
template <>
void OwnedData<std::string>::registerWrite();
template <>
void OwnedData<std::vector<float> >::registerWrite();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<int>::registerReadWrite();
template <>
void OwnedData<float>::registerReadWrite();
template <>
void OwnedData<double>::registerReadWrite();
template <>
void OwnedData<std::string>::registerReadWrite();
template <>
void OwnedData<std::vector<float> >::registerReadWrite();

}

#endif // OWNEDDATA_H
