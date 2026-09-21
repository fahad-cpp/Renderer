#pragma once
#include <chrono>
class Timer {
  public:
    double dtms;
    Timer() {
        m_StartTimePoint = std::chrono::high_resolution_clock::now();
        dtms             = 0;
    }
    ~Timer() {
        Stop();
    }
    void Stop() {
        std::chrono::time_point<std::chrono::high_resolution_clock> endTimePoint = std::chrono::high_resolution_clock::now();
        dtms                                                                     = std::chrono::duration<double, std::chrono::microseconds::period>(endTimePoint - m_StartTimePoint).count();
        dtms                                                                     = dtms / 1000.0;
    }

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
};
