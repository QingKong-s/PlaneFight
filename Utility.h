#pragma once
#include "CApp.h"

#define UTILITY_NAMESPACE_BEGIN namespace Utility {
#define UTILITY_NAMESPACE_END }

UTILITY_NAMESPACE_BEGIN
struct Texture
{
	ID2D1Bitmap1* pBitmap;
	IWICBitmap* pWicBitmap;
	int cx;
	int cy;

	operator ID2D1Bitmap1* ()
	{
		return pBitmap;
	}
};

Texture LoadTexture(PCWSTR pszFile, ID2D1DeviceContext* pDC);

void Sleep(int iTime);

inline int Rand(int iMin, int iMax)
{
	return rand() % ((LONGLONG)iMax - (LONGLONG)iMin + 1ll) + (LONGLONG)iMin;
}

inline BOOL IsRectsIntersect(const D2D1_RECT_F& rc1, const D2D1_RECT_F& rc2)
{
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min// ÄÔ²ÐWinSDK
	return
		std::max(rc1.left, rc2.left) < std::min(rc1.right, rc2.right) &&
		std::max(rc1.top, rc2.top) < std::min(rc1.bottom, rc2.bottom);
#pragma pop_macro("max")
#pragma pop_macro("min")
}
UTILITY_NAMESPACE_END