#include <Python.h>

//这是C的原生函数，实现+1功能
int add_one(int a) 
{
    return a + 1;
}



//这一串代码要实现的功能如下，按照python规定的调用方式：
//1.定义一个新的静态函数，接收2个PyObject *参数，返回1个PyObject *值
//2.PyArg_ParseTuple方法将python输入的变量变成C的变量，即上述args→num
//3.紧接着调用C原生函数add_one，传入num
//4.最后将调用返回的C变量，转换为PyObject*或其子类，并通过PyLong_FromLong方法，返回python值
static PyObject* py_add_one(PyObject* self, PyObject* args) 
{
    int num;
    if (!PyArg_ParseTuple(args, "i", &num)) return NULL;
    return PyLong_FromLong(add_one(num));
}



//这串代码的目的是创建一个数组，来指明Python可以调用这个扩展函数：
//其中"add_one"，代表编译后python调用时希望使用的函数名。而py_add_one，代表要调用当前C代码中的哪一个函数，即static PyObject * py_add_one。METH_VARARGS，代表函数的参数传递形式，主要包括位置参数和关键字参数两种
//最后如果希望添加新的函数，则在最后的{NULL, NULL}里按同样格式填写新的调用信息，比如加一些求和求乘积函数
static PyMethodDef Methods[] = 
{
    {"add_one", py_add_one, METH_VARARGS},
    {NULL, NULL}
};

//创建module的信息
static struct PyModuleDef cModule = 
{
    PyModuleDef_HEAD_INIT,
    "Test", /*模块名，即是生成模块后的名字，如numpy,opencv等等*/
    "", /* 模块文档，可以为空NULL */
    -1, /* 模块中每个解释器的状态大小，模块在全局变量中保持状态，则为-1 */
    Methods//即上一步定义的Methods，说明了可调用的函数集
};

//module初始化
PyMODINIT_FUNC PyInit_Test(void) { return PyModule_Create(&cModule); }


/*
setup.py

from distutils.core import setup, Extension #这里要用到distutils库
module1 = Extension('Test', sources = ['test.c']) #打包文件为test.c，这里没有设置路径，所有.c文件和setup.py放在同一目录下
setup (name = 'Test', #打包名
       version = '1.0', #版本
       description = 'This is a demo package', #说明文字
       ext_modules = [module1])



python setup.py build --compiler msvc #编译代码
python setup.py build --compiler msvc install #编译代码并直接将包放入当前python环境的包的路径以供调用

*/