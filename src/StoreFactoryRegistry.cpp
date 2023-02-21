#include "pch.h"
#include "ruru.h"
#include "internal/basic_storage_engine.h"

static std::map<std::string, ruru::IStorageEngineFactory *> gEngineFactoryRegistry;

namespace ruru
{
    IStorageEngineFactory *getEngineFactory(const std::string &name)
    {
        if (gEngineFactoryRegistry.find(name) == gEngineFactoryRegistry.end())
            return nullptr;
        return gEngineFactoryRegistry[name];
    }

    bool registerEngineFactory(const std::string &name, IStorageEngineFactory *engineFactory)
    {
        if (gEngineFactoryRegistry.find(name) != gEngineFactoryRegistry.end())
            return false;
        gEngineFactoryRegistry[name] = engineFactory;
        return true;
    }

    void InitEngineFactory()
    {
        if (gEngineFactoryRegistry.find(_basic_factory) == gEngineFactoryRegistry.end())
        {
            internal::BasicStorageEngineFactory *factory = new internal::BasicStorageEngineFactory();
            gEngineFactoryRegistry[_basic_factory] = factory;
        }
    }

    void Init()
    {
        InitEngineFactory();
    }

}
