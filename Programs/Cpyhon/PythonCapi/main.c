#include <Python.h>

//����C��ԭ��������ʵ��+1����
int add_one(int a) 
{
    return a + 1;
}



//��һ������Ҫʵ�ֵĹ������£�����python�涨�ĵ��÷�ʽ��
//1.����һ���µľ�̬����������2��PyObject *����������1��PyObject *ֵ
//2.PyArg_ParseTuple������python����ı������C�ı�����������args��num
//3.�����ŵ���Cԭ������add_one������num
//4.��󽫵��÷��ص�C������ת��ΪPyObject*�������࣬��ͨ��PyLong_FromLong����������pythonֵ
static PyObject* py_add_one(PyObject* self, PyObject* args) 
{
    int num;
    if (!PyArg_ParseTuple(args, "i", &num)) return NULL;
    return PyLong_FromLong(add_one(num));
}



//�⴮�����Ŀ���Ǵ���һ�����飬��ָ��Python���Ե��������չ������
//����"add_one"����������python����ʱϣ��ʹ�õĺ���������py_add_one������Ҫ���õ�ǰC�����е���һ����������static PyObject * py_add_one��METH_VARARGS���������Ĳ���������ʽ����Ҫ����λ�ò����͹ؼ��ֲ�������
//������ϣ������µĺ�������������{NULL, NULL}�ﰴͬ����ʽ��д�µĵ�����Ϣ�������һЩ�����˻�����
static PyMethodDef Methods[] = 
{
    {"add_one", py_add_one, METH_VARARGS},
    {NULL, NULL}
};

//����module����Ϣ
static struct PyModuleDef cModule = 
{
    PyModuleDef_HEAD_INIT,
    "Test", /*ģ��������������ģ�������֣���numpy,opencv�ȵ�*/
    "", /* ģ���ĵ�������Ϊ��NULL */
    -1, /* ģ����ÿ����������״̬��С��ģ����ȫ�ֱ����б���״̬����Ϊ-1 */
    Methods//����һ�������Methods��˵���˿ɵ��õĺ�����
};

//module��ʼ��
PyMODINIT_FUNC PyInit_Test(void) { return PyModule_Create(&cModule); }


/*
setup.py

from distutils.core import setup, Extension #����Ҫ�õ�distutils��
module1 = Extension('Test', sources = ['test.c']) #����ļ�Ϊtest.c������û������·��������.c�ļ���setup.py����ͬһĿ¼��
setup (name = 'Test', #�����
       version = '1.0', #�汾
       description = 'This is a demo package', #˵������
       ext_modules = [module1])



python setup.py build --compiler msvc #�������
python setup.py build --compiler msvc install #������벢ֱ�ӽ������뵱ǰpython�����İ���·���Թ�����

*/