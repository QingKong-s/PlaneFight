#pragma warning (disable:4996)
#include "CWndMain.h"

constexpr PCWSTR c_pszWndClassMain = L"QK.WndClass.PlaneFight";
constexpr int
c_cxStd = 512,
c_cyStd = 768,
c_cxExplosion = 64,
c_cyExplosion = 64,
c_cExplosionUnitPerRow = 4,
c_cExplosionFrame = 16
;

#define BOOLNOT(x) (x) = !(x)

LRESULT CALLBACK CWndMain::WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CWndMain*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SIZE:
		p->OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MOUSEMOVE:
		p->m_ptCursor =
		{
			((float)GET_X_LPARAM(lParam) / p->m_fScaleFactor),
			((float)GET_Y_LPARAM(lParam) / p->m_fScaleFactor)
		};
		return 0;

	case WM_LBUTTONDOWN:
		p->m_bLBtnDown = TRUE;
		SetCapture(hWnd);
		return 0;

	case WM_LBUTTONUP:
		if (p->m_bLBtnDown)
		{
			p->m_bLBtnDown = FALSE;
			ReleaseCapture();
		}
		return 0;

	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
		{
			if (p->m_bGameOver)
				p->Reset();
			else
			{
				BOOLNOT(p->m_bGamePause);
				p->m_bPauseBKPrepared = FALSE;
			}
		}
	}
	return 0;

	case WM_NCCREATE:
		p = (CWndMain*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;

	case WM_CREATE:
		p->OnCreate(hWnd);
		return 0;

	case WM_DESTROY:
		p->OnDestroy();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void CWndMain::OnSize(int cx, int cy)
{
	m_cxClient = cx;
	m_cyClient = cy;

	m_fScaleFactor = (float)cx / (float)c_cxStd;

	m_pDC->SetTarget(NULL);// 移除引用
	m_pBmpBK->Release();

	HRESULT hr;
	if (FAILED(hr = m_pSwapChain->ResizeBuffers(0, cx, cy, DXGI_FORMAT_UNKNOWN, 0)))
	{
		assert(FALSE);
	}

	IDXGISurface1* pSurface;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface));
	if (!pSurface)
	{
		assert(FALSE);
	}

	D2D1_BITMAP_PROPERTIES1 D2dBmpProp
	{
		{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
		96,
		96,
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		NULL
	};
	if (FAILED(hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &m_pBmpBK)))
	{
		assert(FALSE);
	}
	pSurface->Release();

	D2dBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
	m_pBmpPauseBK->Release();
	m_pDC->CreateBitmap({ (UINT)cx,(UINT)cy }, NULL, 0, D2dBmpProp, &m_pBmpPauseBK);

	m_pDC->SetTransform(D2D1::Matrix3x2F::Scale(m_fScaleFactor, m_fScaleFactor));
}

void CWndMain::OnCreate(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	DXGI_SWAP_CHAIN_DESC1 DxgiSwapChainDesc
	{
		8,
		8,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		FALSE,
		{1, 0},
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		2,
		DXGI_SCALING_NONE,
		DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		DXGI_ALPHA_MODE_UNSPECIFIED,
		0
	};

	HRESULT hr;
	if (FAILED(hr = App->m_pDxgiFactory->CreateSwapChainForHwnd(
		App->m_pDxgiDevice,
		hWnd,
		&DxgiSwapChainDesc,
		NULL,
		NULL,
		&m_pSwapChain)))// 创建交换链
	{
		assert(FALSE);
	}

	if (FAILED(hr = App->m_pD2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pDC)))// 创建设备上下文
	{
		assert(FALSE);
	}

	IDXGISurface1* pSurface;
	if (FAILED(hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface))))// 取缓冲区
	{
		assert(FALSE);
	}

	D2D1_BITMAP_PROPERTIES1 D2dBmpProp
	{
		{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
		96,
		96,
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		NULL
	};
	if (FAILED(hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &m_pBmpBK)))// 创建位图自DXGI表面
	{
		assert(FALSE);
	}
	pSurface->Release();

	D2dBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
	m_pDC->CreateBitmap({ 8,8 }, NULL, 0, D2dBmpProp, &m_pBmpPauseBK);

	LoadTexture();

	m_pDC->CreateSolidColorBrush(D2D1::ColorF(0.85f, 0.85f, 0.85f, 0.8f), &m_pBrHealthBK);
	m_pDC->CreateSolidColorBrush(D2D1::ColorF(1.f, 0.f, 0.f, 0.8f), &m_pBrHealth);
	m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrText);

	App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		20.f, L"zh-cn", &m_pTfScore);
	App->m_pDwFactory->CreateTextFormat(L"微软雅黑", NULL,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		60.f, L"zh-cn", &m_pTfTitle);
}

void CWndMain::OnDestroy()
{
	for (auto& x : m_Texture)
	{
		x.pBitmap->Release();
		x.pWicBitmap->Release();
	}

	m_pBmpBK->Release();
	m_pBmpPauseBK->Release();
	m_pBrHealth->Release();
	m_pBrHealthBK->Release();
	m_pBrText->Release();
	m_pTfScore->Release();
	m_pTfTitle->Release();
	m_pDC->Release();
	m_pSwapChain->Release();
}

void CWndMain::LoadTexture()
{
	WCHAR szPath[MAX_PATH];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	PWSTR pszFileName = PathFindFileNameW(szPath);

	wcscpy(pszFileName, LR"(Res\Background1.png)");
	m_Texture[TIDX_BACKGND1] = Utility::LoadTexture(szPath, m_pDC);

	wcscpy(pszFileName, LR"(Res\Hero1.png)");
	m_Texture[TIDX_HERO1] = Utility::LoadTexture(szPath, m_pDC);

	wcscpy(pszFileName, LR"(Res\HeroBullet.png)");
	m_Texture[TIDX_HEROBULLET] = Utility::LoadTexture(szPath, m_pDC);

	wcscpy(pszFileName, LR"(Res\Enemy1.png)");
	m_Texture[TIDX_ENEMY1] = Utility::LoadTexture(szPath, m_pDC);

	wcscpy(pszFileName, LR"(Res\EnemyBullet.png)");
	m_Texture[TIDX_ENEMYBULLET] = Utility::LoadTexture(szPath, m_pDC);

	wcscpy(pszFileName, LR"(Res\Explosion.png)");
	m_Texture[TIDX_EXPLOSION] = Utility::LoadTexture(szPath, m_pDC);

	wcscpy(pszFileName, LR"(Res\Supply.png)");
	m_Texture[TIDX_SUPPLY] = Utility::LoadTexture(szPath, m_pDC);
}

void CWndMain::PreparePauseBK(int iType)
{
	D2D1_POINT_2U pt{};
	D2D1_RECT_U rcU{ 0,0,(UINT32)m_cxClient,(UINT32)m_cyClient };
	m_pBmpPauseBK->CopyFromBitmap(&pt, m_pBmpBK, &rcU);

	ID2D1SolidColorBrush* pBrush;
	m_pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.6f), &pBrush);

	m_pDC->SetTarget(m_pBmpPauseBK);
	m_pDC->BeginDraw();

	m_pDC->FillRectangle({ 0.f,0.f,c_cxStd,c_cyStd }, pBrush);
	IDWriteTextLayout* pTextLayout;
	D2D1_RECT_F rcF{};
	PCWSTR pszTitle, pszComment;
	if (iType == 0)
	{
		pszTitle = L"游戏暂停";
		pszComment = L"按Esc键继续";
	}
	else
	{
		pszTitle = L"游戏结束";
		pszComment = L"按Esc键重玩";
	}

	App->m_pDwFactory->CreateTextLayout(pszTitle, (UINT32)wcslen(pszTitle), m_pTfTitle, c_cxStd, c_cyStd / 3, &pTextLayout);
	pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
	m_pDC->DrawTextLayout({}, pTextLayout, m_pBrText);
	pTextLayout->Release();

	App->m_pDwFactory->CreateTextLayout(pszComment, (UINT32)wcslen(pszComment), m_pTfScore, c_cxStd, c_cyStd * 2 / 3, &pTextLayout);
	pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	m_pDC->DrawTextLayout({ 0.f,c_cyStd / 3 }, pTextLayout, m_pBrText);
	pTextLayout->Release();

	m_pDC->EndDraw();
	pBrush->Release();
}

#define RAND_INIT_ENEMY(x) x.RandInit((float)Utility::Rand(0, c_cxStd - m_Texture[TIDX_ENEMY1].cx), (float)-m_Texture[TIDX_ENEMY1].cy)

void CWndMain::Reset()
{
	m_uScore = 0u;
	m_uMiss = 0u;
	m_fHealth = 100.f;
	m_bLBtnDown = FALSE;
	m_bGameOver = FALSE;
	m_bGamePause = FALSE;
	m_bPauseBKPrepared = FALSE;
	for (auto& x : m_Enemy)
		RAND_INIT_ENEMY(x);
	m_EnemyBullet.clear();
	m_HeroBullet.clear();
	m_Explosion.clear();
	m_Supply.clear();
}

void CWndMain::EnemyOnDel(CEnemy& x)
{
	++m_uScore;
	m_Explosion.emplace_back(
		x.x + (m_Texture[TIDX_ENEMY1].cx - c_cxExplosion) / 2,
		x.y + (m_Texture[TIDX_ENEMY1].cy - c_cyExplosion) / 2,
		0);
	if (Utility::Rand(1, 10) == 10)
	{
		m_Supply.emplace_back(
			x.x + (m_Texture[TIDX_ENEMY1].cx - m_Texture[TIDX_SUPPLY].cx) / 2,
			x.y + (m_Texture[TIDX_ENEMY1].cy - m_Texture[TIDX_SUPPLY].cx) / 2,
			150);
	}
	RAND_INIT_ENEMY(x);
}


ATOM CWndMain::RegisterWndClass()
{
	WNDCLASSEX wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc_Main;
	wcex.hInstance = App->GetHInstance();
	wcex.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.lpszClassName = c_pszWndClassMain;
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

HWND CWndMain::Create(PCWSTR pszText, int x, int y, int cx, int cy, DWORD dwStyle, DWORD dwExStyle)
{
	m_hWnd = CreateWindowExW(dwExStyle, c_pszWndClassMain, pszText, dwStyle, 
		x, y, cx, cy, NULL, NULL, App->GetHInstance(), this);

	for (auto& x : m_Enemy)
		RAND_INIT_ENEMY(x);

	return m_hWnd;
}

void CWndMain::Tick()
{
	std::vector<size_t> NeedDelIndex{};
	NeedDelIndex.reserve(20u);
	D2D1_RECT_F rcF;
	if (m_bGameOver)
	{
		if (!m_bPauseBKPrepared)
		{
			PreparePauseBK(1);
			m_bPauseBKPrepared = TRUE;
		}
		m_pDC->SetTarget(m_pBmpBK);
		m_pDC->BeginDraw();
		rcF = { 0.f,0.f,c_cxStd,c_cyStd };
		m_pDC->DrawBitmap(m_pBmpPauseBK, rcF);
		m_pDC->EndDraw();
		m_pSwapChain->Present(0, 0);
		return;
	}
	else if (m_bGamePause)
	{
		if (!m_bPauseBKPrepared)
		{
			PreparePauseBK(0);
			m_bPauseBKPrepared = TRUE;
		}
		m_pDC->SetTarget(m_pBmpBK);
		m_pDC->BeginDraw();
		rcF = { 0.f,0.f,c_cxStd,c_cyStd };
		m_pDC->DrawBitmap(m_pBmpPauseBK, rcF);
		m_pDC->EndDraw();
		m_pSwapChain->Present(0, 0);
		return;
	}

	D2D1_RECT_F rcHero
	{
		m_ptCursor.x - m_Texture[TIDX_HERO1].cx / 2,
		m_ptCursor.y - m_Texture[TIDX_HERO1].cy / 2,
		m_ptCursor.x + m_Texture[TIDX_HERO1].cx / 2,
		m_ptCursor.y + m_Texture[TIDX_HERO1].cy / 2
	};

	ULONGLONG ullTime;
	m_pDC->SetTarget(m_pBmpBK);
	m_pDC->BeginDraw();
	// 画背景
	m_yBkImg += 2;
	if (m_yBkImg > c_cyStd)
		m_yBkImg = 0;
	rcF.left = 0.f;
	rcF.right = c_cxStd;
	rcF.top = (float)(m_yBkImg - c_cyStd);
	rcF.bottom = rcF.top + c_cyStd;
	m_pDC->DrawBitmap(m_Texture[TIDX_BACKGND1], &rcF);
	rcF.top = (float)m_yBkImg;
	rcF.bottom = rcF.top + c_cyStd;
	m_pDC->DrawBitmap(m_Texture[TIDX_BACKGND1], &rcF);

	// 画敌机
	ullTime = GetTickCount64();
	for (auto& x : m_Enemy)
	{
		x.y += x.fSpeed;
		if (x.y > m_cyClient)
		{
			RAND_INIT_ENEMY(x);
			++m_uMiss;
		}
		rcF.left = x.x;
		rcF.top = x.y;
		rcF.right = rcF.left + m_Texture[TIDX_ENEMY1].cx;
		rcF.bottom = rcF.top + m_Texture[TIDX_ENEMY1].cy;

		if (Utility::IsRectsIntersect(rcHero, rcF) && ullTime - m_ullLastCollisionTime > 80ull)
		{
			m_ullLastCollisionTime = ullTime;
			x.fHealth -= 15.f;
			m_fHealth -= 15.f;
			if (m_fHealth <= 0.f)
				m_bGameOver = TRUE;

			if (x.fHealth <= 0.f)
				EnemyOnDel(x);
		}

		m_pDC->DrawBitmap(m_Texture[TIDX_ENEMY1], &rcF);

		// 发射子弹
		if (ullTime - x.ullLastBulletTime > 140u)
		{
			x.ullLastBulletTime = ullTime;
			if (Utility::Rand(1, 30) == 30)
			{
				const float fSpeed = x.fSpeed + 1.f;
				m_EnemyBullet.push_back({ rcF.left + 17.f,rcF.top + 68.f,fSpeed });
				m_EnemyBullet.push_back({ rcF.left + 95.f,rcF.top + 68.f,fSpeed });
			}
		}

		// 画血条
		rcF.top = rcF.bottom - 2.f;
		const float fRight = rcF.right;
		rcF.right = rcF.left + (100.f - x.fHealth) / 100.f * m_Texture[TIDX_ENEMY1].cx;
		m_pDC->FillRectangle(&rcF, m_pBrHealthBK);

		rcF.left = rcF.right;
		rcF.right = fRight;
		m_pDC->FillRectangle(&rcF, m_pBrHealth);
	}

	// 画补给
	for (size_t i = 0; i < m_Supply.size(); ++i)
	{
		auto& x = m_Supply[i];
		rcF = { x.x,x.y,x.x + m_Texture[TIDX_SUPPLY].cx,x.y + m_Texture[TIDX_SUPPLY].cy };
		m_pDC->DrawBitmap(m_Texture[TIDX_SUPPLY], &rcF, x.uTick / 150.f);
		if (Utility::IsRectsIntersect(rcHero, rcF))
		{
			m_fHealth += 55.f;
			if (m_fHealth > 100.f)
				m_fHealth = 100.f;
			NeedDelIndex.push_back(i);
		}
		else
		{
			--x.uTick;
			if (x.uTick == 0u)
				NeedDelIndex.push_back(i);
		}
	}

	if (NeedDelIndex.size())
		for (auto it = NeedDelIndex.rbegin(); it < NeedDelIndex.rend(); ++it)
			m_Supply.erase(m_Supply.begin() + *it);
	NeedDelIndex.clear();

	// 画主角
	m_pDC->DrawBitmap(m_Texture[TIDX_HERO1], &rcHero);

	// 画主角子弹
	// 发射子弹
	if (m_bLBtnDown)
	{
		ullTime = GetTickCount64();
		if (ullTime - m_ullLastBulletTime > 140ull)
		{
			constexpr float fSpeed = 8.f;
			m_HeroBullet.push_back({ rcHero.left + 12.f,rcHero.top + 9.f,fSpeed });
			m_HeroBullet.push_back({ rcHero.left + 36.f,rcHero.top + 0.f,fSpeed });
			m_HeroBullet.push_back({ rcHero.left + 64.f,rcHero.top + 0.f,fSpeed });
			m_HeroBullet.push_back({ rcHero.left + 88.f,rcHero.top + 9.f,fSpeed });
			m_ullLastBulletTime = ullTime;
		}
	}
	// 子弹飞行与碰撞
	for (size_t i = 0; i < m_HeroBullet.size(); ++i)
	{
		auto& x = m_HeroBullet[i];
		x.y -= x.fSpeed;

		rcF.left = x.x;
		rcF.top = x.y;
		rcF.right = rcF.left + m_Texture[TIDX_HEROBULLET].cx;
		rcF.bottom = rcF.top + m_Texture[TIDX_HEROBULLET].cy;

		if (x.y < 0)
			NeedDelIndex.push_back(i);
		else
			for (auto& y : m_Enemy)
			{
				if (Utility::IsRectsIntersect(
					{ y.x,y.y,y.x + m_Texture[TIDX_ENEMY1].cx,y.y + m_Texture[TIDX_ENEMY1].cy },
					rcF))
				{
					y.fHealth -= 15.f;
					if (y.fHealth <= 0)
						EnemyOnDel(y);
					NeedDelIndex.push_back(i);
					break;
				}
			}

		m_pDC->DrawBitmap(m_Texture[TIDX_HEROBULLET], &rcF);
	}

	if (NeedDelIndex.size())
		for (auto it = NeedDelIndex.rbegin(); it < NeedDelIndex.rend(); ++it)
			m_HeroBullet.erase(m_HeroBullet.begin() + *it);
	NeedDelIndex.clear();
	// 画敌人子弹
	for (size_t i = 0; i < m_EnemyBullet.size(); ++i)
	{
		auto& x = m_EnemyBullet[i];
		x.y += x.fSpeed;

		rcF.left = x.x;
		rcF.top = x.y;
		rcF.right = rcF.left + m_Texture[TIDX_ENEMYBULLET].cx;
		rcF.bottom = rcF.top + m_Texture[TIDX_ENEMYBULLET].cy;

		if (x.y > c_cyStd)
			NeedDelIndex.push_back(i);
		else if (Utility::IsRectsIntersect(rcHero, rcF))
		{
			m_fHealth -= 15.f;
			if (m_fHealth <= 0.f)
			{
				m_bGameOver = TRUE;
			}
			NeedDelIndex.push_back(i);
		}

		m_pDC->DrawBitmap(m_Texture[TIDX_ENEMYBULLET], &rcF);
	}

	if (NeedDelIndex.size())
		for (auto it = NeedDelIndex.rbegin(); it < NeedDelIndex.rend(); ++it)
			m_EnemyBullet.erase(m_EnemyBullet.begin() + *it);
	NeedDelIndex.clear();
	// 画爆炸效果
	D2D1_RECT_F rcF2;

	for (size_t i = 0; i < m_Explosion.size(); ++i)
	{
		auto& x = m_Explosion[i];
		rcF = { x.x,x.y,x.x + c_cxExplosion,x.y + c_cyExplosion };
		rcF2.left = (FLOAT)((x.idxExplosion % c_cExplosionUnitPerRow) * c_cxExplosion);
		rcF2.top = (FLOAT)((x.idxExplosion / c_cExplosionUnitPerRow) * c_cyExplosion);
		rcF2.right = rcF2.left + c_cxExplosion;
		rcF2.bottom = rcF2.top + c_cyExplosion;
		m_pDC->DrawBitmap(m_Texture[TIDX_EXPLOSION], &rcF, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &rcF2);
		++x.idxExplosion;
		if (x.idxExplosion == c_cExplosionFrame)
			NeedDelIndex.push_back(i);
	}

	if (NeedDelIndex.size())
		for (auto it = NeedDelIndex.rbegin(); it < NeedDelIndex.rend(); ++it)
			m_Explosion.erase(m_Explosion.begin() + *it);
	// 画血条
	constexpr float cxHealth = 80.f;
	rcF.top = 4.f;
	rcF.bottom = rcF.top + 10.f;
	rcF.left = 6.f;

	float fPercent = (100.f - m_fHealth) / 100.f;
	if (fPercent < 0.f)
		fPercent = 0.f;
	else if (fPercent > 1.f)
		fPercent = 1.f;
	rcF.right = rcF.left + fPercent * cxHealth;
	m_pDC->FillRectangle(&rcF, m_pBrHealthBK);

	rcF.left = rcF.right;
	rcF.right = 6.f + cxHealth;
	m_pDC->FillRectangle(&rcF, m_pBrHealth);
	// 画文本
	WCHAR sz[36];
	int cch = swprintf(sz, L"得分：%u\n遗漏：%u", m_uScore, m_uMiss);
	const float yText = rcF.bottom + 6.f;
	rcF = { 6,yText,c_cxStd,c_cyStd };
	m_pDC->DrawTextW(sz, cch, m_pTfScore, &rcF, m_pBrText);
	// 
	m_pDC->EndDraw();
	// 上屏
	m_pSwapChain->Present(0, 0);
}
