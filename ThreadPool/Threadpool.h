#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <any>
#include <unordered_map>
//#include <semaphore>


class Semaphore{
private:
    std::mutex mux_;
    std::condition_variable cv_;
    int  res_limit_;
public:
    Semaphore(int limit = 0):res_limit_(limit){};
    ~Semaphore() = default;


    void wait(){
        std::unique_lock<std::mutex> lock_(mux_);
        // 等待信号量有资源，没有资源的话，会阻塞当前线程
        cv_.wait(lock_,[&]()->bool{return res_limit_>0;});
        res_limit_--;
    }
    //增加一个信号量资源
    void post(){
        std::unique_lock<std::mutex> lock_(mux_);
        //
        res_limit_++;
        cv_.notify_all();
    }
    
};

//Task
class Reselt;

//template<typename... T>
class Task{
public:
    void exec();
    void setResult(Reselt* result);
    virtual std::any run()= 0;
    Task():res_(nullptr){};
private:
    Reselt* res_; //对象的生命周期是要长于task的
};

enum PoolMpde{
    MODE_FIXED,
    MODE_CACHED,
};


class Reselt
{
public:
    Reselt(std::shared_ptr<Task> task,bool isVal=true);
    ~Reselt() =  default;
    //setval 方法，执行任务执行完的返回值 
    // 1.锁一下，或者根据isvalid 查看结果
    void setValue(std::any any_);
    //get 用户调用这个方法获取task返回值
    // 如果没有执行完，需要等待在这个信号上面
    std::any get();
private:
    std::any any_;
    Semaphore sem_;
    std::shared_ptr<Task> Task_; //指向对应的获取返回值的任务对象 
    std::atomic_bool isValid_;

};



class Thread
{

public:
    //线程函数对象类型
    using ThreadFunc = std::function<void(int)>;
    //线程构造
    Thread(ThreadFunc a);

    //线程析构
    ~Thread();
    //启动线程
    void start();

    //get the Id of thread
    int getId()const;
private:
    ThreadFunc func_;
    inline static  int generateId_ = 0;
    int threadId_;

};



//线程池类型
class ThreadPool{
public:
    //线程池子构造
    ThreadPool();
    ~ThreadPool();

    //设置线程池的工作模式
    void setMode(PoolMpde mode);
    // start the threadpool
    void start(int initthread =4);
    //set number of thread
    void setInitialThreadNumber(int a);
    //
    Reselt submitTask(std::shared_ptr<Task> task_);
    //set task_max
    void setMaxTask(int a);


    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) =delete;
    // set the max number of thread number
    void setMaxThread(int a);
private:
    void threadHandler(int thread_id_);

    //检查pool运行状态
    bool could_change()const;
    // set 

private:
    //std::vector<std::unique_ptr<Thread>>  threads_; //list
    std::unordered_map<int,std::unique_ptr<Thread>> threads_;



    int initThreadSize_;
    int maxNumberOfThread;


    std::queue<std::shared_ptr<Task>> taskQue_;
    //std::queue<Task*> 
    std::atomic_int  TaskSize_;
    int Task_Max_;


    //mutex
    std::mutex taskQueMtx_; //保证任务队列的线程安全
    std::condition_variable TaskListNotFull_;
    std::condition_variable TaskListNotEmpty_;

    PoolMpde poolMode_; //当前线程池的工作模式
    std::atomic_bool isPoolrunning; //池子是否跑起来
    std::atomic_int   idleThreadSize; // free thread 
    std::atomic_int   TotalThreadNumber; //total number of thread



    // 
    std::condition_variable exitCond;
};


#endif