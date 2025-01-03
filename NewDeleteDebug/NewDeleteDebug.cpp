#include <iostream>
#include "myNew.h"

class Test
{
public:
    char* a;
    int* b;
    Test() {}
    Test(int aSize, int bSize)
    {
        a = new char[aSize];
        b = new int[bSize];
    }
    ~Test()
    {
        delete[] a;
        //delete[] b;
    }
};
class Rest
{
public:
    int a;
    char b;
    Rest(){}
    Rest(int a, char b) : a(a),b(b)
    {}
    ~Rest()
    {
    }
};
class abc
{
public:
    Rest* r;
    abc(int a)
    {
        r = new Rest[a];
    }
    ~abc()
    {
        delete[] r;
    }
};
int main()
{
    Test t(10, 20);
    Rest* r = new Rest(10,20);
    Rest* rArray = new Rest[10];
    //delete r;
    delete[] rArray;
    abc a(10);
}