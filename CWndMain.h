#pragma once
#include "CApp.h"
#include "Utility.h"

struct CBullet
{
	float x;
	float y;
	float fSpeed;
};

struct CEnemy
{
	float x;
	float y;
	float fHealth;
	float fSpeed;
	ULONGLONG ullLastBulletTime;

	CEnemy() = default;

	CEnemy(float x, float y)
	{
		RandInit(x, y);
	}

	void RandInit(float x_, float y_)
	{
		x = x_;
		y = y_;
		fHealth = 100.f;
		fSpeed = (float)Utility::Rand(6, 10);
		ullLastBulletTime = 0ull;
	}
};

struct CExplosion
{
	float x;
	float y;
	int idxExplosion = 0;
};

struct CSupply
{
	float x;
	float y;
	UINT uTick;
};

class CWndMain
{
private:
	enum
	{
		TIDX_BACKGND1,
		TIDX_HERO1,
		TIDX_HEROBULLET,
		TIDX_ENEMY1,
		TIDX_ENEMYBULLET,
		TIDX_EXPLOSION,
		TIDX_SUPPLY,

		TIDX_MAX,
	};

	HWND m_hWnd = NULL;

	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBmpBK = NULL;
	ID2D1Bitmap1* m_pBmpPauseBK = NULL;
	ID2D1SolidColorBrush* m_pBrHealth = NULL;
	ID2D1SolidColorBrush* m_pBrHealthBK = NULL;
	ID2D1SolidColorBrush* m_pBrText = NULL;
	IDWriteTextFormat* m_pTfScore = NULL;
	IDWriteTextFormat* m_pTfTitle = NULL;

	Utility::Texture m_Texture[TIDX_MAX]{};

	int m_cxClient = 0;
	int m_cyClient = 0;

	int m_yBkImg = 0;

	D2D1_POINT_2F m_ptCursor{};

	float m_fScaleFactor = 1.f;
	
	std::vector<CBullet> m_HeroBullet{};
	std::vector<CBullet> m_EnemyBullet{};
	std::vector<CEnemy> m_Enemy{ 6 };
	std::vector<CExplosion> m_Explosion{};
	std::vector<CSupply> m_Supply{};

	BOOL m_bLBtnDown = FALSE;

	ULONGLONG m_ullLastBulletTime = 0ull;
	ULONGLONG m_ullLastCollisionTime = 0ull;

	UINT m_uScore = 0u;
	UINT m_uMiss = 0u;

	float m_fHealth = 100.f;

	BOOL m_bGameOver = FALSE;
	BOOL m_bGamePause = FALSE;
	BOOL m_bPauseBKPrepared = FALSE;
private:
	static LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnSize(int cx, int cy);

	void OnCreate(HWND hWnd);

	void OnDestroy();

	void LoadTexture();

	void PreparePauseBK(int iType);

	void Reset();

	void EnemyOnDel(CEnemy& x);
public:
	CWndMain()
	{
		m_HeroBullet.reserve(32u);
	}

	void Tick();

	static ATOM RegisterWndClass();

	HWND GetHwnd() const { return m_hWnd; }

	HWND Create(PCWSTR pszText, int x, int y, int cx, int cy, DWORD dwStyle, DWORD dwExStyle);

	void GameOver();

	void Pause();
};