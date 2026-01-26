#pragma once

namespace eastl
{
	namespace experimental
	{
		template <class T>
		class singleton
		{
		public:
			singleton() = default;
			~singleton() = default;
			singleton(const singleton&) = delete;
			singleton(singleton&&) = delete;

			singleton& operator=(const singleton&) = delete;
			singleton&& operator=(singleton&&) = delete;

			template <typename ...Args>
			static T* get(Args&&... args);
		private:
			inline static eastl::unique_ptr<T> m_instance_ptr = nullptr;
		};

		template <class T>
		template <typename ...Args>
		T* singleton<T>::get(Args&&... args)
		{
			if (m_instance_ptr.get() == nullptr)
			{
				if (m_instance_ptr.get() == nullptr)
				{
					m_instance_ptr = eastl::make_unique<T>();
				}
			}
			return m_instance_ptr.get();
		}
	}
}