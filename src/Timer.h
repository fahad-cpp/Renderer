#pragma once
#include <chrono>
class Timer {
public:
	float dtms;
	Timer() {
		m_StartTimePoint = std::chrono::high_resolution_clock::now();
		dtms = 0;
	}
	~Timer() {
		Stop();
	}
	void Stop() {
		auto endTimePoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

		long long duration = end - start;
		dtms = (double(duration) * 0.001);

	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
};