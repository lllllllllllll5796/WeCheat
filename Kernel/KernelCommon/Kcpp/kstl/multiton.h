#pragma once

namespace eastl
{
	namespace experimental
	{
		/**
		 * \brief 多列模式
		 * \tparam 对象类型
		 * \tparam 区分对象的属性
		 */
		template < typename T, typename K = eastl::string>
		class multiton
		{
		public:
			template<typename... Args>
			static FORCEINLINE eastl::shared_ptr<T> get(const K& key, Args&&... args);

			template<typename... Args>
			static FORCEINLINE eastl::shared_ptr<T> get(K&& key, Args&&... args);

			multiton() = default;
			~multiton() = default;
			multiton(const multiton&) = delete;
			multiton& operator= (const multiton&) = delete;
			multiton(multiton&&) = delete;
			multiton&& operator=(multiton&&) = delete;
		private:
			template<typename Key, typename... Args>
			static eastl::shared_ptr<T> GetInstance(Key&& key, Args&&...args);


			inline static eastl::unordered_map<K, eastl::shared_ptr<T>> m_map;
		};

		//-------------------------------------------------------------------

		template<typename T, typename K>
		template<typename ...Args>
		FORCEINLINE eastl::shared_ptr<T> multiton<T, K>::get(const K& key, Args && ...args)
		{
			return GetInstance(eastl::move(key), eastl::forward<Args>(args)...);
		}

		template<typename T, typename K>
		template<typename ...Args>
		FORCEINLINE eastl::shared_ptr<T> multiton<T, K>::get(K&& key, Args && ...args)
		{
			return GetInstance(eastl::move(key), eastl::forward<Args>(args)...);
		}


		template<typename T, typename K>
		template<typename Key, typename ...Args>
		FORCEINLINE eastl::shared_ptr<T> multiton<T, K>::GetInstance(Key&& key, Args && ...args)
		{
			eastl::shared_ptr<T> instance = nullptr;
			auto it = m_map.find(key);
			if (it == m_map.end())
			{
				instance = eastl::make_shared<T>(eastl::forward<Args>(args)...);
				m_map.insert(key, instance);
			}
			else
			{
				instance = it->second;
			}

			return instance;
		}
	}
}