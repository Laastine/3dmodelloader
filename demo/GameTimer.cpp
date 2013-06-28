#include "GameTimer.h"
#include <windows.h>

GameTimer::GameTimer() : m_secondsPerCount(0.0), m_deltaTime(-1.0), m_baseTime(0),
    m_pausedTime(0), m_prevTime(0), m_currTime(0), m_stopTime(0), m_stopped(false) {
    __int64 countsPerSec;
    QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
    m_secondsPerCount = 1.0 / (double)countsPerSec;
}

/*Get time minus paused time*/
float GameTimer::getGameTime() const {
    if( m_stopped ) {
        return (float) ((m_stopTime - m_baseTime) * m_secondsPerCount);
    } else {
        return (float) (((m_currTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
    }
}

float GameTimer::getDeltaTime() const {
    return (float)m_deltaTime;
}

void GameTimer::reset() {
    __int64 currTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

    m_baseTime = currTime;
    m_prevTime = currTime;
    m_stopTime = 0;
    m_stopped  = false;
}

void GameTimer::start() {
    __int64 startTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

    if( m_stopped ) {
        m_pausedTime += (startTime - m_stopTime);

        m_prevTime = startTime;
        m_stopTime = 0;
        m_stopped  = false;
    }
}

void GameTimer::stop() {
    if( !m_stopped ) {
        __int64 currTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

        m_stopTime = currTime;
        m_stopped  = true;
    }
}

void GameTimer::tick() {
    if( m_stopped ) {
        m_deltaTime = 0.0;
        return;
    }

    __int64 currTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
    m_currTime = currTime;

    // Time difference between this frame and the previous.
    m_deltaTime = (m_currTime - m_prevTime) * m_secondsPerCount;

    //Update for next screen frame
    m_prevTime = m_currTime;

    if(m_deltaTime < 0.0) {
        m_deltaTime = 0.0;
    }
}
