#ifndef GAMETIMER_H
#define GAMETIMER_H


class GameTimer {
    public:
        GameTimer();

        float getGameTime()const;  // in seconds
        float getDeltaTime()const; // in seconds

        void reset(); // Call before message loop.
        void start(); // Call when unpaused.
        void stop();  // Call when paused.
        void tick();  // Call every frame.

    private:
        double m_secondsPerCount;
        double m_deltaTime;

        __int64 m_baseTime;
        __int64 m_pausedTime;
        __int64 m_stopTime;
        __int64 m_prevTime;
        __int64 m_currTime;

        bool m_stopped;
};

#endif // GAMETIMER_H