#include "d3dUtil.h"
#include <Windows.h>
#include <string>
#include <sstream>

using namespace std;

wchar_t* stringToWchar(string in, wchar_t* out) {
	const size_t origsize = strlen(in.c_str()) + 1;
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, out, origsize, in.c_str(), _TRUNCATE);

	return out;
}

//Returns the amount of milliseconds elapsed since the UNIX epoch.
__int64 getTimeMs64() {
	/* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

	/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
	* to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	__int64 ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 100; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

	return ret;
}

//Converts string to float without data loss
float stringToFloat(string& input) {
	istringstream is(input);
	float doubleNumber;
	is >> doubleNumber;
	return doubleNumber;
}

bool convertLPWtoString(string& s, const LPWSTR pw) {
	UINT codepage = CP_ACP;
    bool res = false;
    char* p = 0;
    int bsz;
    bsz = WideCharToMultiByte(codepage, 0, pw, -1, 0, 0, 0, 0);
    if (bsz > 0) {
        p = new char[bsz];
        int rc = WideCharToMultiByte(codepage, 0, pw, -1, p, bsz, 0, 0);
        if (rc != 0) {
            p[bsz-1] = 0;
            s = p;
            res = true;
        }
    }
    delete [] p;
    return res;
}