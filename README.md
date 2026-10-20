# MemoryLeakDetector

`new` / `delete`를 오버로딩해 할당 위치(파일, 줄 번호)를 기록하고, 프로그램이 끝날 때 해제되지 않은 메모리를 로그 파일로 남기는 메모리 누수 탐지기다.

## 소개

- 매크로로 `new`를 `new(__FILE__, __LINE__)`로 바꿔 모든 할당 지점을 추적한다.
- 외부 도구 없이, 프로그램 종료 시점에 누수 목록(주소, 줄 번호, 파일 경로)을 텍스트 파일로 남긴다.
- 솔루션과 프로젝트 이름은 `NewDeleteDebug`다.

## 주요 기능

- 할당 기록: 파일명과 줄 번호를 인자로 받는 전역 `operator new` / `operator new[]`
- 해제 기록: 전역 `operator delete` / `operator delete[]`를 재정의해 기록에서 해당 주소를 지운다.
- 누수 보고: 종료 시 해제되지 않은 할당을 `LEAK` 줄로 기록한다.
- 짝이 맞지 않는 해제 탐지: `new[]`로 받은 메모리를 `delete`로, 또는 `new`로 받은 메모리를 `delete[]`로 해제하면 `NOALLOC` / `ARRAY` 줄로 기록한다.
- 로그 파일 자동 생성: 실행 시각으로 이름을 붙인 `Alloc_YYYYMMDD_HHMMSS.txt`

로그 예시 (저장소의 [`Alloc_20240928_060231.txt`](NewDeleteDebug/Alloc_20240928_060231.txt)):

```
LEAK	[0x22137376][Line : 49] C:\Users\a\source\repos\NewDeleteDebug\NewDeleteDebug\NewDeleteDebug.cpp
LEAK	[0x22250456][Line : 13] C:\Users\a\source\repos\NewDeleteDebug\NewDeleteDebug\NewDeleteDebug.cpp
```

## 핵심 구현

### 할당 위치 기록: [`myNew.h`](NewDeleteDebug/myNew.h), [`myNew.cpp`](NewDeleteDebug/myNew.cpp)

- `#define new new(__FILE__, __LINE__)`로 사용자 코드의 `new T`를 `operator new(size, file, line)` 호출로 바꾼다.
- `myNew.cpp`는 맨 앞에서 `#undef new`를 해 자기 자신에는 매크로가 적용되지 않게 한다.
- 실제 메모리는 `malloc`으로 할당하고 `free`로 해제한다.
- 할당 정보 `AllocInfo`(주소, 크기, 파일명, 줄 번호)는 전역 고정 배열 `allocInfo[100]`에 저장한다. 그래서 한 번 실행에 최대 100건까지 기록한다.
- `delete`가 호출되면 배열에서 같은 주소를 찾아 기록을 지운다(`ptr = nullptr`).

### 종료 시 누수 보고: 전역 객체의 소멸자 활용

- `allocInfo` 배열은 전역 객체라 `main()`이 끝난 뒤 정적 객체 소멸 단계에서 각 원소의 소멸자가 불린다.
- `~AllocInfo()`는 `ptr`이 남아 있으면, 즉 해제되지 않았으면 `LEAK [주소][Line : 줄] 파일경로`를 로그 파일에 추가한다.
- 첫 `AllocInfo` 생성자가 불릴 때 현재 시각으로 로그 파일 이름을 정하고 빈 파일을 만든다.

### 짝이 맞지 않는 해제 탐지

- 소멸자가 있는 타입을 `new[]`로 할당하면 컴파일러가 배열 원소 개수(배열 쿠키)를 앞에 붙인다. 그래서 짝이 맞지 않게 해제하면 `operator delete`에 들어오는 주소가 기록된 주소와 쿠키 크기만큼 어긋난다.
- `delete` / `delete[]`에 들어온 주소가 기록에 없으면, `sizeof(int)`만큼 어긋난 주소가 기록에 있는지 다시 찾는다. 있으면 `NOALLOC` / `ARRAY` 로그를 남기고 잘못된 해제는 하지 않는다. 이 할당은 해제되지 않은 채 남으므로 종료 시 `LEAK`으로도 보고된다.
- 포인터를 `int`로 변환해 비교·출력하고 배열 쿠키를 4바이트로 가정하므로 x86(Win32) 빌드를 기준으로 작성되었다.

### 예제: [`NewDeleteDebug.cpp`](NewDeleteDebug/NewDeleteDebug.cpp)

- 13번 줄: `Test` 생성자에서 `new int[]`로 할당한 `b`를 소멸자에서 해제하지 않는다.
- 49번 줄: `new Rest(10, 20)`으로 할당한 객체를 `delete`하지 않는다.
- 위 로그 예시의 두 `LEAK` 줄이 이 두 지점이다.

## 기술 스택

- 언어: C++ (전역 `operator new` / `delete` 오버로딩, 매크로)
- 플랫폼: Windows 콘솔, MSVC CRT (`malloc` / `free`, `_wfopen_s`, `_time64`)
- 개발 환경: Visual Studio 2022 (플랫폼 도구 집합 v143)

## 프로젝트 구조

```
NewDeleteDebug.sln
NewDeleteDebug/
├── myNew.h                     # new 매크로, 연산자 선언, AllocInfo 선언
├── myNew.cpp                   # new/delete 구현, 누수 및 불일치 로그 기록
├── NewDeleteDebug.cpp          # 누수가 일어나는 예제 코드
└── Alloc_20240928_060231.txt   # 예제 실행 로그
```

## 빌드 및 실행

> 현재 커밋의 `myNew.h`는 `operator new`의 파일명 인자를 `const wchar_t*`로 선언하지만, 매크로는 `__FILE__`(`const char*`)을 넘긴다. 그래서 그대로 빌드하면 컴파일 오류(C2665)가 난다. `myNew.h`의 매크로를 `#define new new(__FILEW__, __LINE__)`로 바꾸면 빌드된다.

1. Visual Studio 2022에서 `NewDeleteDebug.sln`을 연다.
2. 플랫폼은 `x86`을 선택한다. 누수 기록은 x64에서도 동작하지만, 짝이 맞지 않는 해제 탐지는 x86 기준으로 작성되었다.
   - 명령줄 빌드: `msbuild NewDeleteDebug.sln /p:Configuration=Debug /p:Platform=x86`
3. 실행하면 작업 디렉터리(Visual Studio에서 실행하면 프로젝트 폴더)에 `Alloc_YYYYMMDD_HHMMSS.txt` 로그가 생긴다.

다른 프로젝트에 붙일 때는 `myNew.h` / `myNew.cpp`를 추가하고, 각 소스에서 표준 헤더를 모두 include한 뒤 마지막에 `myNew.h`를 include한다.

## 개발 기간

- 2025-01-03 (저장소 등록, 단일 커밋)
- 함께 올린 실행 로그 파일명 기준 2024년 9월에 작성
- 1인 개인 프로젝트
