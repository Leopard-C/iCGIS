#pragma once

/***************** 检测内存泄露 **************/
/*********** 会严重影响程序运行速度 **********/
#ifdef USE_VLD								//
#undef USE_VLD								//
#endif										//
											//
#define USE_VLD 1							//
											//
#if USE_VLD == 0							//
#include "vld.h"							//
#endif										//
/********************************************/
