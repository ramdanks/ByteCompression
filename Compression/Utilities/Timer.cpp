#include "Timer.h"
#include <iostream>
#include <direct.h>
#include <fstream>

namespace Ramdan
{
	Timer::Timer()
		:
		mTitle(""),
		mTime(SEC),
		mPrintOnDestroy(false)
	{
	}

	Timer::Timer( std::string Title, TimerPoint TP, bool PrintOnDestroy )
		:
		mTitle(Title),
		mTime(TP),
		mPrintOnDestroy(PrintOnDestroy)
	{
		Tic();
	}

	Timer::~Timer()
	{
		if ( mPrintOnDestroy )
		{
			float res = Toc();
			std::string spec;
			if ( mTime == ADJUST )
			{
				if ( res > MIN )
				{
					res = Adjust_Time( MIN, res );
					spec = "(min)";
				}
				else if ( res > SEC )
				{
					res = Adjust_Time( SEC, res );
					spec = "(sec)";
				}
				else if ( res * MS >= 1.0f )
				{
					res = Adjust_Time( MS, res );
					spec = "(ms)";
				}
				else if ( res * US >= 1.0f )
				{
					res = Adjust_Time( US, res );
					spec = "(us)";
				}
				else
				{
					res = Adjust_Time( NS, res );
					spec = "(ns)";
				}
			}
			std::cout << mTitle << " Time:" << res << spec << std::endl;
		}
	}

	void Timer::Tic()
	{
		TimeLog = std::chrono::high_resolution_clock::now();
	}

	float Timer::Toc()
	{
		Duration = std::chrono::high_resolution_clock::now() - TimeLog;
		return Adjust_Time( this->mTime, Duration.count() );
	}

	float Timer::Adjust_Time( TimerPoint TP, float Sec )
	{
		switch ( TP )
		{
			case MIN:	return Sec / MIN;
			case SEC:	return Sec;
			case MS:	return Sec * MS;
			case US:	return Sec * US;
			case NS:	return Sec * NS;
			default:	return Sec;
		}
	}

	StatTimer::StatTimer()
		:
		mLow( 0.0f ),
		mAvg( 0.0f ),
		mHigh( 0.0f ),
		mIteration( 0u )
	{
		Tic();
	}

	void StatTimer::Refresh()
	{
		float res = Toc();
		if ( mIteration == 0u )
		{
			mHigh = res;
			mLow = res;
			mAvg = res;
		}
		else
		{
			if ( res > mHigh ) mHigh = res;
			else if ( res < mLow ) mLow = res;
			mAvg = (mAvg * mIteration + res) / (mIteration + 1);
		}
		mIteration++;
		Tic();
	}

	void StatTimer::Print_Result( TimerPoint TP )
	{
		std::cout << "Low:" << Adjust_Time( TP, mLow ) << std::endl;
		std::cout << "Avg:" << Adjust_Time( TP, mAvg ) << std::endl;
		std::cout << "High:" << Adjust_Time( TP, mHigh ) << std::endl;
	}

	float StatTimer::Get_Low() const
	{
		return mLow;
	}

	float StatTimer::Get_Avg() const
	{
		return mAvg;
	}

	float StatTimer::Get_High() const
	{
		return mHigh;
	}
}