#include "Utility.h"

UTILITY_NAMESPACE_BEGIN
Texture LoadTexture(PCWSTR pszFile, ID2D1DeviceContext* pDC)
{
	auto pFactory = App->m_pWicFactory;
	IWICBitmap* pBitmap;
	IWICBitmapDecoder* pDecoder;
	IWICBitmapFrameDecode* pFrameDecoder;
	IWICFormatConverter* pConverter;

	HRESULT hr = pFactory->CreateDecoderFromFilename(pszFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);

	pFactory->CreateFormatConverter(&pConverter);

	pDecoder->GetFrame(0, &pFrameDecoder);

	pConverter->Initialize(pFrameDecoder, GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);

	pFactory->CreateBitmapFromSource(pConverter, WICBitmapNoCache, &pBitmap);

	pFrameDecoder->Release();
	pDecoder->Release();
	pConverter->Release();

	ID2D1Bitmap1* pD2dBitmap;
	pDC->CreateBitmapFromWicBitmap(pBitmap, &pD2dBitmap);
	UINT cx, cy;
	pBitmap->GetSize(&cx, &cy);
	return { pD2dBitmap,pBitmap,(int)cx,(int)cy };
}

void Sleep(int iTime)
{
	HANDLE hTimer = CreateWaitableTimerW(NULL, FALSE, NULL);

	LARGE_INTEGER llDueTime;
	llDueTime.QuadPart = -10 * iTime * 1000LL;

	SetWaitableTimer(hTimer, &llDueTime, 0, NULL, NULL, 0);
	MSG msg;
	DWORD dwRet;
	while ((dwRet = MsgWaitForMultipleObjects(1, &hTimer, FALSE, INFINITE, QS_ALLEVENTS)) != WAIT_OBJECT_0)
	{
		if (dwRet == WAIT_OBJECT_0 + 1)
		{
			while (PeekMessageW(&msg, NULL, 0u, 0u, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					PostQuitMessage((int)msg.wParam);
					return;
				}
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		else
			break;
	}

	CancelWaitableTimer(hTimer);
	CloseHandle(hTimer);
}
UTILITY_NAMESPACE_END