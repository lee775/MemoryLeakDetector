#pragma once
#include <vector>

class AllocInfo
{
public:
	void* ptr;
	int size;
	wchar_t filename[1024];
	int  line;
	bool array;
	void addInfo(void*, int, const wchar_t*, int, bool);
	AllocInfo();
	AllocInfo(void*, int, const wchar_t*, int, bool);
	~AllocInfo();
};

class AllocVec
{
public:
	std::vector<AllocInfo> allocVec;
	void addVec(void*, int, const wchar_t*, int, bool);
	AllocVec();
	~AllocVec();
};

void* operator new (size_t size, const wchar_t* file, int line);
void* operator new[](size_t size, const wchar_t* file, int line);
void operator delete(void* p, char* file, int line);
void operator delete[](void* p, char* file, int line);
void operator delete(void* p);
void operator delete[](void* p);

#define new new(__FILE__, __LINE__)