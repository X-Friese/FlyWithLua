#ifndef FWL_DATAREF_H
#define FWL_DATAREF_H

#include "XPLMDataAccess.h"

#include <string>
#include <iostream>

using namespace std;

template <typename T>
class DataRef
{
protected:
    const string xpPath;
    XPLMDataRef xpPointer = nullptr;
    bool writtable = false;
    T precedingValue;
    bool wasModified = false;

    void checkDatarefCorrectType() const
    {
        return;
    }

    void checkDatarefExisting()
    {
        if(wasFound())
        {
            return;
        }
        else
        {
            find();
            if(!wasFound())
            {
                throw runtime_error("Trying to access a not-found-dataref");
            }
        }
    }

    void checkDatarefWrittable()
    {
        if(writtable)
        {
            return;
        }
        else if(XPLMCanWriteDataRef(xpPointer))
        {
            writtable = true;
        }
        else
        {
            throw runtime_error("Trying to write a read-only-dataref");
        }
    }

    void registerModification()
    {
        if(!wasModified)
        {
            wasModified = true;
            precedingValue = get();
        }
    }
public:
    explicit DataRef(const string &path) : xpPath(path)
    {
    }

    void find()
    {
        xpPointer = XPLMFindDataRef(xpPath.c_str());
        if(wasFound())
        {
            checkDatarefCorrectType();
        }
        // It's legal to try to find a dataref without finding it... Just don't try to use it later
    }

    bool wasFound() const
    {
        return nullptr != xpPointer;
    }

    T get()
    {
        return T();
    }

    void set(T newValue)
    {
        (void)newValue;
    }

    void resetPreceding()
    {
        wasModified = false;
    }

    void restorePreceding()
    {
        if(wasModified)
        {
            set(precedingValue);
        }
    }
};

template <>
inline void DataRef<int>::checkDatarefCorrectType() const
{

    if(0 == (xplmType_Int & XPLMGetDataRefTypes(xpPointer)))
    {
        throw logic_error("Uncompatible dataref type");
    }
}

template <>
inline void DataRef<float>::checkDatarefCorrectType() const
{
    if(0 == (xplmType_Float & XPLMGetDataRefTypes(xpPointer)))
    {
        throw logic_error("Uncompatible dataref type");
    }
}

template <>
inline void DataRef<double>::checkDatarefCorrectType() const
{
    if(0 == (xplmType_Double & XPLMGetDataRefTypes(xpPointer)))
    {
        throw logic_error("Uncompatible dataref type");
    }
}

template <>
inline int DataRef<int>::get()
{
    DataRef<int>::checkDatarefExisting();
    return XPLMGetDatai(xpPointer);
}

template <>
inline float DataRef<float>::get()
{
    DataRef<float>::checkDatarefExisting();
    return XPLMGetDataf(xpPointer);
}

template <>
inline double DataRef<double>::get()
{
    DataRef<double>::checkDatarefExisting();
    return XPLMGetDatad(xpPointer);
}

template <>
inline void DataRef<int>::set(int newValue)
{
    DataRef<int>::checkDatarefExisting();
    DataRef<int>::checkDatarefWrittable();
    DataRef<int>::registerModification();
    XPLMSetDatai(xpPointer, newValue);
}

template <>
inline void DataRef<float>::set(float newValue)
{
    DataRef<float>::checkDatarefExisting();
    DataRef<float>::checkDatarefWrittable();
    DataRef<float>::registerModification();
    XPLMSetDataf(xpPointer, newValue);
}

template <>
inline void DataRef<double>::set(double newValue)
{
    DataRef<double>::checkDatarefExisting();
    DataRef<double>::checkDatarefWrittable();
    DataRef<double>::registerModification();
    XPLMSetDatad(xpPointer, newValue);
}

#endif // FWL_DATAREF_H
