#ifndef _BINK_EXPORTS_
#define _BINK_EXPORTS_

#define BINK_SURFACE_32          3

struct BINK
{
	uInt Width;
	uInt Height;
};

#ifdef _M_IX86

#define	BINK_DLL_NAME							"binkw32.dll"
#define	BINK_OPEN_FUNC_NAME						"_BinkOpen@8"
#define	BINK_GET_RECTS_FUNC_NAME				"_BinkGetRects@8"
#define	BINK_SET_SOUND_TRACK_FUNC_NAME			"_BinkSetSoundTrack@8"
#define	BINK_GOTO_FUNC_NAME						"_BinkGoto@12"
#define	BINK_OPEN_DIRECT_SOUND_FUNC_NAME		"_BinkOpenDirectSound@4"
#define	BINK_SET_SOUND_SYSTEM_FUNC_NAME			"_BinkSetSoundSystem@8"
#define	BINK_SET_SOUND_ON_OFF_FUNC_NAME			"_BinkSetSoundOnOff@8"
#define	BINK_OPEN_MILES_FUNC_NAME				"_BinkOpenMiles@4"
#define	BINK_GET_SUMMARY_FUNC_NAME				"_BinkGetSummary@8"
#define	BINK_GET_KEY_FRAME_FUNC_NAME			"_BinkGetKeyFrame@12"
#define	BINK_GET_REALTIME_FUNC_NAME				"_BinkGetRealtime@12"
#define	BINK_GET_TRACK_ID_FUNC_NAME				"_BinkGetTrackID@8"
#define	BINK_SET_VOLUME_G1_FUNC_NAME			"_BinkSetVolume@8"
#define	BINK_SET_VOLUME_G2_FUNC_NAME			"_BinkSetVolume@12"
#define	BINK_CLOSE_FUNC_NAME					"_BinkClose@4"
#define	BINK_WAIT_FUNC_NAME						"_BinkWait@4"
#define	BINK_REGISTER_FRAME_BUFFERS_FUNC_NAME	"_BinkRegisterFrameBuffers@8"
#define	BINK_DO_FRAME_FUNC_NAME					"_BinkDoFrame@4"
#define	BINK_SHOULD_SKIP_FUNC_NAME				"_BinkShouldSkip@4"
#define	BINK_NEXT_FRAME_FUNC_NAME				"_BinkNextFrame@4"
#define	BINK_GET_FRAME_BUFFERS_INFO_FUNC_NAME	"_BinkGetFrameBuffersInfo@8"
#define	BINK_PAUSE_FUNC_NAME					"_BinkPause@8"
#define	BINK_COPY_TO_BUFFER_FUNC_NAME			"_BinkCopyToBuffer@28"
#define	BINK_COPY_TO_BUFFER_RECT_FUNC_NAME		"_BinkCopyToBufferRect@44"
#define	BINK_GET_ERROR_FUNC_NAME				"_BinkGetError@0"

#else

#define	BINK_DLL_NAME							"binkw64.dll"
#define	BINK_OPEN_FUNC_NAME						"BinkOpen"
#define	BINK_GET_RECTS_FUNC_NAME				"BinkGetRects"
#define	BINK_SET_SOUND_TRACK_FUNC_NAME			"BinkSetSoundTrack"
#define	BINK_GOTO_FUNC_NAME						"BinkGoto"
#define	BINK_OPEN_DIRECT_SOUND_FUNC_NAME		"BinkOpenDirectSound"
#define	BINK_SET_SOUND_SYSTEM_FUNC_NAME			"BinkSetSoundSystem"
#define	BINK_GET_SUMMARY_FUNC_NAME				"BinkGetSummary"
#define	BINK_GET_KEY_FRAME_FUNC_NAME			"BinkGetKeyFrame"
#define	BINK_GET_REALTIME_FUNC_NAME				"BinkGetRealtime"
#define	BINK_GET_TRACK_ID_FUNC_NAME				"BinkGetTrackID"
#define	BINK_SET_VOLUME_FUNC_NAME				"BinkSetVolume"
#define	BINK_CLOSE_FUNC_NAME					"BinkClose"
#define	BINK_WAIT_FUNC_NAME						"BinkWait"
#define	BINK_REGISTER_FRAME_BUFFERS_FUNC_NAME	"BinkRegisterFrameBuffers"
#define	BINK_DO_FRAME_FUNC_NAME					"BinkDoFrame"
#define	BINK_SHOULD_SKIP_FUNC_NAME				"BinkShouldSkip"
#define	BINK_NEXT_FRAME_FUNC_NAME				"BinkNextFrame"
#define	BINK_GET_FRAME_BUFFERS_INFO_FUNC_NAME	"BinkGetFrameBuffersInfo"
#define	BINK_PAUSE_FUNC_NAME					"BinkPause"
#define	BINK_COPY_TO_BUFFER_FUNC_NAME			"BinkCopyToBuffer"
#define	BINK_COPY_TO_BUFFER_RECT_FUNC_NAME		"BinkCopyToBufferRect"
#define	BINK_GET_ERROR_FUNC_NAME				"BinkGetError"

#endif

using BinkOpenFuncPtr = BINK* (__stdcall*)(const char* name, uInt flags);
using BinkGetRectsFuncPtr = int(__stdcall*)(BINK* bnk, uInt flags);
using BinkSetSoundTrackFuncPtr = void(__stdcall*)(uInt total_tracks, uInt* tracks);
using BinkGotoFuncPtr = void(__stdcall*)(BINK* bnk, uInt frame, int flags);
using BinkOpenDirectSoundFuncPtr = void* (__stdcall*)(uInt param);
using BinkSetSoundSystemFuncPtr = int(__stdcall*)(void* open, uInt param);
using BinkSetSoundOnOffFuncPtr = int(__stdcall*)(BINK* bnk, int onoff);
using BinkOpenMilesFuncPtr = void* (__stdcall*)(uInt param);
using BinkGetSummaryFuncPtr = void(__stdcall*)(BINK* bnk, void* sum);
using BinkGetKeyFrameFuncPtr = uInt(__stdcall*)(BINK* bnk, uInt frame, int flags);
using BinkGetRealtimeFuncPtr = void(__stdcall*)(BINK* bink, void* run, uInt frames);
using BinkGetTrackIDFuncPtr = uInt(__stdcall*)(BINK* bnk, uInt trackindex);
using BinkSetVolumeFuncG2Ptr = void(__stdcall*)(BINK* bnk, uInt trackid, int volume);
using BinkSetVolumeFuncG1Ptr = void(__stdcall*)(BINK* bnk, int volume);
using BinkCloseFuncPtr = void(__stdcall*)(BINK* bnk);
using BinkWaitFuncPtr = int(__stdcall*)(BINK* bnk);
using BinkRegisterFrameBuffersFuncPtr = void(__stdcall*)(BINK* bink, void* fbset);
using BinkDoFrameFuncPtr = int(__stdcall*)(BINK* bnk);
using BinkShouldSkipFuncPtr = int(__stdcall*)(BINK* bink);
using BinkNextFrameFuncPtr = void(__stdcall*)(BINK* bnk);
using BinkGetFrameBuffersInfoFuncPtr = void(__stdcall*)(BINK* bink, void* fbset);
using BinkPauseFuncPtr = int(__stdcall*)(BINK* bnk, int pause);
using BinkCopyToBufferFuncPtr = int(__stdcall*)(BINK* bnk,
                                                void* dest,
                                                int destpitch,
                                                uInt destheight,
                                                uInt destx,
                                                uInt desty,
                                                uInt flags);
using BinkCopyToBufferRectFuncPtr = int(__stdcall*)(BINK* bnk,
                                                    void* dest,
                                                    int destpitch,
                                                    uInt destheight,
                                                    uInt destx,
                                                    uInt desty,
                                                    uInt srcx,
                                                    uInt srcy,
                                                    uInt srcw,
                                                    uInt srch,
                                                    uInt flags);
using BinkGetErrorFuncPtr = const char* (__stdcall*)();

#endif
