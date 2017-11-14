//微软写的那个Game.h太他妈的混乱了，没用的东西一大堆，于是我就写了个轻量化的GameLite类。
//https://github.com/Microsoft/DirectXTK/wiki/Getting-Started

#include "pch.h"
#include "GameLite.h"

#define PICPOS_FORMAT L"%4d,%4d"

GameLite::GameLite()
{
	screenSize.left = screenSize.top = 0;
	screenSize.right = GAME_WINDOW_WIDTH;
	screenSize.bottom = GAME_WINDOW_HEIGHT;
	isPausedFromSystem = false;
	picToScrRect = {};
}

void GameLite::Init(ID3D11Device *pdevice, ID3D11DeviceContext *pcontext, ID2D1Factory *pd2ddevice, HWND hwnd)
{
	d3ddevice = pdevice;
	d2ddevice = pd2ddevice;
	resourceLoader.Init(pdevice);
	spriteBatch = std::make_unique<DirectX::SpriteBatch>(pcontext);
	resourceLoader.LoadTextureFromFile(L"C:\\Program Files\\Windows Media Player\\Media Renderer\\DMR_120.png",
		&pic, (int*)&picToScrRect.right, (int*)&picToScrRect.bottom);
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();
	mouse->SetWindow(hwnd);
	resourceLoader.LoadFontFromSystem(ssfont, 1024, 1024, L"宋体", 48, D2D1::ColorF(D2D1::ColorF::White),
		DWRITE_FONT_WEIGHT_REGULAR);
	wsprintf(picpostext, PICPOS_FORMAT, picToScrRect.left, picToScrRect.top);
	textcenterpos = ssfont->MeasureString(picpostext);
	textcenterpos.x /= 2;
	mdBlock = DirectX::GeometricPrimitive::CreateBox(pcontext, DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
	matWorld = DirectX::SimpleMath::Matrix::Identity;
	matView = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(0, 2.0f, -2.5f),
		DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
	fcounter = 0;
	pressing_p = false;

	CreateDWTextFormat(textformat, L"宋体", DWRITE_FONT_WEIGHT_NORMAL, 48.0f);
	textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	//textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	circle.radiusX = circle.radiusY = 120.0f;
}

void GameLite::Uninit()
{
	UninitD2D();
	pic->Release();
	resourceLoader.Uninit();
}

void GameLite::InitD2D(IDXGISwapChain *pswchain)
{
	IDXGISurface *surface;
	pswchain->GetBuffer(0, IID_PPV_ARGS(&surface));
	d2ddevice->CreateDxgiSurfaceRenderTarget(surface, D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), &d2drendertarget);
	surface->Release();

	d2drendertarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &circleBrush);
	d2drendertarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OrangeRed), &outlineBrush);
}

void GameLite::UninitD2D()
{
	circleBrush->Release();
	outlineBrush->Release();
	d2drendertarget->Release();
}

void GameLite::Update()
{
	matWorld = DirectX::SimpleMath::Matrix::CreateRotationY(0.005f*fcounter*DirectX::XM_PI);
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
	if (pressing_p && !key.P)
	{
		resourceLoader.TakeScreenShotToFile(L"shot.png");
	}
	pressing_p = key.P;
	
	auto mousepos = mouse->GetState();
	circle.point.x = static_cast<float>(mousepos.x);
	circle.point.y = static_cast<float>(mousepos.y);
	wsprintf(cursorposText, L"%4d,%4d", mousepos.x, mousepos.y);

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
	d2drendertarget->DrawEllipse(circle, outlineBrush, 8.0f);
	d2drendertarget->DrawEllipse(circle, circleBrush, 4.0f);
	d2drendertarget->DrawText(cursorposText, lstrlen(cursorposText), textformat.Get(),
		D2D1::RectF((float)screenSize.left, (float)screenSize.top, (float)screenSize.right,
		(float)screenSize.bottom), circleBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
	d2drendertarget->EndDraw();

	if (!isPausedFromSystem)
		Update();
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
	textpos.x = screenSize.right / 2.0f;
	textpos.y = (float)screenSize.bottom;
	matProjection = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.0f,
		(float)GetScreenWidth() / GetScreenHeight(), 0.1f, 10.0f);
	resourceLoader.SetSwapChain(pswchain);
	InitD2D(pswchain);
}

void GameLite::OnBeforeResizeWindow()
{
	UninitD2D();
}

int GameLite::GetScreenWidth()
{
	return screenSize.right - screenSize.left;
}

int GameLite::GetScreenHeight()
{
	return screenSize.bottom - screenSize.top;
}

