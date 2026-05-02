#pragma once

#include <containers/atomicringbuffer.h>
#include <engine.h>
#include <memory>
#include <thread>

class EngineWorker
{
    std::unique_ptr<Engine> m_engine;
    int editorPID;
    std::string url;

    bool m_listening;
    bool m_running;
    std::thread publisherThread;
    std::thread m_engineThread;
    std::thread m_repThread;
    std::thread m_subThread;

  public:
    EngineWorker(std::unique_ptr<Application> app, int editorPID, const std::string &url);
    ~EngineWorker();

    void start();
    void processEvents();
    Engine &getEngine();
};
