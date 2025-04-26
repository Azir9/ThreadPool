#include <iostream>
#include "Threadpool.h"
#include <thread>

/*
class MyAny{
public:
    MyAny(){};
    ~MyAny(){};

    MyAny(const MyAny&) = delete;
    MyAny(MyAny&&) = default;
    MyAny& operator=(const MyAny&)= delete;


    template<typename T>    
    MyAny(T data):base_(std::make_unique<Derive>(data)){}

    template<typename T>  
    T cast_(){
        Derive<T> *de_ = dynamic_cast<Derive<T>*>(data_.get())
        if (de_ == nullptr){
            throw "type is wrong";
        } 
        else return de_.data_;
    }

private:
    class Base{
    public:
        virtual ~Base(){};
    };

    template<typename T>
    class Derive: public Base{
    public:
        Derive(T data): data_(data)
        {}

        T data_;
    };
private:
    std::unique_ptr<Base> base_;
};

*/




//有些场景，是希望能够获取线程执行任务的返回值的
class MyTask :public Task{

public:
//设计一个返回值


    MyTask(int begin, int end)
        : start_(begin)
        , end_(end)
    {}
    std::any run(){
        std::cout << "Tid" << std::this_thread::get_id() << "Start!" <<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        int res = 0;
        for (int i =start_;start_<end_;start_++) res +=start_;
        
        std::cout << "Tid" << std::this_thread::get_id() << "End!" <<std::endl;

        return  res;
    }
private:
    int start_;
    int end_;
};



int  main() {

    ThreadPool pool;
    pool.setMode(PoolMpde::MODE_CACHED);
    //所有东西准备好之后，启动线程池
    pool.start();


    //线程通信
    
    Reselt res_1 = pool.submitTask(std::make_shared<MyTask>(1,900));
    Reselt res_2 = pool.submitTask(std::make_shared<MyTask>(1,900));
    Reselt res_3 = pool.submitTask(std::make_shared<MyTask>(1,900));
    Reselt res_4 = pool.submitTask(std::make_shared<MyTask>(1,900));
    Reselt res_5 = pool.submitTask(std::make_shared<MyTask>(1,900));
    Reselt res_6 = pool.submitTask(std::make_shared<MyTask>(1,900));

    int  a = std::any_cast<int>(res_1.get());
    int  b = std::any_cast<int>(res_2.get());
    int  c = std::any_cast<int>(res_3.get());
    int  d = std::any_cast<int>(res_4.get());
    int  e = std::any_cast<int>(res_5.get());
    int  f = std::any_cast<int>(res_6.get());
    //int  f = std::any_cast<int>(res_6.get());

    std::cout <<a<<b<<c<<d<<e<<f <<std::endl;
    return 0;
}