#pragma once

#include <thread>
#include <mutex>
#include <memory>
#include <asio.hpp>
#include <asio/steady_timer.hpp>
#include <juce_core/juce_core.h>
#include "HelperFunctions.h"

// TODO: Maybe replace all with bool MessageManager::callAsync (std::function<void()> fn)

class QueueThread : public juce::Thread, public juce::Thread::Listener
{
public:
    QueueThread(const juce::String& threadName = juce::String("queue_thread")) 
        : Thread(threadName)
    {
        addListener(this);
    }
    ~QueueThread()
    {

    }
    
    bool startThread()
    {
        JLOG("Starting thread: " + getThreadName());
        return Thread::startThread();
    }
    
    void exitSignalSent() override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mWorkPtr.reset();
        JLOG("Closing Thread: " + getThreadName());

    }
    asio::io_service& getIoService()
    {
        return mIoService;
    }

    void run() override
    {
        // lock the mutex
        std::unique_lock<std::mutex> lock(mMutex);

        // Create a work object to keep the io_service running
        mWorkPtr = WorkPtr(new asio::io_service::work(mIoService));

        // Run the io_service
        lock.unlock();
        mIoService.run();
        lock.lock();

        // The io_service has stopped running
        mIoService.reset();
    }

private:
    typedef std::unique_ptr< asio::io_service::work >  WorkPtr;
    WorkPtr mWorkPtr;
    std::mutex mMutex;
    asio::io_service mIoService;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QueueThread)
};