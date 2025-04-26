#include"Threadpool.h"
#include <thread>
#include <memory>
#include <functional>
#include <iostream>
#include <chrono>

const int TASK_MAX_ =  INT32_MAX;
const int THREAD_MAX =  8;
const int THREAD_MAX_WAIT_SECRONDS = 10;

/// ///// /////////////////////////////
///thread
///////////////

/// ///// /////////////////////////////
///thread pool
///////////////
ThreadPool::ThreadPool()
    :initThreadSize_(4)
    ,TaskSize_(0)
    ,Task_Max_(TASK_MAX_)
    ,poolMode_(PoolMpde::MODE_FIXED)
    ,isPoolrunning(false)
    ,idleThreadSize(0)
    ,maxNumberOfThread(THREAD_MAX)
    ,TotalThreadNumber(4)
    {}
ThreadPool::~ThreadPool(){
    isPoolrunning = false;
    // waitfor all thread return 

    std::unique_lock<std::mutex> unilock(taskQueMtx_);
    TaskListNotEmpty_.notify_all();
    exitCond.wait(unilock,[&]()->bool{return threads_.size()==0;});
}


//设置线程池的工作模式
void ThreadPool::setMode(PoolMpde mode)  {
    if (could_change()) return ;
    poolMode_ = mode;
    
}

//set task_max
void ThreadPool::setMaxTask(int a){    
    if (could_change()) return ;
    Task_Max_ =a;
}
// set thread max
void ThreadPool::setMaxThread(int a){    
    if (could_change()) return ;
    if (poolMode_  == PoolMpde::MODE_CACHED) maxNumberOfThread =a;
}
//
Reselt ThreadPool::submitTask(std::shared_ptr<Task> task_){
    //获取锁
    std::unique_lock<std::mutex> ulm(taskQueMtx_);
    /*
    while(ThreadPool::taskQue_.size() == Task_Max_)
    {
    //线程通信 等待
        TaskListNotFull_.wait(ulm);
    //用户调用之后阻塞了，返回提交失败
    }
    */
   /*
   	while (!__p())
	  wait(__lock);
      }
   */
  //   TaskListNotFull_.wait(ulm,[&]()->bool { return taskQue_.size() < Task_Max_;}); //什么时候退出
  // wait waitfor
    if (!TaskListNotFull_.wait_for(ulm,std::chrono::seconds(1),[&]()->bool { return taskQue_.size() < Task_Max_;})){

        std::cerr <<"task fall "<<std::endl;
        return Reselt(task_); //wrong
    } //什么时候退出

    //如果有空余，放在任务队列中
    taskQue_.emplace(task_);
    TaskSize_++;
    //新放了，以及不满,在note
    TaskListNotEmpty_.notify_all();
    if(poolMode_ == PoolMpde::MODE_CACHED 
    && TaskSize_ >idleThreadSize  // the number of free thread is lower than tasksize
    &&  TotalThreadNumber<maxNumberOfThread){ //
		std::cout << ">>> create new thread..." << std::endl;
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadHandler,this,std::placeholders::_1)); //在线程池里添加线程
        //threads_.emplace_back(std::move(ptr)); //因为make_unique
        int threadId  = ptr->getId();
        threads_[threadId] = std::move(ptr);
        threads_[threadId]->start();
        TotalThreadNumber ++;
        idleThreadSize++;
    } 
    return Reselt(task_);
}
// start the threadpool
void ThreadPool::start(int initthread){
    isPoolrunning = true;
    initThreadSize_ = initthread;
    
    //create thread 
    for (int i =0;i<initThreadSize_;i++){
        //在线程池子里添加对象
        //每个对象，因为刚开始传入的threadHandler 我们也不确定是个什么类型
        // 所以考虑用智能指针去接一下，出局部作用域析构
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadHandler,this,std::placeholders::_1)); //在线程池里添加线程
        //threads_.emplace_back(std::move(ptr)); //因为make_unique
        int threadId  = ptr->getId();
        threads_[threadId] = std::move(ptr);

    }
    // start all thread
    for (int i =0;i<initThreadSize_;i++){
        threads_[i]->start();
        idleThreadSize++;
    } 
}

bool ThreadPool::could_change()const{
    return isPoolrunning;
}
//创建线程对象的时候，希望线程可以那倒线程对象
void ThreadPool::threadHandler(int thread_id_){

    //每一个线程运行的时候都会进入这么一个时间函数
    //等所有任务必须执行完，线程池才可以回收所有资源
    auto lastTime  = std::chrono::high_resolution_clock().now();
    for(;;){
        std::shared_ptr<Task> task;
        {
            //获取锁
            std::unique_lock<std::mutex> unl(ThreadPool::taskQueMtx_);

            //cached 模式下，有可能已经创建了很多线程但是空闲时间超过60s，应该把多余的线程结束掉
            //结束回收掉（超过initTheadSize 数量的线程要进行回收掉）
            //当前时间 - 上次时间 》60
            /*
            if (poolMode_ == PoolMpde::MODE_CACHED){
                while(taskQue_.size()==0){ //if there is no task wait there
                    if(std::cv_status::timeout  == TaskListNotEmpty_.wait_for(unl,std::chrono::seconds(1))){
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur =  std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                    
                        if(dur.count() >=THREAD_MAX_WAIT_SECRONDS
                            && TotalThreadNumber>initThreadSize_){
                                threads_.erase(thread_id_); //
                                idleThreadSize--;
                                TotalThreadNumber--;
                                std::cout<<"threadid:"<<std::this_thread::get_id() << "exist" <<std::endl;
                                return ;
                            }
                    }
                }
            }
            else{
                TaskListNotEmpty_.wait(unl,[&]()->bool{return taskQue_.size()>0;});
                if (!isPoolrunning){}
            }*/
            //wait for if the  taskque is empty
            //锁+双重判断
            while (taskQue_.size()==0){

                if (!isPoolrunning){
                    threads_.erase(thread_id_);
                    exitCond.notify_all();
                    std::cout<< "ID"<<thread_id_<< "is over" <<std::endl;
                    return ;
                }


                if (poolMode_ == PoolMpde::MODE_CACHED){
                    if(std::cv_status::timeout  == TaskListNotEmpty_.wait_for(unl,std::chrono::seconds(1))){
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur =  std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                    
                        if(dur.count() >=THREAD_MAX_WAIT_SECRONDS
                            && TotalThreadNumber>initThreadSize_){
                                threads_.erase(thread_id_); //
                                idleThreadSize--;
                                TotalThreadNumber--;
                                std::cout<<"threadid:"<<std::this_thread::get_id() << "exist" <<std::endl;
                                return ;
                            }
                    }
                }
                else{
                    //
                    TaskListNotEmpty_.wait(unl);
                }
            }



            //等待队列里有东西 :
            idleThreadSize--;
            //从任务队列取出一个队列

            task = taskQue_.front(); //导出一个 任务
            taskQue_.pop(); //删除
            TaskSize_--;

            //告诉其他锁，可以继续消费
            if(taskQue_.size()>0) TaskListNotEmpty_.notify_all(); //通知等待的，可以继续唤醒

            //锁释放掉，可以继续生产
            TaskListNotFull_.notify_all();
        }

        // 
        if(task!= nullptr)task->exec();
        //task has already finished, the 
        lastTime  = std::chrono::high_resolution_clock().now();
        idleThreadSize++;
    }
    threads_.erase(thread_id_);
    exitCond.notify_all();
    std::cout<< "ID"<<thread_id_<< "is over" <<std::endl;
    //threads_.erase(thread_id_);
                
}

//////// Thread //////

Thread::~Thread(){}

Thread::Thread(ThreadFunc func1)
    :func_(func1)
    ,threadId_(generateId_++) {}


 
void Thread::start(){
    int id = this->getId();
    std::thread t(func_,id); //c++ 11 线程函数
    t.detach(); //如果出了start 那么声明周期结束了  
}

int Thread::getId()const{
    return threadId_;
}

//////// result /////

Reselt::Reselt(std::shared_ptr<Task> task,bool isVal):
Task_(task),
isValid_(isVal)
{
    Task_->setResult(this);
};


std::any Reselt::get(){
    if (!isValid_){
        return "";
    }
    sem_.wait(); //task没执行完 阻塞住
    return std::move(any_);
}


void Reselt::setValue(std::any any_){
    //存储task返回值
    this->any_ = std::move(any_);
    sem_.post();
}



////////Task /////


void Task::exec(){
    if (res_!=nullptr){
        
        res_->setValue(run());
    }


}

void Task::setResult(Reselt* result){
    res_ = result;
}

