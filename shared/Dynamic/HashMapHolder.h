#ifndef PROJECT_HASHMAPHOLDER_H
#define PROJECT_HASHMAPHOLDER_H

#include "Common.h"
#include "SharedMutex.h"

template <class T>
class HashMapHolder
{
public:
    typedef UNORDERED_MAP<uint32, T*> MapType;

    static void Insert(T* o)
    {
        {
            MX_UNIQUE_GUARD writeGuard(*GetLock());
            GetContainer()[o->GetHandle()] = o;
        }
    }

    static void Remove(T* o)
    {
        {
            MX_UNIQUE_GUARD writeGuard(*GetLock());
            GetContainer().erase(o->GetHandle());
        }
    }

    static T* Find(uint32 handle)
    {
        MX_SHARED_GUARD readGuard(*GetLock());
        typename MapType::iterator itr = GetContainer().find(handle);
        return (itr != GetContainer().end()) ? itr->second : nullptr;
    }

    static auto GetContainer() -> MapType&
    {
        static MapType m_objectMap;
        return m_objectMap;
    }
    static MX_SHARED_MUTEX* GetLock()
    {
        static MX_SHARED_MUTEX i_lock;
        return &i_lock;
    }

private:
    //Non instanceable only static
    HashMapHolder() = default;
    static MX_SHARED_MUTEX i_lock;
    static MapType m_objectMap;
};


#endif // PROJECT_HASHMAPHOLDER_H
