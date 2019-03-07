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

#include "logwriter.h"
#include <iostream>
#include <cstdlib>

using namespace PPL;

LogWriter::LogWriter()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

LogWriter::~LogWriter()
{
    if (m_logfile)
        m_logfile.close();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void LogWriter::setLogFile(const std::string& filename)
{
    m_logfile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void LogWriter::postEntry(const LogEntry& entry)
{
    switch (entry.lvl) {
    case L_INFO:
        if (m_logfile)
            m_logfile << "[INFO] " << entry.txt << std::endl;
        else
            std::cout << "[INFO] " << entry.txt << std::endl;
        break;
    case L_WARN:
        if (m_logfile)
            m_logfile << "[WARN] " << entry.txt << std::endl;
        else
            std::cout << "[WARN] " << entry.txt << std::endl;
        break;
    case L_ERROR:
        if (m_logfile)
            m_logfile << "[ERROR] " << entry.txt << std::endl;
        else
            std::cerr << "[ERROR] " << entry.txt << std::endl;
        break;
    case L_FAIL:
        if (m_logfile)
            m_logfile << "[FAIL] " << entry.txt << std::endl;
        else
            std::cerr << "[FAIL] " << entry.txt << std::endl;
        std::exit(EXIT_FAILURE);
        break;
    default:
        std::cerr << "[FAIL-undefined] " << entry.txt << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

LogWriter& LogWriter::getLogger()
{
    // Meyers Singleton. Threadsafe in gcc, but not in MSVC !!!
    static LogWriter writer;
    return writer;
}
