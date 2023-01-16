# -*- coding: utf-8 -*- 
from ctypes import *
from ctypes import *    
test = CDLL("./Cpyhon.dll")

print("加载成功")

result =test.sum(5,10)    #调用库里的函数sum，求和函数
print(result)    #打印结果