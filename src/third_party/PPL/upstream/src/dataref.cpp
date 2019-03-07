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
#include <cmath>
#include <climits>
#include <limits>

#include "dataref.h"

using namespace PPL;


template<>
void DataRef<int>::shareDataRef(const std::string& identifier, bool publish_in_dre)
{
    share(XPLMShareData(identifier.c_str(), xplmType_Int, NotifactionFunc, this), publish_in_dre);
}

template<>
void DataRef<float>::shareDataRef(const std::string& identifier, bool publish_in_dre)
{
    share(XPLMShareData(identifier.c_str(), xplmType_Float, NotifactionFunc, this), publish_in_dre);
}

template<>
void DataRef<double>::shareDataRef(const std::string& identifier, bool publish_in_dre)
{
    share(XPLMShareData(identifier.c_str(), xplmType_Double, NotifactionFunc, this), publish_in_dre);
}

template<>
void DataRef<std::vector<int> >::shareDataRef(const std::string& identifier, bool publish_in_dre)
{
    share(XPLMShareData(identifier.c_str(), xplmType_IntArray, NotifactionFunc, this), publish_in_dre);
}

template<>
void DataRef<std::vector<float> >::shareDataRef(const std::string& identifier, bool publish_in_dre)
{
    share(XPLMShareData(identifier.c_str(), xplmType_FloatArray, NotifactionFunc, this), publish_in_dre);
}

template<>
void DataRef<std::string>::shareDataRef(const std::string& identifier, bool publish_in_dre)
{
    share(XPLMShareData(identifier.c_str(), xplmType_Data, NotifactionFunc, this), publish_in_dre);
}

template<>
void DataRef<int>::unshareData()
{
    XPLMUnshareData(identifier_.c_str(), xplmType_Int, NotifactionFunc, this);
}

template<>
void DataRef<float>::unshareData()
{
    XPLMUnshareData(identifier_.c_str(), xplmType_Float, NotifactionFunc, this);
}

template<>
void DataRef<double>::unshareData()
{
    XPLMUnshareData(identifier_.c_str(), xplmType_Double, NotifactionFunc, this);
}

template<>
void DataRef<std::vector<int> >::unshareData()
{
    XPLMUnshareData(identifier_.c_str(), xplmType_IntArray, NotifactionFunc, this);
}

template<>
void DataRef<std::vector<float> >::unshareData()
{
    XPLMUnshareData(identifier_.c_str(), xplmType_FloatArray, NotifactionFunc, this);
}

template<>
void DataRef<std::string>::unshareData()
{
    XPLMUnshareData(identifier_.c_str(), xplmType_Data, NotifactionFunc, this);
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template <>
void DataRef<int>::checkDataType()
{
    if ( XPLMGetDataRefTypes(m_data_ref) != xplmType_Int )
        throw IncompatibleTypeException(identifier_ + "declared to be int, but isn't.");
}

template <>
void DataRef<float>::checkDataType()
{
    if ( XPLMGetDataRefTypes(m_data_ref) != xplmType_Float )
        throw IncompatibleTypeException(identifier_ + "declared to be float, but isn't.");
}

template <>
void DataRef<double>::checkDataType()
{
    if ( XPLMGetDataRefTypes(m_data_ref) != (xplmType_Float | xplmType_Double) )
        throw IncompatibleTypeException(identifier_ + "declared to be double, but isn't.");
}

template <>
void DataRef<std::vector<float> >::checkDataType()
{
    if (XPLMGetDataRefTypes(m_data_ref) != xplmType_FloatArray)
        throw IncompatibleTypeException(identifier_ + "declared to be a float array, but isn't.");
}

template <>
void DataRef<std::vector<int> >::checkDataType()
{
    if (XPLMGetDataRefTypes(m_data_ref) != xplmType_IntArray)
        throw IncompatibleTypeException(identifier_ + "declared to be a int array, but isn't.");
}

template <>
void DataRef<std::string>::checkDataType()
{
    if (XPLMGetDataRefTypes(m_data_ref) != xplmType_Data)
        throw IncompatibleTypeException(identifier_ + "declared to be a byte array, but isn't.");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <>
DataRef<int>::operator int() const
{
    return XPLMGetDatai(m_data_ref);
}

template <>
DataRef<float>::operator float() const
{
    return XPLMGetDataf(m_data_ref);
}

template <>
DataRef<double>::operator double() const
{
    return XPLMGetDatad(m_data_ref);
}

template <>
DataRef<std::vector<float> >::operator std::vector<float>() const
{
    cache_.resize(XPLMGetDatavf(m_data_ref, nullptr, 0, 0));
    XPLMGetDatavf(m_data_ref, &cache_[0], 0, cache_.size());
    return cache_;
}

template <>
DataRef<std::vector<int> >::operator std::vector<int>() const
{
    cache_.resize(XPLMGetDatavi(m_data_ref, nullptr, 0, 0));
    XPLMGetDatavi(m_data_ref, &cache_[0], 0, cache_.size());
    return cache_;
}

template <>
DataRef<std::string>::operator std::string() const
{
    cache_.resize(XPLMGetDatab(m_data_ref, nullptr, 0, 0));
    XPLMGetDatab(m_data_ref, &cache_[0], 0, cache_.size());
    return std::string(cache_.data(), strnlen(cache_.data(), cache_.size()));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template <>
const DataRef<int>& DataRef<int>::operator=(const int& value)
{
    if (m_read_write != ReadOnly)
        XPLMSetDatai(m_data_ref, value);
    return *this;
}

template <>
const DataRef<float>& DataRef<float>::operator=(const float& value)
{
    if (m_read_write != ReadOnly)
        XPLMSetDataf(m_data_ref, value);
    return *this;
}

template <>
const DataRef<double>& DataRef<double>::operator=(const double& value)
{
    if (m_read_write != ReadOnly)
        XPLMSetDatad(m_data_ref, value);
    return *this;
}

template <>
const DataRef<std::vector<float> >& DataRef<std::vector<float> >::operator=(const std::vector<float>& value)
{
    if (m_read_write != ReadOnly)
        XPLMSetDatavf(m_data_ref, const_cast<float*>(&value[0]), 0, value.size());
    return *this;
}

template <>
const DataRef<std::vector<int> >& DataRef<std::vector<int> >::operator=(const std::vector<int>& value)
{
    if (m_read_write != ReadOnly)
        XPLMSetDatavi(m_data_ref, const_cast<int*>(&value[0]), 0, value.size());
    return *this;
}

template <>
const DataRef<std::string>& DataRef<std::string>::operator=(const std::string& value)
{
    if (m_read_write != ReadOnly)
        XPLMSetDatab(m_data_ref, const_cast<char*>(value.c_str()) , 0, value.size() + 1);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
dataref_trait<std::vector<float> >::BasicType DataRef<std::vector<float> >::operator[](std::size_t index) const
{
    const std::vector<float>& vf(*this);
    return vf[index];
}

template<>
dataref_trait<std::vector<int> >::BasicType DataRef<std::vector<int> >::operator[](std::size_t index) const
{
    const std::vector<int>& vi(*this);
    return vi[index];
}

template<>
dataref_trait<std::string>::BasicType DataRef<std::string>::operator[](std::size_t index) const
{
    cache_.resize(XPLMGetDatab(m_data_ref, nullptr, 0, 0));        // can't use convert to std::string method here, because we might want the raw data with embedded null bytes.
    XPLMGetDatab(m_data_ref, &cache_[0], 0, cache_.size());
    return cache_[index];
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template <>
bool DataRef<int>::hasChanged() const
{
    return m_history != operator int();
}

template <>
bool DataRef<float>::hasChanged() const
{
    return (std::fabs(m_history - operator float()) > std::numeric_limits<float>::epsilon() );
}

template <>
bool DataRef<double>::hasChanged() const
{
    return (std::fabs(m_history - operator double()) > std::numeric_limits<double>::epsilon() );
}

template <>
bool DataRef<std::vector<int> >::hasChanged() const
{
    const std::vector<int>& actual = operator std::vector<int>();
    if (m_history.size() != actual.size())
        return true;
    for (std::size_t i = 0; i < actual.size() ; ++i)
        if (actual[i] != m_history[i])
            return true;
    return false;
}

template <>
bool DataRef<std::vector<float> >::hasChanged() const
{
    const std::vector<float>& actual = operator std::vector<float>();
    if (m_history.size() != actual.size())
        return true;
    for (std::size_t i = 0; i < actual.size() ; ++i)
        if (std::fabs(actual[i] - m_history[i]) > std::numeric_limits<float>::epsilon())
            return true;
    return false;
}

template <>
bool DataRef<std::string>::hasChanged() const
{
    const std::string& actual = operator std::string();
    if (m_history.size() != actual.size())
        return true;
    return m_history != actual;
}


template <>
void DataRef<std::vector<int> >::forceChanged()
{
    const std::vector<int>& actual = operator std::vector<int>();
    if (m_history.size() != actual.size())
        return;
    for (std::size_t i = 0; i < m_history.size() ; ++i)
        m_history[i] = std::numeric_limits<int>::max();
}

template <>
void DataRef<std::vector<float> >::forceChanged()
{
    const std::vector<float>& actual = operator std::vector<float>();
    if (m_history.size() != actual.size())
        return;
    for (std::size_t i = 0; i < m_history.size() ; ++i)
        m_history[i] = std::numeric_limits<float>::max();
}

template <>
void DataRef<std::string>::forceChanged()
{
    const std::string& actual = operator std::string();
    if (m_history.size() != actual.size())
        return;
    for (std::size_t i = 0; i < m_history.size() ; ++i)
        m_history[i] = std::numeric_limits<char>::max();
}


template<>
void DataRef<std::vector<int> >::setVal(std::size_t pos, int val)
{
    if (cache_.size() <= pos)
        throw std::out_of_range("");
    cache_[pos] = val;
    XPLMSetDatavi(m_data_ref, const_cast<int*>(&cache_[pos]), pos, 1);
}

template<>
void DataRef<std::vector<float> >::setVal(std::size_t pos, float val)
{
    if (cache_.size() <= pos)
        throw std::out_of_range("");
    cache_[pos] = val;
    XPLMSetDatavf(m_data_ref, const_cast<float*>(&cache_[pos]), pos, 1);
}

template<>
void DataRef<std::string >::setVal(std::size_t pos, char val)
{
    if (cache_.size() <= pos)
        throw std::out_of_range("");
    cache_[pos] = val;
    XPLMSetDatab(m_data_ref, const_cast<char*>(&cache_[pos]), pos, 1);
}

template<>
void DataRef<std::vector<int> >::reserve(std::size_t i)
{
    if (cache_.size() < i)
        cache_.resize(i);
    XPLMSetDatavi(m_data_ref, const_cast<int*>(&cache_[0]), 0, i);
}

template<>
void DataRef<std::vector<float> >::reserve(std::size_t i)
{
    if (cache_.size() < i)
        cache_.resize(i);
    XPLMSetDatavf(m_data_ref, const_cast<float*>(&cache_[0]), 0, i);
}
template<>
void DataRef<std::string >::reserve(std::size_t i)
{
    if (cache_.size() < i)
        cache_.resize(i);
    XPLMSetDatab(m_data_ref, const_cast<char*>(&cache_[0]), 0, i);
}

template<>
void DataRef<std::vector<int> >::reserve()
{
    std::size_t i = XPLMGetDatavi(m_data_ref, nullptr, 0, 0);
    cache_.resize(i);
    m_history.resize(i);
}

template<>
void DataRef<std::vector<float> >::reserve()
{
    std::size_t i = XPLMGetDatavf(m_data_ref, nullptr, 0, 0);
    cache_.resize(i);
    m_history.resize(i);
}
template<>
void DataRef<std::string >::reserve()
{
    std::size_t i = XPLMGetDatab(m_data_ref, nullptr, 0, 0);
    cache_.resize(i);
    m_history.resize(i);
}


namespace PPL {

template class DataRef<float>;
template class DataRef<int>;
template class DataRef<double>;
template class DataRef<std::vector<float> >;
template class DataRef<std::vector<int> >;
template class DataRef<std::string>;

}

