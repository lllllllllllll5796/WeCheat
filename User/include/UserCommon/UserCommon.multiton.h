#pragma once
#include <unordered_map>

template < typename T, typename K = std::string>
class Multiton
{
public:
    template<typename... Args>
    static std::shared_ptr<T> get(const K& key, Args&&... args)
    {
        return GetInstance(std::move(key), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static std::shared_ptr<T> get(K&& key, Args&&... args)
    {
        return GetInstance(std::move(key), std::forward<Args>(args)...);
    }

    Multiton() = default;
    virtual ~Multiton() = default;
    Multiton(const Multiton&) = delete;
    Multiton& operator = (const Multiton&) = delete;
private:
    template<typename Key, typename... Args>
    static std::shared_ptr<T> GetInstance(Key&& key, Args&&...args)
    {
        std::shared_ptr<T> instance = nullptr;
        auto it = m_map.find(key);
        if (it == m_map.end())
        {
            instance = std::make_shared<T>(std::forward<Args>(args)...);
            m_map.emplace(key, instance);
        }
        else
        {
            instance = it->second;
        }

        return instance;
    }
private:
    static std::unordered_map<K, std::shared_ptr<T>> m_map;
};

template <typename T, typename K>
std::unordered_map<K, std::shared_ptr<T>> Multiton<T, K>::m_map;