#pragma once
#define GAME_WINDOW_WIDTH 800
#define GAME_WINDOW_HEIGHT 600
#define GAME_WINDOW_TITLE L"Direct3D Ӧ�ó���"
#include"ResLoader.h"
class GameLite
{
public:
	GameLite();
	//����
	virtual void Init(ID3D11Device *pdevice, ID3D11DeviceContext *pcontext, ID2D1Factory *pd2ddevice, HWND hwnd);
	//�ͷ�
	virtual void Uninit();
	//����D2D
	//DXGI������Դ����DXGI��Դ����ʱ��Ҫ���¼���D2D.
	virtual void InitD2D(IDXGISwapChain *pswchain);
	//�ͷ�D2D
	//DXGI������Դ����DXGI��Դ����ǰ��Ҫ���ͷ�D2D��
	virtual void UninitD2D();
	//���³����и������ݣ�����ѭ����
	virtual void Update();
	//��ͼ��ÿ֡��
	virtual void Draw();
	//ϵͳ��ͣ
	virtual void PauseFromSystem();
	//ϵͳ�ָ�
	virtual void ResumeFromSystem();
	//��ȡ��Ϸ����Ļ���
	int GetScreenWidth();
	//��ȡ��Ϸ����Ļ�߶�
	int GetScreenHeight();

	//ϵͳ������Դ���ԣ�����Ϊ���ڵ��³ߴ�
	virtual void OnUpdateResProp(int _w, int _h, IDXGISwapChain *pswchain);
	//���ڱ�����ǰ�Ĳ�������Ϊ���ڵ�����Ҫ�ؽ���Դ����������Ҫ���ͷ���Դ�Ĳ���
	virtual void OnBeforeResizeWindow();
private:
	//�����Դ
	RECT screenSize;//�����Ļ�ߴ�
	bool isPausedFromSystem;//�Ƿ�����С����ԭ���ʹ������ͣ������
	ResLoader resourceLoader;//��Դ������
	ID3D11Device *d3ddevice;//D3D�豸ָ�룬�����棬�����ͷ�
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;//DXTK�ľ��鴦�����

	ID3D11ShaderResourceView *pic;//��Ϸ������һ������ͼ
	RECT picToScrRect;//���ͼƬλ����ߴ�
	std::unique_ptr<DirectX::Keyboard> keyboard;//���̶���
	std::unique_ptr<DirectX::Mouse> mouse;//������
	std::unique_ptr<DirectX::SpriteFont> ssfont;//���ص���DXTK�����
	TCHAR picpostext[20];//��ʾͼƬ���������
	DirectX::SimpleMath::Vector2 textpos, textcenterpos;//��ʾͼƬ���������λ��

	DirectX::SimpleMath::Matrix matWorld, matView, matProjection;//���ڻ���������ĸ��־���
	std::unique_ptr<DirectX::GeometricPrimitive> mdBlock;//�������������
	int fcounter;//֡������

	bool pressing_p;//�Ƿ�����P��

	ID2D1Factory *d2ddevice;//D2D�豸ָ�룬�����棬�����ͷ�
	ID2D1RenderTarget *d2drendertarget;//D2D��ͼĿ�꣬��D2D��ͼ�Ƚϼ򵥹ʲ��ٷ�װ
	ID2D1SolidColorBrush *circleBrush, *outlineBrush;//��ͬ��ɫ�ı�ˢ��Դ
	D2D1_ELLIPSE circle;//Բ�ζ���
	Microsoft::WRL::ComPtr<IDWriteTextFormat> textformat;//DWrite�Ű�ϵͳ������
	TCHAR cursorposText[10];//��ʾָ�����������

	Microsoft::WRL::ComPtr<IDWriteFontFace> fontface;//DWrite����Face
	Microsoft::WRL::ComPtr<ID2D1PathGeometry> btgeometry;//��������·��
	Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> bluetowhiteBrush;//������ɫ����ı�ˢ
};

