#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wincodec.h>
#include <CommCtrl.h>
#include <Shlwapi.h>

#include <vector>
#include <string>
#include <assert.h>


class CApp
{
public:
	ID2D1Factory1*		m_pD2dFactory		= NULL;
	IDWriteFactory*		m_pDwFactory		= NULL;
	IWICImagingFactory* m_pWicFactory		= NULL;

	ID2D1Device*		m_pD2dDevice		= NULL;

	IDXGIDevice1*		m_pDxgiDevice		= NULL;
	IDXGIFactory2*		m_pDxgiFactory		= NULL;
private:
	HINSTANCE			m_hInstance			= NULL;
	
public:
	~CApp()
	{
		m_pD2dFactory->Release();
		m_pDwFactory->Release();
		m_pWicFactory->Release();
		m_pD2dDevice->Release();
		m_pDxgiDevice->Release();
		m_pDxgiFactory->Release();
	}

	void Init(HINSTANCE hInstance);

	inline HINSTANCE GetHInstance() const { return m_hInstance; }
};

extern CApp* App;