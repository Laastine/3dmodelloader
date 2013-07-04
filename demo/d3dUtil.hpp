#ifndef D3DUTIL_H
#define D3DUTIL_H

#pragma warning(disable: 4005)	//Ignore warnings from depracated D3D libraries

#if defined(DEBUG) || defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <d3dx10.h>
#include <dxerr.h>
#include <cassert>
#include <string>

using namespace std;

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x) {                                                          \
        HRESULT hr = (x);                                      \
        if(FAILED(hr))                                         \
        {                                                      \
            DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
        }                                                      \
    }
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif

#define ReleaseCOM(x) { if(x){ x->Release();x = 0; } }

wchar_t* stringToWchar(string in, wchar_t* out);

__int64 getTimeMs64();

float stringToFloat(string& input);

bool convertLPWtoString(string& s, const LPWSTR pw);

template<typename T>
D3DX10INLINE T Min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
D3DX10INLINE T Max(const T& a, const T& b) {
    return a > b ? a : b;
}

template<typename T>
D3DX10INLINE T Lerp(const T& a, const T& b, float t) {
    return a + (b-a)*t;
}

template<typename T>
D3DX10INLINE T Clamp(const T& x, const T& low, const T& high) {
    return x < low ? low : (x > high ? high : x);
}

const float INFINITY = FLT_MAX;
const float PI       = 3.14159265358979323f;
const float MATH_EPS = 0.0001f;

#endif // D3DUTIL_H