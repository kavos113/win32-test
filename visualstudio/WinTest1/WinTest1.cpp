#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <list>
#include <memory>
#pragma comment(lib, "d2d1")

#include "BaseWindow.h"
#include "Resource.h"

class DPIScale
{
	static float scale;

public:
	static void Initialize(ID2D1Factory* pFactory)
	{
		HWND hwnd = GetDesktopWindow();
		UINT dpi = GetDpiForWindow(hwnd);
		scale = dpi / 96.0f;
	}

	template <typename T>
	static D2D1_POINT_2F PixelsToDips(T x, T y)
	{
		return D2D1::Point2F(static_cast<float>(x) / scale, static_cast<float>(y) / scale);
	}

	static float PixelsToDips(int pixels)
	{
		return static_cast<float>(pixels) / scale;
	}
};

float DPIScale::scale = 1.0f;

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

struct MyEllipse
{
	D2D1_ELLIPSE ellipse;
	D2D1_COLOR_F color;

	void Draw(ID2D1RenderTarget* pRT, ID2D1SolidColorBrush* pBrush)
	{
		pBrush->SetColor(color);
		pRT->FillEllipse(ellipse, pBrush);
		pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		pRT->DrawEllipse(ellipse, pBrush, 1.0f);
	}

	BOOL HitTest(float x, float y)
	{
		const float a = ellipse.radiusX;
		const float b = ellipse.radiusY;
		const float x1 = x - ellipse.point.x;
		const float y1 = x - ellipse.point.y;
		const float d = (x1 * x1) / (a * a) + (y1 * y1) / (b * b);
		return d <= 1.0f;
	}
};

D2D1::ColorF::Enum colors[] = {
	D2D1::ColorF::Green,
	D2D1::ColorF::Blue,
	D2D1::ColorF::Yellow,
	D2D1::ColorF::Purple,
	D2D1::ColorF::Cyan,
	D2D1::ColorF::Magenta,
	D2D1::ColorF::Lime,
	D2D1::ColorF::Teal,
	D2D1::ColorF::Orange,
	D2D1::ColorF::Brown,
	D2D1::ColorF::Pink,
	D2D1::ColorF::Olive,
	D2D1::ColorF::Navy,
	D2D1::ColorF::White,
	D2D1::ColorF::Black
};

class MainWindow : public BaseWindow<MainWindow>
{
	enum Mode
	{
		DrawMode,
		SelectMode,
		DragMode
	};

	HCURSOR hCursor;

	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pBrush;
	D2D1_POINT_2F           ptMouse;

	Mode mode;
	size_t nextColor;

	std::list<std::shared_ptr<MyEllipse>> ellipses;
	std::list<std::shared_ptr<MyEllipse>>::iterator selection;

	std::shared_ptr<MyEllipse> Selection()
	{
		if (selection == ellipses.end())
		{
			return nullptr;
		}
		else
		{
			return (*selection);
		}
	}

	void ClearSelection()
	{
		selection = ellipses.end();
	}

	HRESULT InsertEllipse(float x, float y);

	BOOL HitTest(float x, float y);
	void SetMode(Mode m);
	void MoveSelection(float x, float y);
	HRESULT CreateGraphicsResources();
	void    DiscardGraphicsResources();
	void    OnPaint();
	void    Resize();
	void    OnLButtonDown(int pixelX, int pixelY, DWORD flags);
	void    OnLButtonUp();
	void    OnMouseMove(int pixelX, int pixelY, DWORD flags);
	void    OnKeyDown(UINT vkey);

public:

	MainWindow() :
		pFactory(NULL),
		pRenderTarget(NULL),
		pBrush(NULL),
		ptMouse(D2D1::Point2F()),
		nextColor(0),
		selection(ellipses.end())
	{

	}

	PCWSTR  ClassName() const
	{
		return L"Direct2D sample class";
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


HRESULT MainWindow::CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (pRenderTarget == nullptr)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&pRenderTarget
		);

		if (SUCCEEDED(hr))
		{
			const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 0, 0);
			hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
		}
	}

	return hr;
}

void MainWindow::DiscardGraphicsResources()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
	HRESULT hr = CreateGraphicsResources();
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);

		pRenderTarget->BeginDraw();

		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

		for (auto i = ellipses.begin(); i != ellipses.end(); ++i)
		{
			(*i)->Draw(pRenderTarget, pBrush);
		}

		if (Selection())
		{
			pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
			pRenderTarget->DrawEllipse(Selection()->ellipse, pBrush, 2.0f);
		}

		hr = pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}
		EndPaint(m_hwnd, &ps);
	}
}

void MainWindow::Resize()
{
	if (pRenderTarget != nullptr)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		pRenderTarget->Resize(size);

		InvalidateRect(m_hwnd, NULL, FALSE);
	}
}

void MainWindow::OnLButtonDown(int pixelX, int pixelY, DWORD flags)
{
	const float dipX = DPIScale::PixelsToDips(pixelX);
	const float dipY = DPIScale::PixelsToDips(pixelY);

	if (mode == DrawMode)
	{
		POINT pt = { pixelX, pixelY };

		if (DragDetect(m_hwnd, pt))
		{
			SetCapture(m_hwnd);

			InsertEllipse(dipX, dipY);
		}
		else
		{
			ClearSelection();

			if (HitTest(dipX, dipY))
			{
				SetCapture(m_hwnd);

				ptMouse = Selection()->ellipse.point;
				ptMouse.x -= dipX;
				ptMouse.y -= dipY;

				SetMode(DragMode);
			}
		}

		InvalidateRect(m_hwnd, nullptr, FALSE);
	}
}

void MainWindow::OnMouseMove(int pixelX, int pixelY, DWORD flags)
{
	const float dipX = DPIScale::PixelsToDips(pixelX);
	const float dipY = DPIScale::PixelsToDips(pixelY);

	if ((flags & MK_LBUTTON) && Selection())
	{
		if (mode == DrawMode)
		{
			const float width = (dipX - ptMouse.x) / 2;
			const float height = (dipY - ptMouse.y) / 2;
			const float x = ptMouse.x + width;
			const float y = ptMouse.y + height;

			Selection()->ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), width, height);
		}
		else if (mode == DragMode)
		{
			Selection()->ellipse.point.x = dipX + ptMouse.x;
			Selection()->ellipse.point.y = dipY + ptMouse.y;
		}

		InvalidateRect(m_hwnd, nullptr, FALSE);
	}
}

void MainWindow::OnLButtonUp()
{
	if ((mode == DrawMode) && Selection())
	{
		ClearSelection();
		InvalidateRect(m_hwnd, nullptr, FALSE);
	}
	else if (mode == DragMode)
	{
		SetMode(SelectMode);
	}
	ReleaseCapture();
}

void MainWindow::OnKeyDown(UINT vkey)
{
	switch (vkey)
	{
	case VK_BACK:
	case VK_DELETE:
		if ((mode == SelectMode) && Selection())
		{
			ellipses.erase(selection);
			ClearSelection();
			SetMode(SelectMode);
			InvalidateRect(m_hwnd, nullptr, FALSE);
		}
		break;

	case VK_LEFT:
		MoveSelection(-1, 0);
		break;

	case VK_RIGHT:
		MoveSelection(1, 0);
		break;

	case VK_UP:
		MoveSelection(0, -1);
		break;

	case VK_DOWN:
		MoveSelection(0, 1);
		break;
	}
}

HRESULT MainWindow::InsertEllipse(float x, float y)
{
	try
	{
		selection = ellipses.insert(
			ellipses.end(),
			std::shared_ptr<MyEllipse>(new MyEllipse())
		);

		ptMouse = D2D1::Point2F(x, y);
		Selection()->ellipse.point = ptMouse;
		Selection()->ellipse.radiusX = 2.0f;
		Selection()->ellipse.radiusY = 2.0f;
		Selection()->color = D2D1::ColorF(colors[nextColor]);

		nextColor = (nextColor + 1) % ARRAYSIZE(colors);
	}
	catch (std::bad_alloc)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

BOOL MainWindow::HitTest(float x, float y)
{
	for (auto i = ellipses.rbegin(); i != ellipses.rend(); ++i)
	{
		if ((*i)->HitTest(x, y))
		{
			selection = (++i).base();
			return TRUE;
		}
	}
	return FALSE;
}

void MainWindow::MoveSelection(float x, float y)
{
	if ((mode == SelectMode) && Selection())
	{
		Selection()->ellipse.point.x += x;
		Selection()->ellipse.point.y += y;
		InvalidateRect(m_hwnd, nullptr, FALSE);
	}
}

void MainWindow::SetMode(Mode m)
{
	mode = m;

	LPWSTR cursor = IDC_ARROW;
	switch (mode)
	{
	case DrawMode:
		cursor = IDC_CROSS;
		break;

	case SelectMode:
		cursor = IDC_ARROW;
		break;

	case DragMode:
		cursor = IDC_SIZEALL;
		break;
	}

	hCursor = LoadCursor(nullptr, cursor);
	SetCursor(hCursor);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	MainWindow window;

	if (!window.Create(L"Direct2D sample", WS_OVERLAPPEDWINDOW))
	{
		return 0;
	}

	ShowWindow(window.Window(), nCmdShow);

	// run message loop

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	wchar_t msg[32];
	switch (uMsg)
	{


	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
		{
			return -1;
		}
		DPIScale::Initialize(pFactory);
		SetMode(DrawMode);
		return 0;

	case WM_DESTROY:
		DiscardGraphicsResources();
		SafeRelease(&pFactory);
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_SIZE:
		Resize();
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		return 0;

	case WM_LBUTTONUP:
		OnLButtonUp();
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		return 0;

	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT)
		{
			SetCursor(hCursor);
			return TRUE;
		}
		break;

	case WM_KEYDOWN:
		OnKeyDown((UINT)wParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_DRAW_MODE:
			SetMode(DrawMode);
			break;

		case ID_SELECT_MODE:
			SetMode(SelectMode);
			break;

		case ID_TOGGLE_MODE:
			if (mode == DrawMode)
			{
				SetMode(SelectMode);
			}
			else
			{
				SetMode(DrawMode);
			}
		}
	}


	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

