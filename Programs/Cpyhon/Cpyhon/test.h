#pragma once

#ifdef BUILD_TEST
#define API_SYMBOL __declspec(dllexport)
#else
#define API_SYMBOL __declspec(dllimport)
#endif
//宏定义，导出或者导入//

extern "C" API_SYMBOL int sum(int x, int y);
//导出函数//
