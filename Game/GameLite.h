#pragma once
#define GAME_WINDOW_WIDTH 800
#define GAME_WINDOW_HEIGHT 600
#define GAME_WINDOW_TITLE L"Direct3D Ӧ�ó���"
#include"ResLoader.h"

class KeyManager
{
private:
	DirectX::Keyboard::State _keyState;
public:
	KeyManager();
	void UpdateState(const DirectX::Keyboard* p);
	bool IsOnKeyDown(const DirectX::Keyboard* p, DirectX::Keyboard::Keys key);
	bool IsOnKeyUp(const DirectX::Keyboard* p, DirectX::Keyboard::Keys key);
};

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
	virtual void Update(float elapsedTime);
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
	//��ȡ������Ļ���
	int GetHWNDWindowWidth();
	//��ȡ������Ļ�߶�
	int GetHWNDWindowHeight();
	//Presentǰ�Ĳ���
	virtual void OnBeforePresent(IDXGISwapChain*swapChain);

	//ϵͳ������Դ���ԣ�����Ϊ���ڵ��³ߴ�
	virtual void OnUpdateResProp(int _w, int _h, IDXGISwapChain *pswchain);
	//���ڱ�����ǰ�Ĳ�������Ϊ���ڵ�����Ҫ�ؽ���Դ����������Ҫ���ͷ���Դ�Ĳ���
	virtual void OnBeforeResizeWindow();
	virtual void OnNewHWNDWindowSize(int _w, int _h);
	int MapMousePointToScreenX(const DirectX::Mouse* p);
	int MapMousePointToScreenY(const DirectX::Mouse* p);
	void ResetHWNDWindowSize();
	void ToggleFullscreen(bool fullscreen);
private:
	//�����Դ
	RECT screenSize;//�����Ļ�ߴ�
	RECT hwndWindowSize;
	HWND hwndWindow;
	WINDOWPLACEMENT hwndWindowPlacement;
	WINDOWINFO hwndWindowInfo;
	bool isFullscreen;
	bool isPausedFromSystem;//�Ƿ�����С����ԭ���ʹ������ͣ������
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

	KeyManager km;

	ID2D1Factory *d2ddevice;//D2D�豸ָ�룬�����棬�����ͷ�
	ID2D1RenderTarget *d2drendertarget;//D2D��ͼĿ�꣬��D2D��ͼ�Ƚϼ򵥹ʲ��ٷ�װ
	ID2D1SolidColorBrush *circleBrush, *outlineBrush;//��ͬ��ɫ�ı�ˢ��Դ
	D2D1_ELLIPSE circle;//Բ�ζ���
	Microsoft::WRL::ComPtr<ID2D1PathGeometry> hpcircle;//HP����
	Microsoft::WRL::ComPtr<IDWriteTextFormat> textformat;//DWrite�Ű�ϵͳ������
	TCHAR cursorposText[20];//��ʾָ�����������

	Microsoft::WRL::ComPtr<IDWriteFontFace> fontface;//DWrite����Face
	Microsoft::WRL::ComPtr<ID2D1PathGeometry> btgeometry;//��������·��
	Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> bluetowhiteBrush;//������ɫ����ı�ˢ
	//percent: 0.0f��1.0f
	HRESULT CreateHPCircle(Microsoft::WRL::ComPtr<ID2D1PathGeometry> &_circle, float r, float percent);
	void DrawHPCircle(ID2D1PathGeometry *_circle, float x, float y, float w, float bw,
		ID2D1Brush *inner, ID2D1Brush *border);
};

