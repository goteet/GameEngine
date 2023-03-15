#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <queue>
#include <memory>

//#define ENABLE_PROFILING

enum class TaskPriority { High, Normal, Low };

enum class ThreadName
{
    Worker,
    DiskIO,

#ifdef ENABLE_PROFILING
    WorkerThread_Debug0,
    WorkerThread_Debug1,
    WorkerThread_Debug2,
    WorkerThread_Debug3,
    WorkerThread_Debug_Empty,
#endif
};

class TaskGraphNode;
class TaskScheduler;

struct Task
{
    typedef void(Route)(Task&);

    static void StartSystem(uint32_t NumWorker = 4);
    static void StopSystem();
    static Task Start(ThreadName Thread, std::function<Task::Route> Route);
    static Task When(ThreadName Thread, std::function<Task::Route> Route, const Task& Prerequisters);
    static Task WhenAll(ThreadName Thread, std::function<Task::Route> Route, const std::vector<Task>& Prerequisters);
    static Task WhenAll(ThreadName Thread, std::function<Task::Route> Route, const Task* Prerequisters, unsigned int numPrerequisters);
    Task Then(ThreadName Thread, std::function<Task::Route>&& route);
    bool DontCompleteUntil(Task task);
    bool IsCompleted() const;
    void SpinWait();
    void ManualRelease();
    Task() = default;
    Task(Task&& task);
    Task(const Task& task);
    ~Task();
    Task& operator=(const Task& task);
    friend bool operator==(const Task& lhs, const Task& rhs) { return lhs.mTask == rhs.mTask; }

private:
    friend class TaskGraphNode;
    static Task WhenAllImpl(ThreadName Thread, std::function<Task::Route> Route, TaskGraphNode** Prerequisters, uint32_t numPrerequisters);
    Task(TaskGraphNode* pTask);
    void ReleaseRef();

    TaskGraphNode* mTask = nullptr;
};


template<typename T>
class LockedQueue
{
    std::mutex mMutex;
    std::queue<T> mQueue;
public:
    bool Dequeue(T& value)
    {
        const std::lock_guard<std::mutex> lock{ mMutex };
        if (mQueue.size())
        {
            value = mQueue.front();
            mQueue.pop();
            return true;
        }
        return false;
    }
    void Enqueue(const T& value)
    {
        const std::lock_guard<std::mutex> lock{ mMutex };
        mQueue.push(value);
    }
};

class TaskGraphNode
{
    friend struct Task;
    friend class TaskScheduler;

    static TaskGraphNode* StartTask(ThreadName Thread, std::function<Task::Route>&& Route, TaskGraphNode** Prerequistes = nullptr, uint32_t NumPrerequistes = 0, TaskPriority Priority = TaskPriority::Normal);
    static void FreeAndRecycleTask(TaskGraphNode* pTask);
    TaskGraphNode* Then(ThreadName Thread, std::function<Task::Route>&& Route, TaskPriority Priority = TaskPriority::Normal);
    bool DontCompleteUntil(TaskGraphNode* task);
    void SpinWait();
    void AddReference();
    void Release();
    bool IsCompleted() const;
    bool IsRecycled() const;
    bool IsAvailable() const;

private: //internal-use-purpose.
    void Execute();
    ThreadName DesireExecutionOn() const { return mThreadName; }
    TaskPriority DesireExecutionPriority() const { return mPriority; }
    TaskGraphNode(ThreadName Thread, std::function<Task::Route>&& Route, TaskPriority Priority);
    void NotifySubsequents();

    /**
    * function didnt detected the state of pNextTask
    * make sure the task is not completed.
    */
    bool AddSubsequent(TaskGraphNode* pNextTask);

    void ScheduleMe();

    /**
    * due to the existance of DonCompleteUntilEmptyTask,
    * we need to check the recursive references.
    */
    bool RecursiveSearchSubsequents(std::vector<TaskGraphNode*>&, TaskGraphNode* pTask);

    ThreadName mThreadName = ThreadName::Worker;
    TaskPriority mPriority = TaskPriority::Normal;
    std::atomic<bool> mCompleted = false;
    std::atomic<bool> mRecycled = false;
    std::atomic<bool> mDontCompleteUntil = false;
    std::atomic<int> mAntecedentDependencyCount = 1;
    std::atomic<int> mReferenceCount = 2;
    std::function<Task::Route> mTaskRoute;
    TaskGraphNode* mDontCompleteUntilEmptyTask = nullptr;

    /**
    * Subsequents will be visit in 4 difference places.
    *	* Subsequents will be notified when task completed. afterward Subsequents shouldnt be accessed.
    *	* Anytime calling the AddSubsequents
    *       do not Subsequents after Antecedent Task completed.
    *		otherwise antecedent cant notified the task with depedency count decreased.
    *	* DontCompleteUntil() - AddSubsequents.
    *	* AddSubsequents, detect the resursive refs.
    */
    std::mutex mSubseuquentsMutex;
    std::vector<TaskGraphNode*> mSubsequents;

#ifdef ENABLE_PROFILING
private:
    unsigned int mTaskID = 0;
    unsigned int mResUID = 0;
#endif
};

class TaskScheduler
{
public:
    static TaskScheduler& Instance();

private://internal use.
    friend class TaskGraphNode;
    friend struct Task;
    void ScheduleTask(TaskGraphNode* task);
    void Start(uint32_t NumWorkers);
    void Stop();

private:
    struct TaskQueue
    {
        TaskQueue() = default;
        bool Enqueue(TaskGraphNode* pTask);
        bool Dequeue(TaskGraphNode*& pTask);
        void Quit();

    private:
        LockedQueue<TaskGraphNode*> QueueH;
        LockedQueue<TaskGraphNode*> QueueN;
        std::vector<TaskGraphNode*> QueueL;

        std::atomic<bool> bSpinWaiting = true;
        std::atomic<int> NumTasks = 0;
        std::atomic<int> NumWaitingThreads = 0;
        std::condition_variable QueueCV;
        std::mutex QueueMutex;
        std::mutex QueueLMutex;
    };
    std::atomic_bool SchedulerKeepRunning = true;
    uint32_t NumWorkers;
    std::thread DiskIO;
    std::thread* WorkerThreads;

    TaskQueue DiskIOThreadTaskQueue;
    TaskQueue WorkerThreadTaskQueue;

    void TaskThreadRoute(TaskQueue* TaskQueue, ThreadName ThreadName, uint32_t ThreadIndex);
    void Join();
};

