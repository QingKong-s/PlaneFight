#include "CApp.h"

CApp* App;

void CApp::Init(HINSTANCE hInstance)
{
	HRESULT hr;
#ifndef NDEBUG
	D2D1_FACTORY_OPTIONS D2DFactoryOptions;
	D2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &D2DFactoryOptions, (void**)&m_pD2dFactory);
#else
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, IID_PPV_ARGS(&m_pD2dFactory));
#endif // !NDEBUG
	if (FAILED(hr))
	{
	}
	//////////////创建DWrite工厂
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pDwFactory);
	if (FAILED(hr))
	{
	}
	//////////////创建DXGI工厂
	ID3D11Device* pD3DDevice;
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
		| D3D11_CREATE_DEVICE_DEBUG
#endif // !NDEBUG
		, NULL, 0, D3D11_SDK_VERSION, &pD3DDevice, NULL, NULL);
	if (FAILED(hr))
	{
	}
	pD3DDevice->QueryInterface(IID_PPV_ARGS(&m_pDxgiDevice));
	pD3DDevice->Release();

	IDXGIAdapter* pDXGIAdapter;
	m_pDxgiDevice->GetAdapter(&pDXGIAdapter);

	pDXGIAdapter->GetParent(IID_PPV_ARGS(&m_pDxgiFactory));
	pDXGIAdapter->Release();
	//////////////创建DXGI设备
	hr = m_pD2dFactory->CreateDevice(m_pDxgiDevice, &m_pD2dDevice);
	if (FAILED(hr))
	{
	}
	//////////////创建WIC工厂
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWicFactory));
	if (FAILED(hr))
	{
	}
	
}