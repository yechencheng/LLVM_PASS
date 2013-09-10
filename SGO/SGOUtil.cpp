
#include <iostream>

using namespace std;

template<class T>
bool ExistOrCreate(T* &p)
{
    if(p != NULL)
        return true;
    else
    {
        p = new T;
        return false;
    }
}