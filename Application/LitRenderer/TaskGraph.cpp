#include "TaskGraph.h"
#include <cassert>
#include <chrono>
#include <thread>
#include <Foundation/Base/MemoryHelper.h>

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef ENABLE_PROFILING
#include <ctime>

namespace
{
    std::atomic<int> sResourceUIDGenerator = 0;
    std::atomic<int> sTaskIDGenerator = 0;
    std::chrono::time_point<std::chrono::steady_clock> sStartTimeStamp;

    enum TaskTokenType
    {
        Create,
        Execute,
        Recycle
    };

    struct TaskProfilerToken
    {
        TaskProfilerToken() = default;

        TaskTokenType	TokenType = Create;
        ThreadName		ThreadName = ThreadName::WorkerThread;
        unsigned int	ThreadIndex = 0;
        unsigned int	TaskID = 0;
        unsigned int	ResUID = 0;
        long long		BeginTimeStamp = 0;
        long long		EndTimeStamp = 0;

    };

    //mpmc_bounded_queue<TaskProfilerToken> TaskCreateInfos(4096);
    std::vector<TaskProfilerToken> TaskCreateInfos;
    std::vector<TaskProfilerToken> DiskIORunningInfos;
    std::vector<TaskProfilerToken> WorkerTaskRunningInfos[TaskScheduler::kNumWorThread];

    void OutputTokenList(std::string& outString, const std::vector<TaskProfilerToken>& list)
    {
        int index = 0;
        for (const TaskProfilerToken& token : list)
        {
            outString += "\t\t{\n";
            {
                //type
                outString += "\t\t\t\"type\":";
                switch (token.TokenType)
                {
                default:		outString += R"("Unknown",)""\n"; break;
                case Create:	outString += R"("Create",)""\n"; break;
                case Execute:	outString += R"("Execute",)""\n"; break;
                case Recycle:	outString += R"("Recycle",)""\n"; break;
                }

                //timestamp
                char buff[128];
                outString += "\t\t\t\"begin\":";
                outString += itoa((unsigned long)token.BeginTimeStamp, buff, 10);
                outString += ",\n";

                outString += "\t\t\t\"end\":";
                outString += itoa((unsigned long)token.EndTimeStamp, buff, 10);
                outString += ",\n";


                //task id
                outString += "\t\t\t\"taskid\":";
                outString += itoa(token.TaskID, buff, 10);
                outString += ",\n";

                //resourceid
                outString += "\t\t\t\"resourceid\":";
                outString += itoa(token.ResUID, buff, 10);
                outString += "\n";
            }
            outString += "\t\t}";
            if (++index != list.size())
            {
                outString += ",\n";
            }
            else
            {
                outString += "\n";
            }
        }
    }

    void OutputProfilingDatas()
    {
        std::string outputstring = "var datasource = {\n";
        outputstring += R"(	"task_creation":[)""\n";
        OutputTokenList(outputstring, TaskCreateInfos);
        outputstring += R"(	],)""\n";

        char buff[4];
        for (int i = 0; i < TaskScheduler::kNumWorThread; i += 1)
        {
            outputstring += R"(	"work_thread_)";
            outputstring += itoa(i, buff, 10);
            outputstring += "\":[\n";
            OutputTokenList(outputstring, WorkerTaskRunningInfos[i]);
            outputstring += R"(	],)""\n";
        }

        outputstring += R"(	"io_thread":[)""\n";
        OutputTokenList(outputstring, DiskIORunningInfos);
        outputstring += R"(	])""\n";
        outputstring += "}\n";

        time_t currentTime;
        time(&currentTime);
        tm* time = localtime(&currentTime);

        char fileNameBuff[256];
        sprintf(fileNameBuff, "profile_%d_%d_%d_%d_%d_%d.txt"
            , time->tm_year
            , time->tm_mon
            , time->tm_mday
            , time->tm_hour
            , time->tm_min
            , time->tm_sec);

        FILE* f = fopen(fileNameBuff, "w+");
        if (f)
        {
            fwrite(outputstring.c_str(), 1, outputstring.size(), f);

            fclose(f);
        }
    }
}
#endif

static LockedQueue<TaskGraphNode*> GlobalTaskPool;

void ReleaseGlobalTaskPool()
{
    TaskGraphNode* Task = nullptr;
    while (GlobalTaskPool.Dequeue(Task))
    {
        delete Task;
    }
}


void Task::StartSystem(uint32_t NumWorker)
{
    TaskScheduler::Instance().Start(NumWorker);
}
void Task::StopSystem()
{
    TaskScheduler::Instance().Stop();
    ReleaseGlobalTaskPool();

#ifdef ENABLE_PROFILING
    OutputProfilingDatas();
#endif
}

Task Task::Start(ThreadName Thread, std::function<Task::Route> Route)
{
    return WhenAllImpl(Thread, Route, nullptr, 0);
}

Task Task::When(ThreadName Thread, std::function<Task::Route> Route, const Task& Prerequister)
{
    TaskGraphNode* PrerequisterNode = Prerequister.mTask;
    return WhenAllImpl(Thread, Route, &PrerequisterNode, 1);
}

Task Task::WhenAll(ThreadName Thread, std::function<Task::Route> Route, const std::vector<Task>& Prerequisters)
{
    uint32_t numPrerequisters = (uint32_t)Prerequisters.size();
    if (numPrerequisters > 0)
    {

        std::vector<TaskGraphNode*> PrerequisterNodes;
        PrerequisterNodes.reserve(numPrerequisters);
        for (const Task& task : Prerequisters)
        {
            PrerequisterNodes.push_back(task.mTask);
        }
        return WhenAllImpl(Thread, Route, &(PrerequisterNodes[0]), numPrerequisters);
    }
    else
    {
        assert(Prerequisters.size() > 0);
        return WhenAllImpl(Thread, Route, nullptr, 0);
    }
}

Task Task::WhenAll(ThreadName Thread, std::function<Task::Route> Route, const Task* Prerequisters, unsigned int numPrerequisters)
{
    if (numPrerequisters > 0)
    {
        std::vector<TaskGraphNode*> PrerequisterNodes;
        PrerequisterNodes.reserve(numPrerequisters);
        for (uint32_t index = 0; index < numPrerequisters; index += 1)
        {
            PrerequisterNodes.push_back(Prerequisters[index].mTask);
        }
        return WhenAllImpl(Thread, Route, &(PrerequisterNodes[0]), numPrerequisters);
    }
    else
    {
        assert(numPrerequisters > 0);
        return WhenAllImpl(Thread, Route, nullptr, 0);
    }
}

Task Task::WhenAllImpl(ThreadName Thread, std::function<Task::Route> Route, TaskGraphNode** Prerequistes, uint32_t numPrerequisters)
{
    //In case the task node not be recycled before we create task wrapper.
    // start() will create task node with 2 reference count.
    // we need to remove the reference.
    Task Task(TaskGraphNode::StartTask(Thread, std::move(Route), Prerequistes, numPrerequisters));
    Task.mTask->Release();
    return Task;
}

Task Task::Then(ThreadName Thread, std::function<Task::Route>&& Route)
{
    if (mTask)
    {
        //In case the task node not be recycled before we create task wrapper.
        // start() will create task node with 2 reference count.
        // we need to remove the reference.
        Task Task(mTask->Then(Thread, std::move(Route)));
        Task.mTask->Release();
        return Task;
    }
    else
    {
        return Task();
    }
}

bool Task::DontCompleteUntil(Task task)
{
    if (mTask && task.mTask)
    {
        return mTask->DontCompleteUntil(task.mTask);
    }
    else
    {
        return false;
    }
}

bool Task::IsCompleted() const
{
    if (mTask)
    {
        return mTask->IsCompleted();
    }
    else
    {
        return true;
    }
}

void Task::SpinWait()
{
    if (mTask)
    {
        mTask->SpinWait();
        ReleaseRef();
    }
}

void Task::ManualRelease()
{
    ReleaseRef();
}

Task::Task(Task&& task) : mTask(task.mTask)
{
    task.mTask = nullptr;
}

Task::Task(const Task& task) : mTask(task.mTask)
{
    mTask->AddReference();
}

Task::~Task()
{
    ReleaseRef();
}

Task& Task::operator=(const Task& task)
{
    task.mTask->AddReference();
    ReleaseRef();
    mTask = task.mTask;
    return *this;
}

Task::Task(TaskGraphNode* pTask) : mTask(pTask)
{
    assert(!mTask->IsRecycled());
    mTask->AddReference();
}

void Task::ReleaseRef()
{
    if (mTask)
    {
        mTask->Release();
        mTask = nullptr;
    }
}


bool IsContained(const std::vector<TaskGraphNode*>& vec, TaskGraphNode* pTask)
{
    return std::find(vec.begin(), vec.end(), pTask) != vec.end();
}

bool TaskGraphNode::RecursiveSearchSubsequents(std::vector<TaskGraphNode*>& checkedTasks, TaskGraphNode* pTask)
{
    // avoid recursive lock
    if (IsContained(checkedTasks, this))
    {
        return true;
    }

    checkedTasks.push_back(this);

    std::lock_guard<std::mutex> guard(mSubseuquentsMutex);
    for (TaskGraphNode* pSubsquent : mSubsequents)
    {
        if (pSubsquent == pTask ||
            pSubsquent->RecursiveSearchSubsequents(checkedTasks, pTask))
        {
            return true;
        }
    }

    return false;
}

TaskGraphNode* TaskGraphNode::StartTask(ThreadName Thread, std::function<Task::Route>&& Route, TaskGraphNode** Prerequistes, uint32_t NumPrerequistes, TaskPriority Priority)
{
#ifdef ENABLE_PROFILING
    TaskProfilerToken token;
    token.ThreadName = ThreadName::WorkerThread;
    token.TokenType = TaskTokenType::Create;
    token.BeginTimeStamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - sStartTimeStamp).count();
#endif

    TaskGraphNode* Task = nullptr;
    if (GlobalTaskPool.Dequeue(Task))
    {
        Task->mThreadName = Thread;
        Task->mPriority = Priority;
        Task->mTaskRoute = std::move(Route);
        Task->mReferenceCount.store(2, std::memory_order_release);
        Task->mCompleted.store(false, std::memory_order_release);
        Task->mRecycled.store(false, std::memory_order_release);
        Task->mAntecedentDependencyCount.store(1, std::memory_order_release);
        Task->mDontCompleteUntil.store(false, std::memory_order_release);
        Task->mDontCompleteUntilEmptyTask = nullptr;
    }
    else
    {
        Task = new TaskGraphNode(Thread, std::move(Route), Priority);
#ifdef ENABLE_PROFILING
        Task->mResUID = sResourceUIDGenerator.fetch_add(1, std::memory_order_acquire);
        token.ResUID = newTask->mResUID;
#endif
    }

#ifdef ENABLE_PROFILING
    Task->mTaskID = sTaskIDGenerator.fetch_add(1, std::memory_order_acquire);
    token.TaskID = newTask->mTaskID;
#endif

    if (Prerequistes && NumPrerequistes > 0)
    {
        for (unsigned int i = 0; i < NumPrerequistes; i += 1)
        {
            Prerequistes[i]->AddSubsequent(Task);
        }
    }

#ifdef ENABLE_PROFILING
    token.EndTimeStamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - sStartTimeStamp).count();
    TaskCreateInfos.push_back(token);
#endif
    Task->ScheduleMe();
    return Task;
}

void TaskGraphNode::FreeAndRecycleTask(TaskGraphNode* Task)
{
    assert
    (
        !Task->mRecycled.load(std::memory_order_acquire) &&
        Task->mReferenceCount.load(std::memory_order_acquire) == 0
    );

    const bool bRecycledOldValue = Task->mRecycled.exchange(true, std::memory_order_release);
    const bool bIsNotRecycledYet = !bRecycledOldValue;
    if (bIsNotRecycledYet)
    {
        assert(Task->IsCompleted());
        GlobalTaskPool.Enqueue(Task);
    }
}

TaskGraphNode* TaskGraphNode::Then(ThreadName Thread, std::function<Task::Route>&& Route, TaskPriority Priority)
{
    TaskGraphNode* Task = this;
    return StartTask(Thread, std::move(Route), &Task, 1, Priority);
}

bool TaskGraphNode::DontCompleteUntil(TaskGraphNode* TaskNode)
{
    // form a loop-graph, its Dangerous !
    if (mDontCompleteUntilEmptyTask == nullptr)
    {
        // Add Reference to allow release twice
        //so that We can use TryRelease in ThreadFunc.
        AddReference();
        mDontCompleteUntilEmptyTask = Then(ThreadName::Worker, [](Task& thisTask) {}, TaskPriority::High);

        /* We are creating a temp empty Task for notifying this one.
           so we need the new task be the antecedent of this one.
           and we need stop subsequents of this one start before the new
           created one being notified. in case of that, we should join
           all subsequents into new created one, let them being notified
           twice to start the task.

            we will adjust the graph from this, to this one:

                                              ancestor
                                                  +
                                                / | \
                                               / /  |\
                       this                   / this| \
                        +                     |  +---. \
                       /|\                    | / \ | \ |
                      / | \                   |/   \|  \|
                     s0 s1 s2                 s0   s1  s2

         */
        {
            // Prevent calling AddSubsequents outside.
            std::lock_guard<std::mutex> guard(mSubseuquentsMutex);
            for (TaskGraphNode* Subsequent : mSubsequents)
            {
                if (Subsequent != mDontCompleteUntilEmptyTask)
                {
                    mDontCompleteUntilEmptyTask->AddSubsequent(Subsequent);
                }
            }
        }

        // this antecedent task is create inside
        // we could ignore the second reference guard in case we send to Task Wrapper.
        // no need to: this->mAntecedentDependencyCount + 1
        mDontCompleteUntil.store(true, std::memory_order_release);
        mDontCompleteUntilEmptyTask->mSubsequents.push_back(this);
        mDontCompleteUntilEmptyTask->Release();

        return TaskNode->AddSubsequent(mDontCompleteUntilEmptyTask);
    }

    return false;
}

void TaskGraphNode::SpinWait()
{
    while (!IsCompleted());
}

void TaskGraphNode::Execute()
{
    {
        //no need to keep the task the same,
        // we just need to send the request to its implement node.
        Task WrapperTask(this);
        mTaskRoute(WrapperTask);
    }

    if (!mDontCompleteUntil.load(std::memory_order_acquire))
    {
        mCompleted.store(true, std::memory_order_release);
    }
    NotifySubsequents();
    Release();
}

void TaskGraphNode::AddReference()
{
    mReferenceCount.fetch_add(1);
}

void TaskGraphNode::Release()
{
    if (mReferenceCount.fetch_sub(1) == 1)
    {
        TaskGraphNode::FreeAndRecycleTask(this);
    }
}

bool TaskGraphNode::IsCompleted() const
{
    return mCompleted.load(std::memory_order_acquire);
}

bool TaskGraphNode::IsRecycled() const
{
    return mRecycled.load(std::memory_order_acquire);
}

bool TaskGraphNode::IsAvailable() const
{
    return mAntecedentDependencyCount.load(std::memory_order_acquire) == 0;
}

TaskGraphNode::TaskGraphNode(ThreadName Thread, std::function<Task::Route>&& Route, TaskPriority Priority)
    : mThreadName(Thread)
    , mTaskRoute(Route)
    , mPriority(Priority)
{

}

void TaskGraphNode::NotifySubsequents()
{
    std::vector<TaskGraphNode*> LocalSubsequents;
    {
        std::lock_guard<std::mutex> guard(mSubseuquentsMutex);
        LocalSubsequents.swap(mSubsequents);
    }

    for (TaskGraphNode* subsequent : LocalSubsequents)
    {
        assert
        (
            !subsequent->IsCompleted()
            && !subsequent->IsRecycled()
            && (!subsequent->IsAvailable()
                || subsequent->mDontCompleteUntil.load(std::memory_order_acquire))
        );

        subsequent->ScheduleMe();
    }
}

bool TaskGraphNode::AddSubsequent(TaskGraphNode* pNextTask)
{
    std::vector<TaskGraphNode*> CheckedTasks;
    if (this != pNextTask && !pNextTask->RecursiveSearchSubsequents(CheckedTasks, this))
    {
        if (!IsCompleted())
        {
            std::lock_guard<std::mutex> guard(mSubseuquentsMutex);
            if (!IsCompleted())
            {
                pNextTask->mAntecedentDependencyCount.fetch_add(1, std::memory_order_acq_rel);
                mSubsequents.push_back(pNextTask);

                //the function will only be called in constructor and dont-complete-until.
                // and when we calling dont-complete-until, the task is already created
                // so we wont miss any newly-add subsequent task.
                //if(mDontCompleteUntilEmptyTask)
                return true;
            }
        }
    }
    return false;
}

void TaskGraphNode::ScheduleMe()
{
    if (mDontCompleteUntil.load(std::memory_order_acquire))
    {
        mCompleted.store(true, std::memory_order_release);
        Release();
    }
    else
    {
        assert(!IsCompleted() && !IsRecycled() && !IsAvailable());

        if (mAntecedentDependencyCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            TaskScheduler::Instance().ScheduleTask(this);
        }
    }
}


TaskScheduler& TaskScheduler::Instance()
{
    static TaskScheduler instance;
    return instance;
}

void TaskScheduler::ScheduleTask(TaskGraphNode* task)
{
    if (task->DesireExecutionOn() == ThreadName::DiskIO)
    {
        DiskIOThreadTaskQueue.Enqueue(task);
    }
    else
    {
        WorkerThreadTaskQueue.Enqueue(task);
    }
}

void TaskScheduler::Start(uint32_t InNumWorkers)
{
#ifdef	ENABLE_PROFILING
    sStartTimeStamp = std::chrono::steady_clock::now();
#endif
    NumWorkers = InNumWorkers;
    WorkerThreads = new std::thread[NumWorkers];
    for (uint32_t Index = 0; Index < NumWorkers; Index += 1)
    {
        WorkerThreads[Index] = std::thread(&TaskScheduler::TaskThreadRoute, this, &WorkerThreadTaskQueue, ThreadName::Worker, Index);

#ifdef WIN32
        SetThreadAffinityMask(WorkerThreads[Index].native_handle(), (DWORD_PTR)(0x4 << (Index * 2)));
#endif
    }

    DiskIO = std::thread(&TaskScheduler::TaskThreadRoute, this, &DiskIOThreadTaskQueue, ThreadName::DiskIO, 0);

#ifdef WIN32
    SetThreadAffinityMask(DiskIO.native_handle(), (DWORD_PTR)0x2);
#endif
}

void TaskScheduler::Stop()
{
    SchedulerKeepRunning.store(false, std::memory_order_release);
    Join();
}

void TaskScheduler::TaskThreadRoute(TaskQueue* TaskQueue, ThreadName ThreadName, uint32_t ThreadIndex)
{
    TaskGraphNode* TaskNode = nullptr;
    bool bHasNewTask = TaskQueue->Dequeue(TaskNode);
    while (SchedulerKeepRunning.load(std::memory_order_acquire) || bHasNewTask)
    {
        if (bHasNewTask)
        {
#ifdef ENABLE_PROFILING
            TaskProfilerToken token;
            token.ThreadName = threadName;
            token.ThreadIndex = index;
            token.TokenType = TaskTokenType::Execute;
            token.BeginTimeStamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - sStartTimeStamp).count();
            token.TaskID = task->mTaskID;
            token.ResUID = task->mResUID;
#endif
            TaskNode->Execute();

#ifdef ENABLE_PROFILING
            token.EndTimeStamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - sStartTimeStamp).count();
            if (threadName == ThreadName::DiskIOThread)
            {
                DiskIORunningInfos.push_back(token);
            }
            else
            {
                WorkerTaskRunningInfos[index].push_back(token);
            }
#endif

        }

        bHasNewTask = TaskQueue->Dequeue(TaskNode);
        if (!bHasNewTask)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(50ms);
        }
    }
}

void TaskScheduler::Join()
{
    DiskIOThreadTaskQueue.Quit();
    WorkerThreadTaskQueue.Quit();
    for (uint32_t Index = 0; Index < NumWorkers; Index++)
    {
        WorkerThreads[Index].join();
    }
    DiskIO.join();

}

bool TaskScheduler::TaskQueue::Enqueue(TaskGraphNode* Task)
{
    TaskPriority TaskPriority = Task->DesireExecutionPriority();
    if (TaskPriority == TaskPriority::High)
    {
        QueueH.Enqueue(Task);
    }

    if (TaskPriority == TaskPriority::Normal)
    {
        QueueN.Enqueue(Task);
    }

    if (TaskPriority == TaskPriority::Low)
    {
        std::lock_guard<std::mutex> lk(QueueLMutex);
        QueueL.push_back(Task);
    }

    NumTasks.fetch_add(1, std::memory_order_release);
    if (NumWaitingThreads.load(std::memory_order_acquire))
    {
        std::unique_lock<std::mutex> lk(QueueMutex);
        QueueCV.notify_one();
    }
    return true;
}

void TaskScheduler::TaskQueue::Quit()
{
    bSpinWaiting.store(false, std::memory_order_release);
    std::unique_lock<std::mutex> lk(QueueMutex);
    QueueCV.notify_all();
}

bool TaskScheduler::TaskQueue::Dequeue(TaskGraphNode*& pTask)
{
    while (bSpinWaiting.load(std::memory_order_acquire))
    {
        NumWaitingThreads.fetch_add(1, std::memory_order_release);
        std::unique_lock<std::mutex> lk(QueueMutex);
        QueueCV.wait(lk, [&] {
            return !bSpinWaiting.load(std::memory_order_acquire)
                || NumTasks.load(std::memory_order_acquire) != 0;
            });
        NumWaitingThreads.fetch_sub(1, std::memory_order_release);

        if (QueueH.Dequeue(pTask))
        {
            NumTasks.fetch_sub(1, std::memory_order_release);
            return true;
        }
        else if (QueueN.Dequeue(pTask))
        {
            NumTasks.fetch_sub(1, std::memory_order_release);
            return true;
        }
        else
        {
            std::lock_guard<std::mutex> lk(QueueLMutex);
            if (QueueL.size() > 0)
            {
                pTask = QueueL.back();
                QueueL.pop_back();
                NumTasks.fetch_sub(1, std::memory_order_release);
                return true;
            }
        }
    }
    return false;
}
