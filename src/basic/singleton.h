#pragma once

#include <memory>

namespace webserver {

namespace {

template<class T, class X, int N>
T& GetInstanceX() {
	static T v;
	return v;
}

template<class T, class X, int N>
std::shared_ptr<T> GetInstancePtr() {
	static std::shared_ptr<T> v(new T);
	return v;
}

}

// 单例模式封装类
template <class T, class X = void, int N = 0> 
class Singleton {
public:
	static T *GetInstance() {
		return &GetInstanceX<T, X, N>();
	}
};

template <class T, class X = void, int N = 0> 
class SingletonPtr {
public:
	static std::shared_ptr<T> GetInstance() {
		return GetInstancePtr<T, X, N>();
	}
};

} // namespace webserver