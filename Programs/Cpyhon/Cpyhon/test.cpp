#define BUILD_TEST
#include "test.h"
#include "stdio.h"

//ͨ��CTypes����
#define DLLEXPORT extern "C" __declspec(dllexport)    //ֱ����Դ�ļ����嵼��

DLLEXPORT int sum(int a, int b) {
    return a + b;
}//�������