#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>


namespace Mgfx
{
template <class T>
class Registrar
{
public:
    using Collection = std::unordered_map<std::type_index, std::shared_ptr<T>>;

    template <class T>
    auto Get() -> T*
    {
        return dynamic_cast<T*>(items[typeid(T)].get());
    };

    template <typename T, typename... Args>
    void Register(Args&&... args)
    {
        items[typeid(T)] = std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    void each(std::function<void(T* pSystem)> f)
    {
        for (auto& sys : items)
        {
            T* pSys = dynamic_cast<T*>(sys.second.get());
            if (pSys)
            {
                f(pSys);
            }
        }
    }

    auto Getitems() const -> const Collection&
    {
        return items;
    }

private:
    Collection items;
};

} // namespace Mgfx
