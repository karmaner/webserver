#ifndef __SRC_BASIC_SINGLETON_H__
#define __SRC_BASIC_SINGLETON_H__

#include <memory>

namespace webserver {

// 单例模式封装类
template <class T, class X = void, int N = 0> 
class Singleton {
public:
    static T *GetInstance() {
        static T v;
        return &v;
    }
};

template <class T, class X = void, int N = 0> 
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
        // return GetInstancePtr<T, X, N>();
    }
};

} // namespace webserver

#endif