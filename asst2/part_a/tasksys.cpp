#include "tasksys.h"
#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    numthreads=num_threads;
    tpool = new std::thread[num_threads];
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {
    //delete tpool;
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //



    std::atomic<int> thr;
    thr = 0;
    for (int i = 0; i < numthreads; ++i) {
        tpool[i] = std::thread([&](){
            while (true) {
                int cur_task_index = thr++;
                if (cur_task_index >= num_total_tasks) {
                    return;
                }
                runnable->runTask(cur_task_index, num_total_tasks);
            }
        });
    }
    for (int i = 0; i < numthreads; ++i) {
        tpool[i].join();
    }
    
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    gmutex.lock();
    numtask=-1;
    done=0;
    alldone=0;
    killsig=0;
    numthreads=num_threads;
    gmutex.unlock();

    tpool = new std::thread[num_threads];

    for (int i=0;i<numthreads;i++){
        tpool[i] = std::thread([&](){
                    
                while (true){
                    gmutex.lock();
                    
                    if (killsig>0) {gmutex.unlock();killsig++;break;}
                    if (numtask==-1) {gmutex.unlock();continue;}

                    int c=curtask++;
                    if (c >=numtask){
                        alldone++;
                        gmutex.unlock();
                        continue;
                    }
gmutex.unlock();
                    runnabl->runTask(c, numtask);

                    done++;
                    
                    
                   
                    

                    

                }

            
            
        });
    }  
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    gmutex.lock();
    killsig++;
    gmutex.unlock();

   while (true){
        gmutex.lock();
        if ((int)killsig>numthreads){
            for (int i=0;i<numthreads;i++){
                tpool[i].join();
            }
            gmutex.unlock();
            break;
        }
        gmutex.unlock();
   }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {




    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //



        gmutex.lock();
        runnabl=runnable;
        numtask=num_total_tasks;
        done=0;
        alldone=0;
        curtask=0;
        gmutex.unlock();


    

    
    while (true){

        gmutex.lock();
        if (numtask!=-1 && done==numtask ){

            numtask=-2;
            done=-1;
            curtask=0;
            gmutex.unlock();

            while (true){
                gmutex.lock();
                if (alldone<numthreads){
                    gmutex.unlock();
                    continue;
                }
                break;
            }
            gmutex.unlock();
            break; 
        }
        gmutex.unlock();
    }
    
    return; 

}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}


TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    
    numtask=-1;
    curtask=0;
    done=0;
    killsig=0;
    numthreads=num_threads;


    tpool = new std::thread[num_threads];

    for (int i=0;i<numthreads;i++){
        tpool[i] = std::thread([&](){
                    
                while (true){
                    
                    if (killsig) {break;}
                    
                    gmutex.lock();

                    int c=curtask;
                    if (c<numtask) curtask++;

                    gmutex.unlock();

                    if (c < numtask){
                        runnabl->runTask(c, numtask);
                        gmutex.lock();
                        done++;
                        if (done==numtask){
                            gmutex.unlock();
                            //ensures main thread has gone into sleep and release lock
                            dmutex.lock();
                            dmutex.unlock();
                            donecv.notify_all();
                        }
                        else gmutex.unlock();
                    }

                    else {
                        std::unique_lock<std::mutex> lockt(domutex);
                        cv.wait(lockt);
                        lockt.unlock();
                    }

                }
        });
    }  


}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    gmutex.lock();

    killsig++;

    cv.notify_all();
    gmutex.unlock();
        
    for (int i=0;i<numthreads;i++){
        tpool[i].join();
        }

}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
        //initialize lock first!
        std::unique_lock<std::mutex> lock(dmutex);

        gmutex.lock();
        runnabl=runnable;
        numtask=num_total_tasks;
        curtask=0;
        done=0;
        gmutex.unlock();
        
        for (int i=0;i<num_total_tasks;i++){
            cv.notify_all();
            }
        
        donecv.wait(lock);    
        lock.unlock();
        
    return; 
}  

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
