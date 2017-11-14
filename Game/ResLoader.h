#pragma once
#include <dwrite.h>
#include <d2d1.h>

//————————D3D函数————————
class ResLoader
{
public:
	ResLoader();
	void Init(ID3D11Device *pdevice);
	void Uninit();
	//从文件加载材质图片
	HRESULT LoadTextureFromFile(LPWSTR fpath, ID3D11ShaderResourceView **pTex, int *pw, int *ph,
		bool convertpmalpha = true);
	//COM函数，加载系统中的字体到字体表，需要手动释放(delete)
	//pCharacters：指定使用的字符，UTF16LE编码，如果不指定则为ASCII字符（32～127）
	//注意字符串必须保持增序，必须含有替代字符（默认为“?”，可以用SetDefaultCharacter修改）。
	//字体大小的单位为DIP（设备无关像素）
	//pxBetweenChar：指示字符的水平+垂直间距，该参数是为了避免绘制出的字符重合而设置的，不会影响实际显示的间距。
	//【注意】该字体表加载功能存在字符位置偏移的Bug，如果是ClearType看不出来但是如果字体在特定字号下是以点阵字符
	//显示时就会非常明显，因此不推荐小字号字体使用。推荐使用Direct2D+DirectWrite排版系统绘制文字等矢量图形。
	HRESULT LoadFontFromSystem(std::unique_ptr<DirectX::SpriteFont> &outSF, unsigned textureWidth,
		unsigned textureHeight, LPWSTR fontName, float fontSize, const D2D1_COLOR_F &fontColor,
		DWRITE_FONT_WEIGHT fontWeight, wchar_t *pszCharacters = NULL, float pxBetweenChar = 1.0f,
		bool convertpmalpha = true);
	//保存屏幕图像至文件(PNG)
	HRESULT TakeScreenShotToFile(LPWSTR fpath);
	void SetSwapChain(IDXGISwapChain *pswchain);
private:
	ID3D11Device *m_pd3dDevice;//仅保存变量，无须释放
	IDXGISwapChain *m_pSwapChain;//同上
};

//COM函数，保存图像至文件(PNG)
HRESULT SaveWicBitmapToFile(IWICBitmap *wicbitmap, LPCWSTR path);
//COM函数，保存图像至内存(PNG)
HRESULT SaveWicBitmapToMemory(IWICBitmap *wicbitmap, std::unique_ptr<BYTE> &outMem, size_t *pbytes);
//COM函数，从内存中打开图像
HRESULT LoadWicBitmapFromMemory(Microsoft::WRL::ComPtr<IWICBitmap> &outbitmap, BYTE *mem, size_t memsize);
int ReadFileToMemory(const char *pfilename, std::unique_ptr<char> &memout, size_t *memsize,
	bool removebom = false);
//将WIC图像转换成：
//pmalpha=true:预乘透明度格式（PNG）
//pmalpha=false:直接透明度格式（PNG）
//【注意】如果有颜色分量的值为0将无法转换为直接透明度格式。
HRESULT WicBitmapConvertPremultiplyAlpha(IWICBitmap *wicbitmap,
	Microsoft::WRL::ComPtr<IWICBitmap> &outbitmap, bool pmalpha);
//————————D2D函数————————
//COM函数，需要调用CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE),需要引入库 WindowsCodecs.lib.
//提示：DXTK有原生的图像绘制功能，可使用ResLoader::LoadTextureFromFile将图片直接加载为ID3D11ShaderResourceView类型的资源。
HRESULT CreateD2DImageFromFile(Microsoft::WRL::ComPtr<ID2D1Bitmap> &pic, ID2D1RenderTarget *prt, LPCWSTR ppath);
//————————DWrite函数————————
//需要引入库 DWrite.lib.
HRESULT CreateDWTextFormat(Microsoft::WRL::ComPtr<IDWriteTextFormat> &textformat, LPCWSTR fontName, DWRITE_FONT_WEIGHT fontWeight,
	FLOAT fontSize, DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL,
	DWRITE_FONT_STRETCH fontExpand = DWRITE_FONT_STRETCH_NORMAL, LPCWSTR localeName = L"");