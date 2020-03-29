#pragma once

const ULONG MAX_SAMPLE_COUNT = 50;

class Timer
{
public:
	Timer();
	~Timer();

private:
	double			m_TimeScale;
	float			m_TimeElapsed;

	__int64			m_nBasePerformanceCounter;
	__int64			m_nPausedPerformanceCounter;
	__int64			m_nStopPerformanceCounter;
	__int64			m_nCurrentPerformanceCounter;
	__int64			m_nLastPerformanceCounter;

	__int64			m_nPerformanceFrequencyPerSec;

	float			m_FrameTime[MAX_SAMPLE_COUNT];
	ULONG			m_nSampleCount;

	unsigned long	m_nCurrentFrameRate;
	unsigned long	m_nFramesPerSecond;
	float			m_FPSTimeElapsed;

	bool			m_Stopped;

public:
	void				Tick(float LockFPS = 0.f);
	void				Reset();
	
	unsigned long		GetFrameRate(LPTSTR string = NULL, int charNum = 0);
	float				GetTimeElapsed();

};

