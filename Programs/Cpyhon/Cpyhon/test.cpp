#define BUILD_TEST
#include "test.h"
#include "stdio.h"

//通过CTypes调用
#define DLLEXPORT extern "C" __declspec(dllexport)    //直接在源文件定义导出

DLLEXPORT int sum(int a, int b) {
    return a + b;
}//两数相加