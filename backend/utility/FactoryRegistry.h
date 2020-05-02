#ifndef _FACTORY_REGISTRY_H
#define _FACTORY_REGISTRY_H

#include <algorithm>
#include <functional>
#include <vector>

#include "Logger.h"

#include "../api/Resources.h"

// Base class for registries which register factory functions
template <typename Factory>
class FactoryRegistry
{
public:
    // Returns next unused id
    std::size_t GetFreeTypeId() const
    {
        auto it = std::find(m_factories.begin(), m_factories.end(), nullptr);
        return it == m_factories.end() ? m_factories.size() : (it - m_factories.begin());
    }
    // Registers factory for type. Returns true on success, false if passed nullptr or type is taken
    bool Register(Factory factory, std::size_t type)
    {
        if (factory == nullptr)
        {
            Res::Logger().Warning("Passed nullpointer to FactoryRegistry::Register()!");
            return false;
        }

        if (m_factories.size() <= type)
        {
            m_factories.resize(type + 1, nullptr);
            m_factories[type] = factory;
            return true;
        }
        else if (m_factories[type] == nullptr)
        {
            m_factories[type] = factory;
            return true;
        }
        else
        {
            // Type is already taken
            return false;
        }
    }
    // Removes the factory for type
    void Remove(std::size_t type)
    {
        if (m_factories.size() > type)
        {
            m_factories[type] = nullptr;
            while (!m_factories.empty() && m_factories.back() == nullptr)
            {
                m_factories.pop_back();
            }
        }
    }
    // Removes all factories
    void RemoveAll() { m_factories.clear(); }

protected:
    std::vector<Factory> m_factories;
};

#endif