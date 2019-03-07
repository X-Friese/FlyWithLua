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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#define SI_CONVERT_GENERIC
#include "SimpleIni.h"

namespace PPL {

/**
  * Convenient access to ini-Files.
  * This is a small wrapper accessing the most frequently used basic functions of SimpleIni
  * @version 0.3
  * @author (c) 2009-2011 by Philipp Muenzel
  */
class Settings : private CSimpleIniA {
public:
    /**
      * init with the path to an ini.-file. If the file does not exist, it is created
      * and everything set via setValue is stored
      * @param filename path to ini-file
      * @param create_if_not_exists whether to create a new .ini-file with default settings if it wasn't there before
      * @param write_new_file whether to rewrite the file with updated settings on exit
      */
    Settings(const std::string& filename, bool create_if_not_exists = false, bool write_new_file = false);

    /**
      * if file should be created
      */
    ~Settings();

    /**
      * load settings from the file an overwrite everything that was set before
      */
    bool loadFromFile();

    /**
      * get a string value
      */
    std::string get(const std::string& section, const std::string& key);

    /**
      * get a long value
      */
    long getLong(const std::string& section, const std::string& key);

    /**
      * set a long value
      */
    void set(const std::string& section, const std::string& key, const std::string& value);

    /**
      * set a string value
      */
    void setLong(const std::string& section, const std::string& key, long value);


private:
    std::string m_config_file_name;
    bool m_file_did_exist_before;
    bool m_create_file_if_not_exists;
    bool m_write_new_file;
};

}
#endif // SETTINGS_H
