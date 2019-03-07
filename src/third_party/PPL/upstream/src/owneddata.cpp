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

#include <cstring>
#include <algorithm>

#include "owneddata.h"

using namespace PPL;

template <>
void OwnedData<int>::registerRead()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Int, 0,
                                                 readFunc<int>, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, this, nullptr);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<float>::registerRead()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Float, 0,
                                                 nullptr, nullptr, readFunc<float>, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, this, nullptr);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<double>::registerRead()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Double, 0,
                                                 nullptr, nullptr, nullptr, nullptr, readFunc<double>, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, this, nullptr);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<std::string>::registerRead()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Data, 0,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, readFuncStr, nullptr, this, nullptr);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<std::vector<float> >::registerRead()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_FloatArray, 0,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 readFuncVF, nullptr, nullptr, nullptr, this, nullptr);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template <>
void OwnedData<int>::registerWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Int, 1,
                                             nullptr, writeFunc<int>, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                             nullptr, nullptr, nullptr, nullptr, nullptr, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<float>::registerWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Float, 1,
                                             nullptr, nullptr, nullptr, writeFunc<float>, nullptr, nullptr, nullptr, nullptr,
                                             nullptr, nullptr, nullptr, nullptr, nullptr, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<double>::registerWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Double, 1,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, writeFunc<double>, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<std::string>::registerWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Data, 1,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, writeFuncStr, nullptr, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<std::vector<float> >::registerWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_FloatArray, 1,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, writeFuncVF, nullptr, nullptr, nullptr, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template <>
void OwnedData<int>::registerReadWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Int, 1,
                                                 readFunc<int>, writeFunc<int>, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, this, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<float>::registerReadWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Float, 1,
                                                 nullptr, nullptr, readFunc<float>, writeFunc<float>, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, this, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<double>::registerReadWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Double, 1,
                                                 nullptr, nullptr, nullptr, nullptr, readFunc<double>, writeFunc<double>, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, this, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<std::string>::registerReadWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_Data, 1,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, readFuncStr, writeFuncStr, this, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
void OwnedData<std::vector<float> >::registerReadWrite()
{
    m_data_ref = XPLMRegisterDataAccessor( m_data_ref_identifier.c_str(), xplmType_FloatArray, 1,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 readFuncVF, writeFuncVF, nullptr, nullptr, this, this);
    if (m_data_ref == nullptr)
        throw DataRefNotPublishedException(m_data_ref_identifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


int PPL::readFuncStr(void* inRefCon, void* outValue, int inOffset, int inMaxLength)
{
    OwnedData<std::string>* p_owned_data = static_cast<OwnedData<std::string>*>(inRefCon);
    std::size_t length = p_owned_data->value().length();
    if (outValue == nullptr)
        return length;
    std::size_t maxlen = std::min(std::size_t(inMaxLength), length);
    strncpy(static_cast<char*>(outValue), p_owned_data->value().substr(inOffset, maxlen).c_str(), maxlen);
    return inMaxLength;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PPL::writeFuncStr(void* inRefCon, void* inValue, int inOffset, int inMaxLength)
{
    OwnedData<std::string>* p_owned_data = static_cast<OwnedData<std::string>*>(inRefCon);
    char* str = static_cast<char*>(inValue);
    p_owned_data->setValue(std::string(str).substr(inOffset,inMaxLength));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int PPL::readFuncVF(void* inRefCon, float* outValues, int inOffset, int inMaxLength)
{
    std::size_t max_length = static_cast<unsigned int>(inMaxLength);
    OwnedData<std::vector<float> >* p_owned_data = static_cast<OwnedData<std::vector<float> >*>(inRefCon);
    if (outValues == nullptr)
        return p_owned_data->value().size();
    int end = (p_owned_data->value().size() < max_length) ? p_owned_data->value().size() : max_length;
    memcpy(outValues, &p_owned_data->value()[inOffset], sizeof(float)*end);
    return end;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PPL::writeFuncVF(void* inRefCon, float* inValues, int inOffset, int inMaxLength)
{
    OwnedData<std::vector<float> >* p_owned_data = static_cast<OwnedData<std::vector<float> >*>(inRefCon);
    p_owned_data->setValue(std::vector<float>(inValues+inOffset, inValues+inOffset+inMaxLength));
}
