#include "stdafx.h"
#include "Timer.h"

Timer::Timer()
{
	::QueryPerformanceFrequency((LARGE_INTEGER*)&m_nPerformanceFrequencyPerSec);
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_nLastPerformanceCounter);

	m_TimeScale = 1.0 / (double)m_nPerformanceFrequencyPerSec;

	m_nBasePerformanceCounter = m_nLastPerformanceCounter;
	m_nPausedPerformanceCounter = 0;
	m_nStopPerformanceCounter = 0;

	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_nFramesPerSecond = 0;
	m_FPSTimeElapsed = 0.f;
}

Timer::~Timer()
{

}

void Timer::Tick(float LockFPS)
{
	if (m_Stopped) {
		m_TimeElapsed = 0.f;
		return;
	}

	float TimeElapsed;

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
	TimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_TimeScale);

	if (LockFPS > 0.f) {
		while (TimeElapsed < (1.f / LockFPS)) {
			::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
			TimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter)*m_TimeScale);
		}
	}

	m_nLastPerformanceCounter = m_nCurrentPerformanceCounter;

	if (fabsf(TimeElapsed - m_TimeElapsed < 1.f)) {
		::memmove(&m_FrameTime[1], m_FrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_FrameTime[0] = TimeElapsed;
		if (m_nSampleCount < MAX_SAMPLE_COUNT)
			++m_nSampleCount;
	}

	++m_nFramesPerSecond;
	m_FPSTimeElapsed += TimeElapsed;
	
	if (m_FPSTimeElapsed > 1.f) {
		m_nCurrentFrameRate = m_nFramesPerSecond;
		m_nFramesPerSecond = 0;
		m_FPSTimeElapsed = 0.f;
	}

	m_TimeElapsed = 0.f;
	
	for (ULONG i = 0; i < m_nSampleCount; ++i)
		m_TimeElapsed += m_FrameTime[i];
	if (m_nSampleCount > 0)
		m_TimeElapsed /= m_nSampleCount;

}

void Timer::Reset()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);

	m_nBasePerformanceCounter = nPerformanceCounter;
	m_nLastPerformanceCounter = nPerformanceCounter;
	m_nStopPerformanceCounter = 0;
	m_Stopped = false;
}

unsigned long Timer::GetFrameRate(LPTSTR string, int charNum)
{
	if (string) {
		_itow_s(m_nCurrentFrameRate, string, charNum, 10);
		wcscat_s(string, charNum, _T(" FPS)"));
	}
	
	return m_nCurrentFrameRate;
}

float Timer::GetTotalTime()
{
	if (m_Stopped)
		return float((m_nStopPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) *m_TimeScale;
	return float((m_nCurrentPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_TimeElapsed;
}