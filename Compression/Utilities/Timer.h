/*======================================*/
// Author	: Ramadhan Kalih Sewu		//
// About	: Logging (Timer)			//
/*======================================*/
#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <string>

enum TimerPoint
{ 
	ADJUST,
	MIN		= 60,
	SEC		= 1,
	MS		= 1000,
	US		= 1000000,
	NS		= 1000000000
};

namespace Ramdan
{
	class Timer
	{
		std::string mTitle;
		TimerPoint mTime;
		bool mPrintOnDestroy;

	public:
		Timer();
		Timer( std::string Title, TimerPoint TP, bool PrintOnDestroy );
		~Timer();

		void Tic();
		float Toc();
		static float Adjust_Time( TimerPoint TP, float Sec );

	private:
		std::chrono::time_point<std::chrono::steady_clock> TimeLog;
		std::chrono::duration<float> Duration;
	};

	class StatTimer : public Timer
	{
		float mLow, mAvg, mHigh;
		long long mIteration;

	public:
		StatTimer();
		void Refresh();
		void Print_Result( TimerPoint TP );

		float Get_Low() const;
		float Get_Avg() const;
		float Get_High() const;
	};
}