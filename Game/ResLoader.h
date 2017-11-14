#pragma once
#include <dwrite.h>
#include <d2d1.h>

//����������������D3D��������������������
class ResLoader
{
public:
	ResLoader();
	void Init(ID3D11Device *pdevice);
	void Uninit();
	//���ļ����ز���ͼƬ
	HRESULT LoadTextureFromFile(LPWSTR fpath, ID3D11ShaderResourceView **pTex, int *pw, int *ph,
		bool convertpmalpha = true);
	//COM����������ϵͳ�е����嵽�������Ҫ�ֶ��ͷ�(delete)
	//pCharacters��ָ��ʹ�õ��ַ���UTF16LE���룬�����ָ����ΪASCII�ַ���32��127��
	//ע���ַ������뱣�����򣬱��뺬������ַ���Ĭ��Ϊ��?����������SetDefaultCharacter�޸ģ���
	//�����С�ĵ�λΪDIP���豸�޹����أ�
	//pxBetweenChar��ָʾ�ַ���ˮƽ+��ֱ��࣬�ò�����Ϊ�˱�����Ƴ����ַ��غ϶����õģ�����Ӱ��ʵ����ʾ�ļ�ࡣ
	//��ע�⡿���������ع��ܴ����ַ�λ��ƫ�Ƶ�Bug�������ClearType����������������������ض��ֺ������Ե����ַ�
	//��ʾʱ�ͻ�ǳ����ԣ���˲��Ƽ�С�ֺ�����ʹ�á��Ƽ�ʹ��Direct2D+DirectWrite�Ű�ϵͳ�������ֵ�ʸ��ͼ�Ρ�
	HRESULT LoadFontFromSystem(std::unique_ptr<DirectX::SpriteFont> &outSF, unsigned textureWidth,
		unsigned textureHeight, LPWSTR fontName, float fontSize, const D2D1_COLOR_F &fontColor,
		DWRITE_FONT_WEIGHT fontWeight, wchar_t *pszCharacters = NULL, float pxBetweenChar = 1.0f,
		bool convertpmalpha = true);
	//������Ļͼ�����ļ�(PNG)
	HRESULT TakeScreenShotToFile(LPWSTR fpath);
	void SetSwapChain(IDXGISwapChain *pswchain);
private:
	ID3D11Device *m_pd3dDevice;//����������������ͷ�
	IDXGISwapChain *m_pSwapChain;//ͬ��
};

//COM����������ͼ�����ļ�(PNG)
HRESULT SaveWicBitmapToFile(IWICBitmap *wicbitmap, LPCWSTR path);
//COM����������ͼ�����ڴ�(PNG)
HRESULT SaveWicBitmapToMemory(IWICBitmap *wicbitmap, std::unique_ptr<BYTE> &outMem, size_t *pbytes);
//COM���������ڴ��д�ͼ��
HRESULT LoadWicBitmapFromMemory(Microsoft::WRL::ComPtr<IWICBitmap> &outbitmap, BYTE *mem, size_t memsize);
int ReadFileToMemory(const char *pfilename, std::unique_ptr<char> &memout, size_t *memsize,
	bool removebom = false);
//��WICͼ��ת���ɣ�
//pmalpha=true:Ԥ��͸���ȸ�ʽ��PNG��
//pmalpha=false:ֱ��͸���ȸ�ʽ��PNG��
//��ע�⡿�������ɫ������ֵΪ0���޷�ת��Ϊֱ��͸���ȸ�ʽ��
HRESULT WicBitmapConvertPremultiplyAlpha(IWICBitmap *wicbitmap,
	Microsoft::WRL::ComPtr<IWICBitmap> &outbitmap, bool pmalpha);
//����������������D2D��������������������
//COM��������Ҫ����CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE),��Ҫ����� WindowsCodecs.lib.
//��ʾ��DXTK��ԭ����ͼ����ƹ��ܣ���ʹ��ResLoader::LoadTextureFromFile��ͼƬֱ�Ӽ���ΪID3D11ShaderResourceView���͵���Դ��
HRESULT CreateD2DImageFromFile(Microsoft::WRL::ComPtr<ID2D1Bitmap> &pic, ID2D1RenderTarget *prt, LPCWSTR ppath);
//����������������DWrite��������������������
//��Ҫ����� DWrite.lib.
HRESULT CreateDWTextFormat(Microsoft::WRL::ComPtr<IDWriteTextFormat> &textformat, LPCWSTR fontName, DWRITE_FONT_WEIGHT fontWeight,
	FLOAT fontSize, DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL,
	DWRITE_FONT_STRETCH fontExpand = DWRITE_FONT_STRETCH_NORMAL, LPCWSTR localeName = L"");