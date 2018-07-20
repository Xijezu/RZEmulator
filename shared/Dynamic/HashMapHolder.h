#pragma once
#include "Common.h"
#include "SharedMutex.h"

template <class T>
class HashMapHolder
{
public:
    typedef std::unordered_map<uint32, T*> MapType;

    static void Insert(T* o)
    {
        {
            NG_UNIQUE_GUARD writeGuard(*GetLock());
            GetContainer()[o->GetHandle()] = o;
        }
    }

    static void Remove(T* o)
    {
        {
            NG_UNIQUE_GUARD writeGuard(*GetLock());
            GetContainer().erase(o->GetHandle());
        }
    }

    static T* Find(uint32 handle)
    {
        NG_SHARED_GUARD readGuard(*GetLock());
        typename MapType::iterator itr = GetContainer().find(handle);
        return (itr != GetContainer().end()) ? itr->second : nullptr;
    }

    static auto GetContainer() -> MapType&
    {
        static MapType m_objectMap;
        return m_objectMap;
    }
    static NG_SHARED_MUTEX* GetLock()
    {
        static NG_SHARED_MUTEX i_lock;
        return &i_lock;
    }

private:
    //Non instanceable only static
    HashMapHolder() = default;
    static NG_SHARED_MUTEX i_lock;
    static MapType m_objectMap;
};