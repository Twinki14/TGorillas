#ifndef TLIB_LOOPTASK_HPP_INCLUDED
#define TLIB_LOOPTASK_HPP_INCLUDED

#include <future>
#include <atomic>

class LoopTask
{
public:
    LoopTask() = default;
    LoopTask(std::string Label, const std::chrono::duration<double>& Duration, void (*Loop)());

    bool Started() const;
    bool Running() const;
    bool Paused() const;

    bool Start();
    void Pause();
    void Resume();
    void Stop(bool Wait = false);
    void Wait() const;

    virtual ~LoopTask();
private:
    std::string Label;
    void (*LoopFunc)() = nullptr;
    std::chrono::duration<double> LoopDuration = std::chrono::milliseconds(10);

    std::future<void> Task;
    std::atomic<bool> TaskStarted = false;
    std::atomic<bool> TaskStop = false;
    std::atomic<bool> TaskPause = false;
    std::atomic<bool> TaskPaused = false;
    void Reset();

    void Loop();
    virtual void OnStart();
    virtual void OnStop();

};

#endif // TLIB_LOOPTASK_HPP_INCLUDED