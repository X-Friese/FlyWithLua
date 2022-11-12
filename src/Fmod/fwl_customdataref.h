#ifndef FWL_CUSTOMDATAREF_H
#define FWL_CUSTOMDATAREF_H

#include "XPLMDataAccess.h"

#include <string>

using namespace std;

template <typename T>
class CustomDataRef
{
protected:
    const string xpPath;
    XPLMDataRef xpPointer = nullptr;

    bool writtable = false;
    T value = T();
public:
    CustomDataRef(const string &path, bool writtable_p) :
        xpPath(path), writtable(writtable_p)
    {
    }

    ~CustomDataRef()
    {
        unregisterAccessor();
    }

    T get() const
    {
        return value;
    }

    static T get(void* inRefcon)
    {
        return reinterpret_cast<CustomDataRef*>(inRefcon)->get();
    }

    void set(T newValue)
    {
        if(writtable)
        {
            value = newValue;
        }
    }

    static void set(void* inRefcon, T newValue)
    {
        reinterpret_cast<CustomDataRef*>(inRefcon)->set(newValue);
    }

    const char* getPath() const
    {
        return xpPath.c_str();
    }

    void registerAccessor()
    {
    }

    void unregisterAccessor()
    {
    }
};

template <>
inline void CustomDataRef<int>::registerAccessor()
{
    if(nullptr == xpPointer)
    {
        xpPointer = XPLMRegisterDataAccessor(xpPath.c_str(), xplmType_Int, writtable ? 1 : 0,
                                             CustomDataRef::get, CustomDataRef::set,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             this, this);
    }
}

template <>
inline void CustomDataRef<float>::registerAccessor()
{
    if(nullptr == xpPointer)
    {
        xpPointer = XPLMRegisterDataAccessor(xpPath.c_str(), xplmType_Float, writtable ? 1 : 0,
                                             nullptr, nullptr,
                                             CustomDataRef::get, CustomDataRef::set,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             this, this);
    }
}

template <>
inline void CustomDataRef<double>::registerAccessor()
{
    if(nullptr == xpPointer)
    {
        xpPointer = XPLMRegisterDataAccessor(xpPath.c_str(), xplmType_Double, writtable ? 1 : 0,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             CustomDataRef::get, CustomDataRef::set,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             nullptr, nullptr,
                                             this, this);
    }
}

template <>
inline void CustomDataRef<int>::unregisterAccessor()
{
    if(nullptr != xpPointer)
    {
        XPLMUnregisterDataAccessor(xpPointer);
        xpPointer = nullptr;
    }
}

template <>
inline void CustomDataRef<float>::unregisterAccessor()
{
    if(nullptr != xpPointer)
    {
        XPLMUnregisterDataAccessor(xpPointer);
        xpPointer = nullptr;
    }
}

template <>
inline void CustomDataRef<double>::unregisterAccessor()
{
    if(nullptr != xpPointer)
    {
        XPLMUnregisterDataAccessor(xpPointer);
        xpPointer = nullptr;
    }
}

#endif // FWL_CUSTOMDATAREF_H
