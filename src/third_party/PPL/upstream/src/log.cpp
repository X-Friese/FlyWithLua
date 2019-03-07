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

#include <cassert>

#include "log.h"
#include "logwriter.h"

using namespace PPL;

Log::Log():
        m_severity(L_UNSPEC),
        m_writer(LogWriter::getLogger())
{}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::Info(Log& log)
{
    assert(log.m_stream.str() == "");
    log.m_severity = L_INFO;
    return log;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::Warn(Log& log)
{
    assert(log.m_stream.str() == "");
    log.m_severity = L_WARN;
    return log;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::Error(Log& log)
{
    assert(log.m_stream.str() == "");
    log.m_severity = L_ERROR;
    return log;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::Fail(Log& log)
{
    assert(log.m_stream.str() == "");
    log.m_severity = L_FAIL;
    return log;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::endl(Log& log)
{
    assert(log.m_stream.str() != "");
    assert(log.m_severity != L_UNSPEC);
    time_t now;
    LogEntry entry;
    if ( time(&now) != (time_t)(-1) )
        entry.time = now;
    else
        entry.time = 0;
    entry.lvl = log.m_severity;
    entry.txt = log.m_stream.str();
    log.m_writer.postEntry(entry);
    log.m_stream.str("");
    log.m_severity=L_UNSPEC;
    return log;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::operator<<(int16_t i)
{
    m_stream << i;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::operator<<(int32_t i)
{
    m_stream << i;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::operator<<(double d)
{
    m_stream << d;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::operator<<(char c)
{
    m_stream << c;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::operator<<(const std::string& s)
{
    m_stream << s;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Log& Log::operator<<(Log& (*f)(Log&))
{
    (*f)(*this);
    return *this;
}
