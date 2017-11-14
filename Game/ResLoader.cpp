//使用微软的方法创建材质图片，字体巨TM繁琐，这个加载器类用于简化这些过程。

#include "pch.h"
#include "ResLoader.h"
#include <fstream>
#include <wincodec.h>
#include <Shlwapi.h>
#include "DirectXTex\DirectXTex.h"

#define MAX_TEXTURE_DIMENSION 8192//参考：https://msdn.microsoft.com/library/windows/desktop/ff476876.aspx

#ifdef _DEBUG
HRESULT _debug_hr = S_OK;
#define C(sth) _debug_hr=sth;if(FAILED(_debug_hr))return _debug_hr
#else
#define C(sth) sth
#endif

using Microsoft::WRL::ComPtr;
using namespace DirectX;

ResLoader::ResLoader()
{
	Uninit();
}


void ResLoader::Init(ID3D11Device *pdevice)
{
	m_pd3dDevice = pdevice;
}

void ResLoader::Uninit()
{
	m_pd3dDevice = nullptr;
}

HRESULT ResLoader::LoadTextureFromFile(LPWSTR fpath, ID3D11ShaderResourceView **pTex, int *pw, int *ph,
	bool convertpmalpha)
{
	ComPtr<ID3D11Resource> tmpRes;
	if (convertpmalpha)
	{
		DirectX::ScratchImage image, pmaimage;
		DirectX::Blob pmaimageres;
		C(LoadFromWICFile(fpath, WIC_FLAGS_NONE, NULL, image));
		C(PremultiplyAlpha(image.GetImages(), image.GetImageCount(), image.GetMetadata(), NULL, pmaimage));
		C(SaveToWICMemory(pmaimage.GetImages(), pmaimage.GetImageCount(), WIC_FLAGS_NONE, GUID_ContainerFormatPng,
			pmaimageres));
		C(CreateWICTextureFromMemory(m_pd3dDevice, (BYTE*)pmaimageres.GetBufferPointer(), pmaimageres.GetBufferSize(),
			&tmpRes, pTex));
	}
	else
	{
		C(CreateWICTextureFromFile(m_pd3dDevice, fpath, &tmpRes, pTex));
	}
	ComPtr<ID3D11Texture2D> tmpTex2D;
	C(tmpRes.As(&tmpTex2D));
	CD3D11_TEXTURE2D_DESC tmpDesc;
	tmpTex2D->GetDesc(&tmpDesc);
	if (pw)*pw = tmpDesc.Width;
	if (ph)*ph = tmpDesc.Height;
	return S_OK;
}

//COM函数，保存图像至文件
HRESULT SaveWicBitmapToFile(IWICBitmap *wicbitmap, LPCWSTR path)
{
	//http://blog.csdn.net/augusdi/article/details/9051723
	IWICImagingFactory *wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IWICStream *wicstream;
	C(wicfactory->CreateStream(&wicstream));
	C(wicstream->InitializeFromFilename(path, GENERIC_WRITE));
	IWICBitmapEncoder *benc;
	IWICBitmapFrameEncode *bfenc;
	C(wicfactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &benc));
	C(benc->Initialize(wicstream, WICBitmapEncoderNoCache));
	C(benc->CreateNewFrame(&bfenc, NULL));
	C(bfenc->Initialize(NULL));
	UINT w, h;
	C(wicbitmap->GetSize(&w, &h));
	C(bfenc->SetSize(w, h));
	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	C(bfenc->SetPixelFormat(&format));
	C(bfenc->WriteSource(wicbitmap, NULL));
	C(bfenc->Commit());
	C(benc->Commit());
	benc->Release();
	bfenc->Release();
	wicstream->Release();
	wicfactory->Release();
	return S_OK;
}

//COM函数，保存图像至内存，需要手动释放(delete)
HRESULT SaveWicBitmapToMemory(IWICBitmap *wicbitmap, std::unique_ptr<BYTE> &outMem, size_t *pbytes)
{
	//http://blog.csdn.net/augusdi/article/details/9051723
	IWICImagingFactory *wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IWICStream *wicstream;
	IStream *memstream = SHCreateMemStream(NULL, 0);
	if (!memstream)return -1;
	C(wicfactory->CreateStream(&wicstream));
	C(wicstream->InitializeFromIStream(memstream));
	IWICBitmapEncoder *benc;
	IWICBitmapFrameEncode *bfenc;
	C(wicfactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &benc));
	C(benc->Initialize(wicstream, WICBitmapEncoderNoCache));
	C(benc->CreateNewFrame(&bfenc, NULL));
	C(bfenc->Initialize(NULL));
	UINT w, h;
	C(wicbitmap->GetSize(&w, &h));
	C(bfenc->SetSize(w, h));
	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	C(bfenc->SetPixelFormat(&format));
	C(bfenc->WriteSource(wicbitmap, NULL));
	C(bfenc->Commit());
	C(benc->Commit());

	ULARGE_INTEGER ulsize;
	C(wicstream->Seek({ 0,0 }, STREAM_SEEK_SET, NULL));
	C(IStream_Size(wicstream, &ulsize));
	*pbytes = (size_t)ulsize.QuadPart;
	outMem.reset(new BYTE[*pbytes]);
	ULONG readbytes;
	C(wicstream->Read(outMem.get(), (ULONG)ulsize.QuadPart, &readbytes));
	if (readbytes != ulsize.QuadPart)
		return -1;
	benc->Release();
	bfenc->Release();
	wicstream->Release();
	memstream->Release();
	wicfactory->Release();
	return S_OK;
}

HRESULT LoadWicBitmapFromMemory(ComPtr<IWICBitmap> &outbitmap, BYTE *mem, size_t memsize)
{
	IWICImagingFactory *wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IStream *memstream = SHCreateMemStream(mem, (UINT)memsize);
	IWICBitmapDecoder *wicimgdecoder;
	C(wicfactory->CreateDecoderFromStream(memstream, NULL, WICDecodeMetadataCacheOnDemand, &wicimgdecoder));
	IWICFormatConverter *wicimgsource;
	C(wicfactory->CreateFormatConverter(&wicimgsource));
	IWICBitmapFrameDecode *picframe;
	C(wicimgdecoder->GetFrame(0, &picframe));
	C(wicimgsource->Initialize(picframe, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f,
		WICBitmapPaletteTypeCustom));
	C(wicfactory->CreateBitmapFromSource(wicimgsource, WICBitmapNoCache, outbitmap.ReleaseAndGetAddressOf()));
	C(picframe->Release());
	C(wicimgsource->Release());
	C(wicimgdecoder->Release());
	C(memstream->Release());
	C(wicfactory->Release());
	return S_OK;
}

int ReadFileToMemory(const char *pfilename, std::unique_ptr<char>& memout, size_t *memsize, bool removebom)
{
	std::ifstream fin(pfilename, std::ios::in | std::ios::binary);
	if (!fin)return -1;
	int startpos = 0;
	if (removebom)
	{
		char bom[3] = "";
		fin.read(bom, sizeof bom);
		if (strncmp(bom, "\xff\xfe", 2) == 0)startpos = 2;
		if (strncmp(bom, "\xfe\xff", 2) == 0)startpos = 2;
		if (strncmp(bom, "\xef\xbb\xbf", 3) == 0)startpos = 3;
	}
	fin.seekg(0, std::ios::end);
	*memsize = (int)fin.tellg() - startpos;
	memout.reset(new char[*memsize]);
	fin.seekg(startpos, std::ios::beg);
	fin.read(memout.get(), *memsize);
	return 0;
}

HRESULT WicBitmapConvertPremultiplyAlpha(IWICBitmap *wicbitmap, ComPtr<IWICBitmap> &outbitmap, bool pmalpha)
{
	std::unique_ptr<uint8_t> mem;
	size_t memsize;
	C(SaveWicBitmapToMemory(wicbitmap, mem, &memsize));

	DirectX::ScratchImage imgold, imgnew;
	DirectX::Blob imgres;
	C(LoadFromWICMemory(mem.get(), memsize, WIC_FLAGS_NONE, NULL, imgold));
	C(PremultiplyAlpha(imgold.GetImages(), imgold.GetImageCount(), imgold.GetMetadata(),
		pmalpha ? NULL : TEX_PMALPHA_REVERSE, imgnew));
	C(SaveToWICMemory(imgnew.GetImages(), imgnew.GetImageCount(), NULL, GUID_ContainerFormatPng, imgres));
	C(LoadWicBitmapFromMemory(outbitmap, (BYTE*)imgres.GetBufferPointer(), imgres.GetBufferSize()));
	return S_OK;
}

//COM函数
HRESULT ResLoader::LoadFontFromSystem(std::unique_ptr<SpriteFont> &outSF, unsigned textureWidth, unsigned textureHeight,
	LPWSTR fontName, float fontSize, const D2D1_COLOR_F &fontColor, DWRITE_FONT_WEIGHT fontWeight,
	wchar_t *pszCharacters, float pxBetweenChar, bool convertpmalpha)
{
	//这个字体加载真心是恶心得要死，以前有D3DX的时候还好点，到现在微软直接把字体支持完全给抛弃了！
	ID2D1Factory *d2dfactory;
	IWICImagingFactory *wicfactory;
	IWICBitmap *fontBitmap;
	ID2D1RenderTarget *fontRT;
	ID2D1SolidColorBrush *fontColorBrush;
	IDWriteFactory *dwfactory;
	IDWriteTextFormat *textformat;
	IDWriteTextLayout *textlayout;
	std::vector<SpriteFont::Glyph>glyphs;
	//创建D2D
	C(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dfactory));
	//创建图像
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	if (textureWidth > MAX_TEXTURE_DIMENSION || textureHeight > MAX_TEXTURE_DIMENSION)return -1;
	C(wicfactory->CreateBitmap(textureWidth, textureHeight, GUID_WICPixelFormat32bppPBGRA,//PBGRA中的P表示预乘透明度
		WICBitmapCacheOnLoad, &fontBitmap));
	//创建图像的RT
	C(d2dfactory->CreateWicBitmapRenderTarget(fontBitmap, D2D1::RenderTargetProperties(), &fontRT));
	//创建字体
	C(fontRT->CreateSolidColorBrush(fontColor, &fontColorBrush));
	C(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwfactory), (IUnknown**)&dwfactory));
	C(dwfactory->CreateTextFormat(fontName, NULL, fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		fontSize, L"", &textformat));
	C(textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
	C(textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
	//绘制
	WCHAR str[2];
	ZeroMemory(str, sizeof str);
	int curx = 0, cury = 0;
	DWRITE_TEXT_METRICS textMet;
	DWRITE_OVERHANG_METRICS ovhMet;
	D2D_POINT_2F drawPos = { 0.0f,0.0f };
	SpriteFont::Glyph eachglyph;
	ZeroMemory(&eachglyph, sizeof eachglyph);
	float chHeight = 0.0f;//每行最大字符高度
	wchar_t *usingCharacters;
	int usingChSize;
	if (pszCharacters)
	{
		usingCharacters = pszCharacters;
		usingChSize = lstrlen(usingCharacters);
	}
	else
	{
		usingChSize = 128 - ' ';
		usingCharacters = new wchar_t[usingChSize];
		for (int i = 0; i < usingChSize; i++)
			usingCharacters[i] = ' ' + i;
	}
	fontRT->BeginDraw();
	for (int i = 0; i < usingChSize; i++)
	{
		eachglyph.Character = usingCharacters[i];
		str[0] = usingCharacters[i];//DirectXTK不支持代理对，因此把字符直接当成16位数处理就行了。
		C(dwfactory->CreateTextLayout(str, lstrlen(str), textformat,
			(float)textureWidth - drawPos.x, (float)textureHeight - drawPos.y, &textlayout));
		C(textlayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
		C(textlayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

		C(textlayout->GetMetrics(&textMet));
		C(textlayout->GetOverhangMetrics(&ovhMet));//上下采用排版间距，左右0间距
#pragma region 浮点数取整
		textMet.height = std::roundf(textMet.height);
		ovhMet.left = std::ceilf(ovhMet.left);
		ovhMet.right = std::ceilf(ovhMet.right);
#pragma endregion

		if (drawPos.x + textMet.widthIncludingTrailingWhitespace + pxBetweenChar > textureWidth)
		{
			drawPos.x = 0;
			drawPos.y += chHeight + pxBetweenChar;
			chHeight = 0.0f;
		}
		chHeight = std::max(chHeight, textMet.height);
		if (drawPos.y + chHeight > textureHeight)return -1;
		eachglyph.Subrect.left = std::lround(drawPos.x);
		eachglyph.Subrect.top = std::lround(drawPos.y);
		//可绘制彩色文字
		fontRT->DrawText(str, lstrlen(str), textformat, D2D1::RectF(drawPos.x + ovhMet.left, drawPos.y,
			(float)textureWidth, (float)textureHeight), fontColorBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
		float chPxWidth = textlayout->GetMaxWidth() + ovhMet.left + ovhMet.right;
		drawPos.x += chPxWidth;
		eachglyph.Subrect.right = std::lround(drawPos.x);
		drawPos.x += pxBetweenChar;
		eachglyph.Subrect.bottom = std::lround(drawPos.y + textMet.height);
		eachglyph.XOffset = -ovhMet.left;
		eachglyph.XAdvance = textMet.widthIncludingTrailingWhitespace - chPxWidth + ovhMet.left;
#ifdef _DEBUG
		if (eachglyph.Character >= '0'&&eachglyph.Character <= '9')
		{
			WCHAR info[40];
			swprintf_s(info, L"字符\'%c\'的宽度应是：%f，实际是：%f\n", eachglyph.Character,
				textMet.widthIncludingTrailingWhitespace,
				eachglyph.XOffset + eachglyph.Subrect.right - eachglyph.Subrect.left + eachglyph.XAdvance);
			OutputDebugString(info);
		}
#endif

		glyphs.push_back(eachglyph);
		textlayout->Release();
	}
	fontRT->EndDraw();
	if (!pszCharacters)
		delete usingCharacters;
	//由图像生成材质
	std::unique_ptr<BYTE> membitmap;
	size_t memsize;
	C(SaveWicBitmapToMemory(fontBitmap, membitmap, &memsize));
#ifdef _DEBUG
	WCHAR _debug_font_filename[MAX_PATH];
	wsprintf(_debug_font_filename, L"%s_%d_%02X%02X%02X%02X.png", fontName, (int)fontSize,
		(int)(fontColor.r * 255), (int)(fontColor.g * 255), (int)(fontColor.b * 255), (int)(fontColor.a * 255));
	C(SaveWicBitmapToFile(fontBitmap, _debug_font_filename));
#endif
	ID3D11Resource *fontTexture;
	ID3D11ShaderResourceView *fontTextureView;
	if (convertpmalpha)
	{
		DirectX::ScratchImage teximage, pmteximage;
		DirectX::Blob pmteximageres;
		C(LoadFromWICMemory(membitmap.get(), memsize, WIC_FLAGS_NONE, NULL, teximage));
		C(PremultiplyAlpha(teximage.GetImages(), teximage.GetImageCount(), teximage.GetMetadata(), NULL, pmteximage));
		C(SaveToWICMemory(pmteximage.GetImages(), pmteximage.GetImageCount(), WIC_FLAGS_NONE, GUID_ContainerFormatPng,
			pmteximageres));
		C(CreateWICTextureFromMemory(m_pd3dDevice, (BYTE*)pmteximageres.GetBufferPointer(),
			pmteximageres.GetBufferSize(), &fontTexture, &fontTextureView));
	}
	else
	{
		C(CreateWICTextureFromMemory(m_pd3dDevice, membitmap.get(), memsize, &fontTexture, &fontTextureView));
	}
	//创建SpriteFont
	outSF.reset(new SpriteFont(fontTextureView, glyphs.data(), glyphs.size(), chHeight));
	outSF.get()->SetDefaultCharacter('?');
	//释放
	fontTextureView->Release();
	fontTexture->Release();
	textformat->Release();
	dwfactory->Release();
	fontColorBrush->Release();
	fontRT->Release();
	fontBitmap->Release();
	wicfactory->Release();
	d2dfactory->Release();
	return S_OK;
}

HRESULT ResLoader::TakeScreenShotToFile(LPWSTR fpath)
{
	//https://github.com/Microsoft/DirectXTK/wiki/ScreenGrab#examples
	ComPtr<ID3D11DeviceContext> ctx;
	m_pd3dDevice->GetImmediateContext(&ctx);
	ComPtr<ID3D11Texture2D> texscreen;
	C(m_pSwapChain->GetBuffer(0, __uuidof(texscreen), (void**)texscreen.GetAddressOf()));
	return SaveWICTextureToFile(ctx.Get(), texscreen.Get(), GUID_ContainerFormatPng, fpath);
}

void ResLoader::SetSwapChain(IDXGISwapChain *pswchain)
{
	m_pSwapChain = pswchain;
}

//COM函数
HRESULT CreateD2DImageFromFile(ComPtr<ID2D1Bitmap> &pic, ID2D1RenderTarget *prt, LPCWSTR ppath)
{
	IWICImagingFactory *wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IWICBitmapDecoder *wicimgdecoder;
	C(wicfactory->CreateDecoderFromFilename(ppath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicimgdecoder));
	IWICFormatConverter *wicimgsource;
	C(wicfactory->CreateFormatConverter(&wicimgsource));
	IWICBitmapFrameDecode *picframe;
	C(wicimgdecoder->GetFrame(0, &picframe));
	C(wicimgsource->Initialize(picframe, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f,
		WICBitmapPaletteTypeCustom));
	if (FAILED(prt->CreateBitmapFromWicBitmap(wicimgsource, NULL, pic.ReleaseAndGetAddressOf())))
		return -2;
	C(picframe->Release());
	C(wicimgsource->Release());
	C(wicimgdecoder->Release());
	C(wicfactory->Release());
	return S_OK;
}

HRESULT CreateDWTextFormat(ComPtr<IDWriteTextFormat> &textformat, LPCWSTR fontName, DWRITE_FONT_WEIGHT fontWeight,
	FLOAT fontSize, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontExpand, LPCWSTR localeName)
{
	IDWriteFactory *dwfactory;
	C(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwfactory), (IUnknown**)&dwfactory));
	C(dwfactory->CreateTextFormat(fontName, NULL, fontWeight, fontStyle, fontExpand, fontSize, localeName,
		textformat.ReleaseAndGetAddressOf()));
	dwfactory->Release();
	return S_OK;
}
