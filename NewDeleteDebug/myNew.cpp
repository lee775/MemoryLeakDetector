#include <time.h>
#include "myNew.h"
#include <iostream>

#undef new

using namespace std;
//vector<AllocInfo> allocVec;
AllocVec allocVecClass;

AllocInfo allocInfo[100];
int idx = 0;

FILE* logFile;
bool first = true;
wchar_t fileName[1024];

void* operator new (size_t size, const wchar_t* file, int line)
{
	void* p = malloc(size);

	//allocVecClass.addVec(p, size, file, line, false);
	allocInfo[idx++].addInfo(p, size, file, line, false);
	//return new char[size];
	return p;
}

void* operator new[](size_t size, const wchar_t* file, int line)
{
	void* p = malloc(size);

	//allocVecClass.addVec(p, size, file, line, true);
	allocInfo[idx++].addInfo(p, size, file, line, false);
	//return new char[size];
	return p;
}

void operator delete(void* p, char* file, int line)
{
	free(p);
}
void operator delete[](void* p, char* file, int line)
{
	free(p);
}

void operator delete(void* p)
{
	//vector <AllocInfo>::iterator Iter;
	//int i = 0;
	//for (Iter = allocVecClass.allocVec.begin(); Iter != allocVecClass.allocVec.end(); Iter++)
	//{
	//	if (Iter->ptr == p)
	//		break;
	//	i++;
	//}
	//Iter = allocVecClass.allocVec.begin() + i;
	//allocVecClass.allocVec.erase(Iter);
	int i;
	for (i = 0; i < idx; i++)
	{
		if (allocInfo[i].ptr == p)
		{
			allocInfo[i].ptr = nullptr;
			break;
		}
	}
	if (i == idx)
	{
		for (i = 0; i < idx; i++)
		{
			if (allocInfo[i].ptr == (void*)((int)p - sizeof(int)))
			{
				_wfopen_s(&logFile, fileName, L"at");
				fwprintf_s(logFile, L"NOALLOC\t[0x%d]\n", (int)p);
				fwprintf_s(logFile, L"ARRAY\t[0x%d][Line : %d] %s\n", (int)p, allocInfo[i].line, allocInfo[i].filename);
				fclose(logFile);
				return;
			}
		}
	}
	//delete p;
	free(p);
}

void operator delete[](void* p)
{
	//vector <AllocInfo>::iterator Iter;
	//int i = 0;
	//for (Iter = allocVecClass.allocVec.begin(); Iter != allocVecClass.allocVec.end(); Iter++)
	//{
	//	if (Iter->ptr == p)
	//		break;
	//	i++;
	//}
	//Iter = allocVecClass.allocVec.begin() + i;
	//allocVecClass.allocVec.erase(Iter);
	int i;
	for (i = 0; i < idx; i++)
	{
		if (allocInfo[i].ptr == p)
		{
			allocInfo[i].ptr = nullptr;
			break;
		}
	}
	if (i == idx)
	{
		for (i = 0; i < idx; i++)
		{
			if (allocInfo[i].ptr == (void*)((int)p + sizeof(int)))
			{
				_wfopen_s(&logFile, fileName, L"at");
				fwprintf_s(logFile, L"NOALLOC\t[0x%d]\n", (int)p);
				fwprintf_s(logFile, L"ARRAY\t[0x%d][Line : %d] %s\n", (int)p, allocInfo[i].line, allocInfo[i].filename);
				fclose(logFile);
				return;
			}
		}
	}

	//delete[] p;
	free(p);
}

AllocInfo::AllocInfo()
{
	if (first)
	{
		first = false;
		struct tm newtime;
		__time64_t long_time;
		_time64(&long_time);
		_localtime64_s(&newtime, &long_time);

		swprintf_s(fileName, 1024, L"Alloc_%d%02d%02d_%02d%02d%02d.txt", newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
		_wfopen_s(&logFile, fileName, L"at");
		fclose(logFile);
	}
}

AllocInfo::AllocInfo(void* p, int size, const wchar_t* fileName, int line, bool array)
{
	this->ptr = p;
	this->size = size;
	wcscpy_s(this->filename, 1024, fileName);
	this->line = line;
	this->array = array;
}

void AllocInfo::addInfo(void* p, int size, const wchar_t* fileName, int line, bool array)
{
	this->ptr = p;
	this->size = size;
	wcscpy_s(this->filename, 1024, fileName);
	this->line = line;
	this->array = array;
}

AllocInfo::~AllocInfo()
{
	if (this->ptr != nullptr)
	{
		_wfopen_s(&logFile, fileName, L"at");
		char log[1024];
		fwprintf_s(logFile, L"LEAK\t[0x%d][Line : %d] %s\n", (int)this->ptr, this->line, this->filename);
		fclose(logFile);
		//cout << "주소 : " << this->ptr << endl;
		//cout << "파일명 : " << this->filename << endl;
		//cout << "줄번호 : " << this->line << endl;
		//cout << "사이즈 : " << this->size << endl;
		//cout << "배열 여부 " << this->array << endl << endl;
	}
}


AllocVec::AllocVec()
{
	this->allocVec.reserve(100);
}
AllocVec::~AllocVec()
{

	vector <AllocInfo>::iterator Iter;
	for (Iter = allocVec.begin(); Iter != allocVec.end(); Iter++)
	{
		cout << "주소 : " << Iter->ptr << endl;
		cout << "파일명 : " << Iter->filename << endl;
		cout << "줄번호 : " << Iter->line << endl;
		cout << "사이즈 : " << Iter->size << endl;
		cout << "배열 여부 " << Iter->array << endl << endl;
	}

}
void AllocVec::addVec(void* p, int size, const wchar_t* fileName, int line, bool array)
{
	this->allocVec.emplace_back(p, size, fileName, line, array);
}
#define new new(__FILE__, __LINE__)