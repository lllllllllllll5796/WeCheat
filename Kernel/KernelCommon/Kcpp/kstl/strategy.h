#pragma once

namespace eastl
{
	namespace experimental
	{
		//strategy ²ßÂÔÄ£Ê½
		template <class Key, class RetType = void>
		class strategy
		{
		public:
			template <typename Func>
			void Set(const Key& key, Func&& lambda)
			{
				auto f = [&]()
				{
					return lambda();
				};
				m_map.insert(eastl::make_pair(key, f));
			}

			template <typename ...Args>
			auto Execute(const Key& key, Args&&... args)
			{
				auto it = m_map.find(key);
				auto itEnd = m_map.end();

				if (it == itEnd)
				{
					LOG_DEBUG("Strategy key = %d\n", key);
					return RetType();
				}
				return it->second(eastl::forward<Args>(args)...);
			}

		private:
			eastl::unordered_map<Key, eastl::function<RetType()>> m_map;
		};
	}
}