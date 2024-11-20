#include "tasksys.h"
#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <bits/stdc++.h>
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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
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
    tid=-1;
    numthreads=num_threads;


    tpool = new std::thread[num_threads];

    for (int i=0;i<numthreads;i++){
        tpool[i] = std::thread([&](){
                    
                while (true){
                    
                    if (killsig) {break;}
                      
                    gmutex.lock();
                    if (!qwait.empty()){
                        TaskID chk=qwait.front();
                        qwait.pop();
                        std::vector<TaskID> depen=dep[chk];
                        bool tmp=true;
                        for (TaskID i:depen){
                            if (fin[i]) {qwait.push(chk);tmp=false;break;}
                        }
                        if (tmp) {
                            for (int i=0;i<count[chk];i++){
                                q.push({chk,i});
                            }
                        }
                        cv.notify_all();
                        gmutex.unlock();
                    }
                    else gmutex.unlock();

                    gmutex.lock();
                    if (!q.empty()){
                        auto m=q.front();
                        TaskID tsk=m.first;
                        int c=m.second;
                        q.pop();
                        

                        gmutex.unlock();
                        rnabl[tsk]->runTask(c, count[tsk]);
                        gmutex.lock();
                        done++;
                        fin[tsk]--;
                        
                        gmutex.unlock();
                    }

                    else if (qwait.empty()){
                        gmutex.unlock();
                            //ensures main thread has gone into sleep and release lock
                            dmutex.lock();
                            dmutex.unlock();
                            donecv.notify_all();
                        std::unique_lock<std::mutex> lockt(domutex);
                        if (killsig) break;
                        cv.wait(lockt);
                        cv.notify_all();
                        lockt.unlock();
                        
                    }
                    else gmutex.unlock();

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
        cv.notify_all();
        tpool[i].join();
        }
        
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    runAsyncWithDeps(runnable, num_total_tasks,std::vector<TaskID> ());
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //
    int t;

    gmutex.lock();
    tid++;
    t=tid;
    dep.push_back(deps);
    fin.push_back(num_total_tasks);
    count.push_back(num_total_tasks);
    rnabl.push_back(runnable);
    bool tmp=true;
    for (auto i:deps){
        if (fin[i]) {qwait.push(tid);tmp=false;break;}
    }
    if (tmp)
        for (int i=0;i<num_total_tasks;i++){
                                q.push({tid,i});
                            }
    cv.notify_all();
    gmutex.unlock();

    

    return t;
}

void TaskSystemParallelThreadPoolSleeping::sync() {
    //run -n 8 simple_test_async
    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    std::unique_lock<std::mutex> lock(dmutex);
    donecv.wait(lock,[&](){for (auto i:fin){if (i!=0) return false;} return true;});   


     
    lock.unlock();
    return;
}
