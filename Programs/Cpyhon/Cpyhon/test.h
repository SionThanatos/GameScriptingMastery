#pragma once

#ifdef BUILD_TEST
#define API_SYMBOL __declspec(dllexport)
#else
#define API_SYMBOL __declspec(dllimport)
#endif
//�궨�壬�������ߵ���//

extern "C" API_SYMBOL int sum(int x, int y);
//��������//
