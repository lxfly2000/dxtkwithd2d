//微软写的那个Game.h太他妈的混乱了，没用的东西一大堆，于是我就写了个轻量化的GameLite类。
//https://github.com/Microsoft/DirectXTK/wiki/Getting-Started

#include "pch.h"
#include "GameLite.h"
#include "Win7PlatformUpdateCheck.h"

#define PICPOS_FORMAT L"%4d,%4d"

GameLite::GameLite():isFullscreen(false),circleBrush(),circle(),cursorposText(),d2ddevice(),d2drendertarget(),d3ddevice(),
fcounter(0),hwndWindow(),hwndWindowInfo(),hwndWindowPlacement(),hwndWindowSize(),outlineBrush(),picpostext(),pic()
{
	screenSize.left = screenSize.top = 0;
	screenSize.right = GAME_WINDOW_WIDTH;
	screenSize.bottom = GAME_WINDOW_HEIGHT;
	isPausedFromSystem = false;
	picToScrRect = {};
}

void GameLite::Init(ID3D11Device *pdevice, ID3D11DeviceContext *pcontext, ID2D1Factory *pd2ddevice, HWND hwnd)
{
	hwndWindow = hwnd;
	d3ddevice = pdevice;
	d2ddevice = pd2ddevice;
	spriteBatch = std::make_unique<DirectX::SpriteBatch>(pcontext);
	DXThrowIfFailed(LoadTextureFromFile(d3ddevice,L"C:\\Program Files\\Windows Media Player\\Media Renderer\\DMR_120.png",
		&pic, (int*)&picToScrRect.right, (int*)&picToScrRect.bottom));
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();
	mouse->SetWindow(hwnd);
	DXThrowIfFailed(LoadFontFromSystem(d3ddevice,ssfont, 1024, 1024, L"宋体", 48, D2D1::ColorF(D2D1::ColorF::White),
		DWRITE_FONT_WEIGHT_REGULAR));
	wsprintf(picpostext, PICPOS_FORMAT, picToScrRect.left, picToScrRect.top);
	textcenterpos = ssfont->MeasureString(picpostext);
	textcenterpos.x /= 2;
	mdBlock = DirectX::GeometricPrimitive::CreateBox(pcontext, DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
	matWorld = DirectX::SimpleMath::Matrix::Identity;
	matView = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(0, 2.0f, -2.5f),
		DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
	fcounter = 0;

	DXThrowIfFailed(CreateDWTextFormat(textformat, L"宋体", DWRITE_FONT_WEIGHT_NORMAL, 48.0f));
	DXThrowIfFailed(CreateDWFontFace(fontface, textformat.Get()));
	DXThrowIfFailed(CreateD2DGeometryFromText(btgeometry, d2ddevice, fontface.Get(), textformat->GetFontSize(), L"Hello!", 6));
	DXThrowIfFailed(textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	//textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	circle.radiusX = circle.radiusY = 120.0f;
}

void GameLite::Uninit()
{
	UninitD2D();
	pic->Release();
}

void GameLite::InitD2D(IDXGISwapChain *pswchain)
{
	IDXGISurface *surface;
	DXThrowIfFailed(pswchain->GetBuffer(0, IID_PPV_ARGS(&surface)));
	try
	{
		DXThrowIfFailedHWND(d2ddevice->CreateDxgiSurfaceRenderTarget(surface, D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), &d2drendertarget),hwndWindow);
	}
	catch (HRESULT hr)
	{
		if (Win7PlatformUpdateCheck(hwndWindow, hr))
			ExitProcess(hr);
		else
			throw hr;
	}
	surface->Release();

	DXThrowIfFailed(d2drendertarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &circleBrush));
	DXThrowIfFailed(d2drendertarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OrangeRed), &outlineBrush));
	DXThrowIfFailed(CreateD2DLinearGradientBrush(bluetowhiteBrush, d2drendertarget, 0, -50, 0, 0,
		D2D1::ColorF(D2D1::ColorF::Blue), D2D1::ColorF(D2D1::ColorF::White)));
}

void GameLite::UninitD2D()
{
	circleBrush->Release();
	outlineBrush->Release();
	d2drendertarget->Release();
}

void GameLite::Update(float elapsedTime)
{
	if (isPausedFromSystem)
		return;
	matWorld = DirectX::SimpleMath::Matrix::CreateRotationY(0.005f*fcounter*DirectX::XM_PI);
	CreateHPCircle(hpcircle, circle.radiusX, 1.0f - (fcounter % 200) / 200.0f);
	auto key = keyboard->GetState();
	if (key.Up)
	{
		picToScrRect.top -= 1;
		picToScrRect.bottom -= 1;
		wsprintf(picpostext, PICPOS_FORMAT, picToScrRect.left, picToScrRect.top);
	}
	if (key.Down)
	{
		picToScrRect.top += 1;
		picToScrRect.bottom += 1;
		wsprintf(picpostext, PICPOS_FORMAT, picToScrRect.left, picToScrRect.top);
	}
	if (key.Left)
	{
		picToScrRect.left -= 1;
		picToScrRect.right -= 1;
		wsprintf(picpostext, PICPOS_FORMAT, picToScrRect.left, picToScrRect.top);
	}
	if (key.Right)
	{
		picToScrRect.left += 1;
		picToScrRect.right += 1;
		wsprintf(picpostext, PICPOS_FORMAT, picToScrRect.left, picToScrRect.top);
	}
	if (km.IsOnKeyDown(keyboard.get(), DirectX::Keyboard::F11))
	{
		ResetHWNDWindowSize();
	}
	if (km.IsOnKeyDown(keyboard.get(), DirectX::Keyboard::Enter))
	{
		auto s = keyboard.get()->GetState();
		if (s.LeftAlt || s.RightAlt)
			ToggleFullscreen(!isFullscreen);
	}
	
	circle.point.x = (float)MapMousePointToScreenX(mouse.get());
	circle.point.y = (float)MapMousePointToScreenY(mouse.get());
	wsprintf(cursorposText, PICPOS_FORMAT, (int)circle.point.x, (int)circle.point.y);

	fcounter++;
}

void GameLite::Draw()
{
	//DXTK画图操作
	spriteBatch->Begin();
	mdBlock->Draw(matWorld, matView, matProjection);
	spriteBatch->Draw(pic, picToScrRect);
	ssfont->DrawString(spriteBatch.get(), picpostext, textpos, DirectX::Colors::White, 0.0f, textcenterpos);
	spriteBatch->End();
	//D2D画图操作
	d2drendertarget->BeginDraw();
	//Clear操作已由DXTK完成故不再需要调用ID2D1RenderTarget::Clear
	DrawHPCircle(hpcircle.Get(), circle.point.x, circle.point.y, 6.0f, 2.0f, circleBrush, outlineBrush);
	//D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT：绘制彩色文字，要求系统为Win8.1/10，在Win7中会使D2D区域无法显示
	//D2D1_DRAW_TEXT_OPTIONS_NONE：不使用高级绘制选项
	d2drendertarget->DrawText(cursorposText, lstrlen(cursorposText), textformat.Get(),
		D2D1::RectF((float)screenSize.left, (float)screenSize.top, (float)screenSize.right,
		(float)screenSize.bottom), circleBrush);
	D2DDrawGeometryWithOutline(d2drendertarget, btgeometry.Get(), 0, (float)screenSize.bottom,
		bluetowhiteBrush.Get(), outlineBrush, 1);
	d2drendertarget->EndDraw();
}

void GameLite::PauseFromSystem()
{
	if (isPausedFromSystem)return;
	//TODO;
	isPausedFromSystem = true;
}

void GameLite::ResumeFromSystem()
{
	if (!isPausedFromSystem)return;
	//TODO.
	isPausedFromSystem = false;
}

void GameLite::OnUpdateResProp(int _w, int _h, IDXGISwapChain *pswchain)
{
	screenSize.right = _w;
	screenSize.bottom = _h;
	textpos.x = (float)(screenSize.right / 2);//不要有小数，否则绘制出的文字会变得模糊。
	textpos.y = (float)screenSize.bottom;
	matProjection = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.0f,
		(float)GetScreenWidth() / GetScreenHeight(), 0.1f, 10.0f);
	InitD2D(pswchain);
}

void GameLite::OnBeforeResizeWindow()
{
	UninitD2D();
}

void GameLite::OnNewHWNDWindowSize(int _w, int _h)
{
	hwndWindowSize.left = 0;
	hwndWindowSize.top = 0;
	hwndWindowSize.right = _w;
	hwndWindowSize.bottom = _h;
}

int GameLite::MapMousePointToScreenX(const DirectX::Mouse* p)
{
	return p->GetState().x * GetScreenWidth() / GetHWNDWindowWidth();
}

int GameLite::MapMousePointToScreenY(const DirectX::Mouse* p)
{
	return p->GetState().y * GetScreenHeight() / GetHWNDWindowHeight();
}

void GameLite::ResetHWNDWindowSize()
{
	if (!isFullscreen)
		GetWindowPlacement(hwndWindow, &hwndWindowPlacement);
	RECT w{ 0,0,GetScreenWidth(), GetScreenHeight() };
	w.right = w.right * GetDeviceCaps(GetDC(hwndWindow), LOGPIXELSX) / USER_DEFAULT_SCREEN_DPI;
	w.bottom = w.bottom * GetDeviceCaps(GetDC(hwndWindow), LOGPIXELSY) / USER_DEFAULT_SCREEN_DPI;
	AdjustWindowRect(&w, isFullscreen ? hwndWindowInfo.dwStyle : GetWindowLongPtr(hwndWindow, GWL_STYLE), FALSE);
	hwndWindowPlacement.rcNormalPosition.right = hwndWindowPlacement.rcNormalPosition.left + w.right - w.left;
	hwndWindowPlacement.rcNormalPosition.bottom = hwndWindowPlacement.rcNormalPosition.top + w.bottom - w.top;
	if (!isFullscreen)
		SetWindowPlacement(hwndWindow, &hwndWindowPlacement);
}

void GameLite::ToggleFullscreen(bool fullscreen)
{
	isFullscreen = fullscreen;
	if (isFullscreen)
	{
		GetWindowInfo(hwndWindow, &hwndWindowInfo);
		GetWindowPlacement(hwndWindow, &hwndWindowPlacement);
		SetWindowLongPtr(hwndWindow, GWL_STYLE, 0);
		SetWindowLongPtr(hwndWindow, GWL_EXSTYLE, WS_EX_TOPMOST);

		SetWindowPos(hwndWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(hwndWindow, SW_SHOWMAXIMIZED);
	}
	else
	{
		SetWindowLongPtr(hwndWindow, GWL_STYLE, hwndWindowInfo.dwStyle);
		SetWindowLongPtr(hwndWindow, GWL_EXSTYLE, hwndWindowInfo.dwExStyle);

		SetWindowPlacement(hwndWindow, &hwndWindowPlacement);
		SetWindowPos(hwndWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

HRESULT GameLite::CreateHPCircle(Microsoft::WRL::ComPtr<ID2D1PathGeometry>& _circle, float r, float percent)
{
	circle.radiusX = circle.radiusY = r;
	return CreateD2DArc(_circle, d2ddevice, r, 270.0f - 360.0f*percent, 270.0f);
}

void GameLite::DrawHPCircle(ID2D1PathGeometry * _circle, float x, float y, float w, float bw,
	ID2D1Brush * ic, ID2D1Brush * bc)
{
	D2DDrawPath(d2drendertarget, _circle, x, y, ic, w);
	d2drendertarget->DrawEllipse(D2D1::Ellipse(circle.point, circle.radiusX + w / 2, circle.radiusY + w / 2), bc, bw);
	d2drendertarget->DrawEllipse(D2D1::Ellipse(circle.point, circle.radiusX - w / 2, circle.radiusY - w / 2), bc, bw);
}

int GameLite::GetScreenWidth()
{
	return screenSize.right - screenSize.left;
}

int GameLite::GetScreenHeight()
{
	return screenSize.bottom - screenSize.top;
}

int GameLite::GetHWNDWindowWidth()
{
	return hwndWindowSize.right-hwndWindowSize.left;
}

int GameLite::GetHWNDWindowHeight()
{
	return hwndWindowSize.bottom - hwndWindowSize.top;
}

void GameLite::OnBeforePresent(IDXGISwapChain*swapChain)
{
	if (km.IsOnKeyDown(keyboard.get(),DirectX::Keyboard::P))
		DXThrowIfFailed(TakeScreenShotToFile(d3ddevice,swapChain,L"shot.png"));
	km.UpdateState(keyboard.get());
}

KeyManager::KeyManager():_keyState()
{
}

void KeyManager::UpdateState(const DirectX::Keyboard* p)
{
	_keyState = p->GetState();
}

bool KeyManager::IsOnKeyDown(const DirectX::Keyboard* p, DirectX::Keyboard::Keys key)
{
	auto s = p->GetState();
	int bytes = key / 8;
	int remain = key - bytes * 8;
	bool lastKey = ((reinterpret_cast<bool*>(&_keyState)[bytes] >> remain) & 1) == 1;
	bool nowKey = ((reinterpret_cast<bool*>(&s)[bytes] >> remain) & 1) == 1;
	return nowKey && !lastKey;
}

bool KeyManager::IsOnKeyUp(const DirectX::Keyboard* p, DirectX::Keyboard::Keys key)
{
	auto s = p->GetState();
	int bytes = key / 8;
	int remain = key - bytes * 8;
	bool lastKey = ((reinterpret_cast<bool*>(&_keyState)[bytes] >> remain) & 1) == 1;
	bool nowKey = ((reinterpret_cast<bool*>(&s)[bytes] >> remain) & 1) == 1;
	return !nowKey && lastKey;
}
