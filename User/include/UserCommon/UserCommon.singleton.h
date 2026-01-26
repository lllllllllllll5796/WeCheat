#pragma once
#include <memory>
#include "UserCommon.Interlocked.h"

template <class T>
class Singleton
{
public:
	static inline T* get();
	Singleton(void) {
		//OutputDebugStringA("Singleton\r\n");
	}
	~Singleton(void) {}

private:
	Singleton(const Singleton&) {}
	Singleton & operator= (const Singleton &) {}

	static std::shared_ptr<T> _instance;
	static CInterlocked _rs;
};

template <class T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

template <class T>
CInterlocked Singleton<T>::_rs;
template <class T>
inline T* Singleton<T>::get()
{
	if (0 == _instance.get())
	{
		CInterlocked::CGuard gd(_rs);
		if (0 == _instance.get())
		{
			_instance.reset(new T);
		}
	}
	return _instance.get();
}