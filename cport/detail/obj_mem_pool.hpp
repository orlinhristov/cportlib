#ifndef __OBJ_MEMORY_POOL_HPP__
#define __OBJ_MEMORY_POOL_HPP__

//
// obj_memory_pool.hpp
//
// Copyright (c) 2013-2016 Orlin Hristov (orlin dot hristov at gmail dot com)
//
// Distributed under the Apache License, Version 2.0
// visit http://www.apache.org/licenses/ for more information.
//

#include <deque>
#include <mutex>
#include <cassert>
#ifndef CPORT_DISABLE_OBJ_MEMORY_POOL
#define DECLARE_OBJ_MEMORY_POOL(ClassType) \
    private: \
        class ClassType##Pool : private std::deque<void *> \
        { \
            std::mutex mutex_; \
        public: \
            ~ClassType##Pool() \
            { \
                while(!empty()) { \
                    void *ptr = front(); \
                    pop_front(); \
                    ::operator delete(ptr); \
                } \
            } \
            void* get(std::size_t size) \
            { \
                assert(sizeof(ClassType) == size); \
                std::unique_lock<std::mutex> lock(mutex_); \
                if (empty()) { \
                    lock.unlock(); \
                    return ::operator new(size); \
                } \
                void *p = front(); \
                pop_front(); \
                return p; \
            } \
            void push(void *p) \
            { \
                std::unique_lock<std::mutex> lock(mutex_); \
                std::deque<void *>::push_back(p); \
            } \
        }; \
        static ClassType##Pool mem_pool_; \
    public: \
        void* operator new(std::size_t size) \
        { \
            return mem_pool_.get(size); \
        } \
        \
        void operator delete(void *p) \
        { \
            mem_pool_.push(p); \
        }

#define IMPLEMENT_OBJ_MEMORY_POOL(Class) \
    Class::Class##Pool Class::mem_pool_

#define IMPLEMENT_OBJ_MEMORY_POOL_T1(Class, T) \
    template <typename T> \
    typename Class<T>::Class##Pool Class<T>::mem_pool_

#define IMPLEMENT_OBJ_MEMORY_POOL_T2(Class, T1, T2) \
    template <typename T1, typename T2> \
    typename Class<T1, T2>::Class##Pool Class<T1, T2>::mem_pool_
#else // CPORT_DISABLE_OBJ_MEMORY_POOL
#define DECLARE_OBJ_MEMORY_POOL(ClassType)
#define IMPLEMENT_OBJ_MEMORY_POOL(Class)
#define IMPLEMENT_OBJ_MEMORY_POOL_T1(Class, T)
#define IMPLEMENT_OBJ_MEMORY_POOL_T2(Class, T1, T2)
#endif // CPORT_DISABLE_OBJ_MEMORY_POOL

#endif //__OBJ_MEMORY_POOL_HPP__
