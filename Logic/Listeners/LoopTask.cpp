#include "LoopTask.hpp"
#include <Core/Script.hpp>
#include <TScript.hpp>

LoopTask::LoopTask(std::string Label, const std::chrono::duration<double>& Duration, void (*Loop)()) : Label(std::move(Label)), LoopDuration(Duration), LoopFunc(Loop)
{

}

bool LoopTask::Started() const
{
    return LoopTask::TaskStarted;
}

bool LoopTask::Running() const
{
    if (LoopTask::TaskStarted)
        return (LoopTask::Task.wait_for(std::chrono::microseconds(0)) != std::future_status::ready);
    else
        return false;
}

bool LoopTask::Paused() const
{
    return LoopTask::TaskPaused;
}

bool LoopTask::Start()
{
    if (!LoopTask::Started() && !LoopTask::Running())
    {
        LoopTask::Task = std::async(std::launch::async, &LoopTask::Loop, this);
        LoopTask::TaskStarted = true;
        return true;
    }
    return false;
}

void LoopTask::Pause()
{
    if (LoopTask::Running())
        LoopTask::TaskPause = true;
}

void LoopTask::Resume()
{
    if (LoopTask::Running() && LoopTask::Paused())
        LoopTask::TaskPause = false;
}

void LoopTask::Stop(bool Wait)
{
    if (LoopTask::Running())
    {
        LoopTask::TaskStop = true;
        if (Wait) this->Wait();
    }
}

void LoopTask::Wait() const
{
    if (!LoopTask::Running())
        return;
    DebugLog("LoopTask [{}] > Waiting for task", this->Label);
    LoopTask::Task.wait();
}

void LoopTask::Reset()
{
    LoopTask::TaskStop = false;
    LoopTask::TaskPause = false;
    LoopTask::TaskPaused = false;
    LoopTask::TaskStarted = false;
    DebugLog("LoopTask [{}] > Task states reset", this->Label);
}

void LoopTask::Loop()
{
    DebugLog("LoopTask [{}] > Started", this->Label);
    this->OnStart();
    while (!LoopTask::TaskStop)
    {
        if (Terminate) break;

        if (LoopTask::TaskPause)
        {
            LoopTask::TaskPaused = true;
            DebugLog("LoopTask [{}] > Pausing", this->Label);
            while (LoopTask::TaskPause)
            {
                if (Terminate || LoopTask::TaskStop)
                {
                    DebugLog("LoopTask [{}] > Stopped", this->Label);
                    LoopTask::Reset();
                    return;
                }
                DebugLog("LoopTask [{}] > Sleeping", this->Label);
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            DebugLog("LoopTask [{}] > Resumed", this->Label);
            continue;
        }

        if (this->LoopFunc) this->LoopFunc();
        std::this_thread::sleep_for(this->LoopDuration);
    }

    this->OnStop();
    DebugLog("LoopTask [{}] > Stopped", this->Label);
    LoopTask::Reset();
}

void LoopTask::OnStart()
{

}

void LoopTask::OnStop()
{

}

LoopTask::~LoopTask()
{

}
