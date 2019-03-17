#pragma once

#include "Win32.h"

#include <cstdint>
#include <cmath>
#include <AntTweakBar.h>

// Based off of "StepTimer.h" from the Direct3D Win32 Game Template by Chuck Walbourn.
// https://github.com/walbourn/directx-vs-templates/blob/master/d3d11game_win32/StepTimer.h

class Timer
{
private:

	// Source timing data uses QPC units.
	LARGE_INTEGER _qpcFrequency;
	LARGE_INTEGER _qpcLastTime;
	uint64_t _qpcMaxDelta;

	// Derived timing data uses a canonical tick format.
	uint64_t _elapsedTicks;
	uint64_t _totalTicks;
	uint64_t _leftOverTicks;

	// Members for tracking the framerate.
	uint32_t _frameCount;
	uint32_t _framesPerSecond;
	uint32_t _framesThisSecond;
	uint64_t _qpcSecondCounter;

	// Members for configuring fixed timestep mode.
	bool _isFixedTimeStep;
	uint64_t _targetElapsedTicks;
	double _targetElapsedSeconds;

	static void TW_CALL TwSetTargetFrequency(const void *value, void *clientData)
	{
		static_cast<Timer *>(clientData)->SetTargetFrequency(*static_cast<const double *>(value));
	}

	static void TW_CALL TwGetTargetFrequency(void *value, void *clientData)
	{
		*static_cast<double *>(value) = static_cast<Timer *>(clientData)->GetTargetFrequency();
	}

public:

	Timer() :
		_elapsedTicks(0),
		_totalTicks(0),
		_leftOverTicks(0),
		_frameCount(0),
		_framesPerSecond(0),
		_framesThisSecond(0),
		_qpcSecondCounter(0),
		_isFixedTimeStep(false),
		_targetElapsedTicks(TicksPerSecond / 60)
	{
		QueryPerformanceFrequency(&_qpcFrequency);
		QueryPerformanceCounter(&_qpcLastTime);

		// Initialize max delta to 1/10 of a second.
		_qpcMaxDelta = _qpcFrequency.QuadPart / 10;
	}

	// Get elapsed time since the previous Update call.
	uint64_t GetElapsedTicks() const { return _elapsedTicks; }
	double GetElapsedSeconds() const { return TicksToSeconds(_elapsedTicks); }

	double GetTargetFrequency()
	{
		return static_cast<double>(TicksPerSecond / _targetElapsedTicks);
	}

	// Get total time since the start of the program.
	uint64_t GetTotalTicks() const { return _totalTicks; }
	double GetTotalSeconds() const { return TicksToSeconds(_totalTicks); }

	// Get total number of updates since start of the program.
	uint32_t GetFrameCount() const { return _frameCount; }

	// Get the current framerate.
	uint32_t GetFramesPerSecond() const { return _framesPerSecond; }

	// Set whether to use fixed or variable timestep mode.
	void SetFixedTimeStep(bool isFixedTimestep) { _isFixedTimeStep = isFixedTimestep; }

	// Set how often to call Update when in fixed timestep mode.
	void SetTargetElapsedTicks(uint64_t targetElapsed) 
	{ 
		_targetElapsedTicks = targetElapsed; 
	}
	void SetTargetElapsedSeconds(double targetElapsed) 
	{ 
		_targetElapsedTicks = SecondsToTicks(targetElapsed); 
	}

	void SetTargetFrequency(double frequency)
	{
		_targetElapsedTicks = static_cast<uint64_t>(TicksPerSecond / frequency);
	}

	// Integer format represents time using 10,000,000 ticks per second.
	static const uint64_t TicksPerSecond = 10000000;

	static double TicksToSeconds(uint64_t ticks) { return static_cast<double>(ticks) / TicksPerSecond; }
	static uint64_t SecondsToTicks(double seconds) { return static_cast<uint64_t>(seconds * TicksPerSecond); }
	
	void RegisterTwBarCallbacks(TwBar* bar, const char* timerName)
	{
		TwAddVarCB(bar, timerName, 
			TW_TYPE_DOUBLE, 
			Timer::TwSetTargetFrequency, 
			Timer::TwGetTargetFrequency, 
			this, "");
	}

	// After an intentional timing discontinuity (for instance a blocking IO operation)
	// call this to avoid having the fixed timestep logic attempt a set of catch-up 
	// Update calls.

	void ResetElapsedTime()
	{
		QueryPerformanceCounter(&_qpcLastTime);

		_leftOverTicks = 0;
		_framesPerSecond = 0;
		_framesThisSecond = 0;
		_qpcSecondCounter = 0;
	}

	// Update timer state, calling the specified Update function the appropriate number of times.
	template<typename TUpdate>
	void Tick(const TUpdate& update)
	{
		// Query the current time.
		LARGE_INTEGER currentTime;

		QueryPerformanceCounter(&currentTime);

		uint64_t timeDelta = currentTime.QuadPart - _qpcLastTime.QuadPart;

		_qpcLastTime = currentTime;
		_qpcSecondCounter += timeDelta;

		// Clamp excessively large time deltas (e.g. after paused in the debugger).
		if (timeDelta > _qpcMaxDelta)
		{
			timeDelta = _qpcMaxDelta;
		}

		// Convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
		timeDelta *= TicksPerSecond;
		timeDelta /= _qpcFrequency.QuadPart;

		uint32_t lastFrameCount = _frameCount;

		if (_isFixedTimeStep)
		{
			// Fixed timestep update logic.

			// If the app is running very close to the target elapsed time (within 1/4 of a millisecond) just clamp
			// the clock to exactly match the target value. This prevents tiny and irrelevant errors
			// from accumulating over time. Without this clamping, a game that requested a 60 fps
			// fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
			// accumulate enough tiny errors that it would drop a frame. It is better to just round 
			// small deviations down to zero to leave things running smoothly.

			if (abs(static_cast<int64_t>(timeDelta - _targetElapsedTicks)) < TicksPerSecond / 4000)
			{
				timeDelta = _targetElapsedTicks;
			}

			_leftOverTicks += timeDelta;

			while (_leftOverTicks >= _targetElapsedTicks)
			{
				_elapsedTicks = _targetElapsedTicks;
				_totalTicks += _targetElapsedTicks;
				_leftOverTicks -= _targetElapsedTicks;
				_frameCount++;

				update();
			}
		}
		else
		{
			// Variable timestep update logic.
			_elapsedTicks = timeDelta;
			_totalTicks += timeDelta;
			_leftOverTicks = 0;
			_frameCount++;

			update();
		}

		// Track the current framerate.
		if (_frameCount != lastFrameCount)
		{
			_framesThisSecond++;
		}

		if (_qpcSecondCounter >= static_cast<uint64_t>(_qpcFrequency.QuadPart))
		{
			_framesPerSecond = _framesThisSecond;
			_framesThisSecond = 0;
			_qpcSecondCounter %= _qpcFrequency.QuadPart;
		}
	}
};