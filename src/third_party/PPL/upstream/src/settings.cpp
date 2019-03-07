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

#include "settings.h"
#include "log.h"

using namespace PPL;

Settings::Settings(const std::string& filename, bool create_if_not_exists, bool write_new_file):
        m_config_file_name(filename),
        m_file_did_exist_before(false),
        m_create_file_if_not_exists(create_if_not_exists),
        m_write_new_file(write_new_file)
{
    FILE* f = fopen(m_config_file_name.c_str(), "rb");
    if (f)
    {
        m_file_did_exist_before = true;
        fclose(f);
        Log() << Log::Info << "Using config file." << Log::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


Settings::~Settings()
{
    if (m_write_new_file)
    {
        if (m_file_did_exist_before || m_create_file_if_not_exists)
        {
            if (SI_OK != this->SaveFile(m_config_file_name.c_str()))
            {
                Log() << Log::Info << "Could not save config file" << Log::endl;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool Settings::loadFromFile()
{
    if (m_file_did_exist_before)
    {
        bool success = (SI_OK == this->LoadFile(m_config_file_name.c_str()));
        return success;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::string Settings::get(const std::string &section, const std::string &key)
{
    return std::string(GetValue(section.c_str(), key.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

long Settings::getLong(const std::string& section, const std::string& key)
{
    return GetLongValue(section.c_str(), key.c_str());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Settings::set(const std::string& section, const std::string& key, const std::string& value)
{
    SetValue(section.c_str(), key.c_str(), value.c_str());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Settings::setLong(const std::string& section, const std::string& key, long value)
{
    SetLongValue(section.c_str(), key.c_str(), value);
}
