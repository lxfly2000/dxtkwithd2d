#pragma once
#define GAME_WINDOW_WIDTH 800
#define GAME_WINDOW_HEIGHT 600
#define GAME_WINDOW_TITLE L"Direct3D 应用程序"
#include"ResLoader.h"
class GameLite
{
public:
	GameLite();
	//加载
	virtual void Init(ID3D11Device *pdevice, ID3D11DeviceContext *pcontext, ID2D1Factory *pd2ddevice, HWND hwnd);
	//释放
	virtual void Uninit();
	//加载D2D
	//DXGI依赖资源，当DXGI资源更新时需要重新加载D2D.
	virtual void InitD2D(IDXGISwapChain *pswchain);
	//释放D2D
	//DXGI依赖资源，当DXGI资源更新前需要先释放D2D。
	virtual void UninitD2D();
	//更新程序中各种数据（程序循环）
	virtual void Update();
	//画图（每帧）
	virtual void Draw();
	//系统暂停
	virtual void PauseFromSystem();
	//系统恢复
	virtual void ResumeFromSystem();
	//获取游戏区屏幕宽度
	int GetScreenWidth();
	//获取游戏区屏幕高度
	int GetScreenHeight();

	//系统更新资源属性，参数为窗口的新尺寸
	virtual void OnUpdateResProp(int _w, int _h, IDXGISwapChain *pswchain);
	//窗口被调整前的操作，因为窗口调整需要重建资源，故这里需要放释放资源的操作
	virtual void OnBeforeResizeWindow();
private:
	//这放资源
	RECT screenSize;//存放屏幕尺寸
	bool isPausedFromSystem;//是否因最小化等原因而使程序暂停运行了
	ResLoader resourceLoader;//资源加载器
	ID3D11Device *d3ddevice;//D3D设备指针，仅保存，无须释放
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;//DXTK的精灵处理对象

	ID3D11ShaderResourceView *pic;//游戏类中有一个材质图
	RECT picToScrRect;//存放图片位置与尺寸
	std::unique_ptr<DirectX::Keyboard> keyboard;//键盘对象
	std::unique_ptr<DirectX::Mouse> mouse;//鼠标对象
	std::unique_ptr<DirectX::SpriteFont> ssfont;//加载到的DXTK字体表
	TCHAR picpostext[20];//显示图片坐标的文字
	DirectX::SimpleMath::Vector2 textpos, textcenterpos;//显示图片坐标的文字位置

	DirectX::SimpleMath::Matrix matWorld, matView, matProjection;//用于绘制正方体的各种矩阵
	std::unique_ptr<DirectX::GeometricPrimitive> mdBlock;//正方体物体对象
	int fcounter;//帧计数器

	bool pressing_p;//是否按下了P键

	ID2D1Factory *d2ddevice;//D2D设备指针，仅保存，无须释放
	ID2D1RenderTarget *d2drendertarget;//D2D绘图目标，因D2D绘图比较简单故不再封装
	ID2D1SolidColorBrush *circleBrush, *outlineBrush;//不同颜色的笔刷资源
	D2D1_ELLIPSE circle;//圆形对象
	Microsoft::WRL::ComPtr<IDWriteTextFormat> textformat;//DWrite排版系统的字体
	TCHAR cursorposText[10];//显示指针坐标的文字

	Microsoft::WRL::ComPtr<IDWriteFontFace> fontface;//DWrite字体Face
	Microsoft::WRL::ComPtr<ID2D1PathGeometry> btgeometry;//文字轮廓路径
	Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> bluetowhiteBrush;//蓝到白色渐变的笔刷
};

