#pragma once

#ifndef TRACY_ENABLE
	#define Leacon_Profiler						do { } while(0)
	#define Leacon_ProfilerFrame				do { } while(0)
	#define Leacon_ProfilerSection(x)			do { } while(0)
	#define Leacon_ProfilerTag(y, x)			do { } while(0)
	#define Leacon_ProfilerLog(text, size)		do { } while(0)
	#define Leacon_ProfilerValue(text, value)	do { } while(0)
	#define Leacon_ProfilerEvalRet(a)			(a)
	#define Leacon_ProfilerEval(a)				(a)
#else	
	#include "tracy/Tracy.hpp"
	#define Leacon_Profiler						ZoneScoped
	#define Leacon_ProfilerFrame				FrameMark
	#define Leacon_ProfilerSection(x)			ZoneScopedN(x)
	#define Leacon_ProfilerTag(y, x)			ZoneText(x, strlen(x))
	#define Leacon_ProfilerLog(text, size)		TracyMessage(text, size)
	#define Leacon_ProfilerValue(text, value)	TracyPlot(text, value)
	#define Leacon_ProfilerEvalRet(a)			([&](){ Leacon_ProfilerSection(#a); return (a); }())
	#define Leacon_ProfilerEval(a)				([&](){ Leacon_ProfilerSection(#a); (a); }())
#endif