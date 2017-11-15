# DirectX Toolkit with Direct2D 测试程序

## 说明
本程序的目的是尝试在Direct3D11.0中使用Direct2D以方便进行在D3D中的字体渲染，程序采用 DirectX Toolkit 以简化操作。

## 编译方法
在 VS2017 中打开解决方案文件，编译`Game`项目即可。若提示缺少相应组件，请自行下载安装。

## 运行方法
编译后找到`Game.exe`运行即可。本程序除了 VC++15 运行库外无任何依赖项。

### Note
我的电脑的DirectX版本是11.0，在MSDN网站上查到的资料说Direct2D只能在D3D 10.1, 11.1, 12及更高版本中才能与D3D结合使用，然而在我测试时发现D3D11.0也能使用D2D, 条件是：
* 在创建D3D设备时指定`D3D11_CREATE_DEVICE_BGRA_SUPPORT`标志；
* 在创建D2D的DXGI渲染目标时的像素格式指定为`D2D1_ALPHA_MODE_PREMULTIPLIED`。

不知道是微软搞错了还是我哪里没有弄清楚，总之先记录一下。

### Note
如果要在Win7系统中运行该程序需要在编译时将代码中的`D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT`改为`D2D1_DRAW_TEXT_OPTIONS_NONE`，否则将无法显示文字。