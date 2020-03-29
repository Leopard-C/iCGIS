#pragma once

/************* Detect memory leak ******************/
/******* Seriously slowing down the program ********/
#ifdef _WIN32

    #define USING_VLD 0

    #if USING_VLD == 1
        #include "vld.h"
    #endif

#endif // _WIN32
/****************************************************/
