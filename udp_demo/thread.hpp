#ifndef THREAD_HPP
#define THREAD_HPP
#include <iostream>
#include <pthread.h>
#include <functional>
#include <cstdio>
typedef void*(*fun_t)(void*);
class Thread{
private:
    fun_t _callback;
    void* _args;
    //线程id,线程启动后获取,值其实就是线程在进程地址空间中的地址
    pthread_t _tid;
public:
    //传入线程的回调函数,以及参数列表
    Thread(fun_t callback,void* args)
    :_callback(callback)
    ,_args(args)
    {}

    //线程启动
    void Start()
    {
        pthread_create(&_tid,nullptr,_callback,_args);
    }

    //线程等待回收资源
    void Join()
    {
        //这里不关心线程的返回值
        pthread_join(_tid,nullptr);
    }
    ~Thread(){}
};
#endif