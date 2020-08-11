#include "ListenerTask.hpp"
#include <Core/Script.hpp>
#include <TScript.hpp>

ListenerTask::ListenerTask(std::string Label, const std::chrono::duration<double>& Duration, void (*Loop)()) : Label(std::move(Label)), LoopDuration(Duration), LoopFunc(Loop)
{

}

bool ListenerTask::Started() const
{
    return ListenerTask::TaskStarted;
}

bool ListenerTask::Running() const
{
    if (ListenerTask::TaskStarted)
        return (ListenerTask::Task.wait_for(std::chrono::microseconds(0)) != std::future_status::ready);
    else
        return false;
}

bool ListenerTask::Paused() const
{
    return ListenerTask::TaskPaused;
}

bool ListenerTask::Start()
{
    if (!ListenerTask::Started() && !ListenerTask::Running())
    {
        ListenerTask::Task = std::async(std::launch::async, &ListenerTask::Loop, this);
        ListenerTask::TaskStarted = true;
        return true;
    }
    return false;
}

void ListenerTask::Pause()
{
    if (ListenerTask::Running())
        ListenerTask::TaskPause = true;
}

void ListenerTask::Resume()
{
    if (ListenerTask::Running() && ListenerTask::Paused())
        ListenerTask::TaskPause = false;
}

void ListenerTask::Stop(bool Wait)
{
    if (ListenerTask::Running())
    {
        ListenerTask::TaskStop = true;
        if (Wait) this->Wait();
    }
}

void ListenerTask::Wait() const
{
    if (!ListenerTask::Running())
        return;
    DebugLog("ListenerTask [{}] > Waiting for task", this->Label);
    ListenerTask::Task.wait();
}

void ListenerTask::Reset()
{
    ListenerTask::TaskStop = false;
    ListenerTask::TaskPause = false;
    ListenerTask::TaskPaused = false;
    ListenerTask::TaskStarted = false;
    DebugLog("ListenerTask [{}] > Task states reset", this->Label);
}

void ListenerTask::Loop()
{
    DebugLog("ListenerTask [{}] > Started", this->Label);
    this->OnStart();
    while (!ListenerTask::TaskStop)
    {
        if (Terminate) break;

        if (ListenerTask::TaskPause)
        {
            ListenerTask::TaskPaused = true;
            DebugLog("ListenerTask [{}] > Pausing", this->Label);
            while (ListenerTask::TaskPause)
            {
                if (Terminate || ListenerTask::TaskStop)
                {
                    DebugLog("ListenerTask [{}] > Stopped", this->Label);
                    ListenerTask::Reset();
                    return;
                }
                DebugLog("ListenerTask [{}] > Sleeping", this->Label);
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            DebugLog("ListenerTask [{}] > Resumed", this->Label);
            continue;
        }

        if (this->LoopFunc) this->LoopFunc();
        std::this_thread::sleep_for(this->LoopDuration);
    }

    this->OnStop();
    DebugLog("ListenerTask [{}] > Stopped", this->Label);
    ListenerTask::Reset();
}

void ListenerTask::OnStart()
{

}

void ListenerTask::OnStop()
{

}

ListenerTask::~ListenerTask()
{

}
