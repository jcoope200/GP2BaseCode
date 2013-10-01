#include "D3D10Renderer.h"

#include <d3d10.h>
#include <d3dx10.h>

struct Vertex{float x, y, z;};

D3D10Renderer::D3D10Renderer()
{
	m_pD3D10Device=NULL;
	m_pRenderTargetView=NULL;
	m_pSwapChain=NULL;
	m_pDepthStencilView=NULL;
	m_pDepthStencilTexture=NULL;
	m_pTempEffect=NULL;
	m_pTempTechnique=NULL;
	m_pTempBuffer=NULL;
	m_pTempVertexLayout=NULL;
}

D3D10Renderer::~D3D10Renderer()
{
	if (m_pD3D10Device)
		m_pD3D10Device->ClearState();

	if (m_pTempEffect)
		m_pTempEffect->Release();
	if (m_pTempVertexLayout)
		m_pTempVertexLayout->Release();
	if (m_pTempBuffer)
		m_pTempBuffer->Release();

	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if (m_pDepthStencilView)
		m_pDepthStencilView->Release();
	if (m_pDepthStencilTexture)
		m_pDepthStencilTexture->Release();
	if (m_pSwapChain)
		m_pSwapChain->Release();
	if (m_pD3D10Device)
		m_pD3D10Device->Release();
	
}

bool D3D10Renderer::init(void *pWindowHandle,bool fullScreen)
{
	HWND window=(HWND)pWindowHandle;
	RECT windowRect;
	GetClientRect(window,&windowRect);

	UINT width=windowRect.right-windowRect.left;
	UINT height=windowRect.bottom-windowRect.top;

	if (!createDevice(window,width,height,fullScreen))
		return false;
	if (!createInitialRenderTarget(width,height))
		return false;
	//if (!loadEffectFromMemory(pMem))
		//return false;
	if (!createBuffer())
		return false;
	if (!createVertexLayout())
		return false;

	return true;
}

bool D3D10Renderer::createDevice(HWND window,int windowWidth, int windowHeight, bool fullScreen)
{
	UINT createDeviceFlags=0;
#ifdef _DEBUG
	createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;
#endif

	//describes the series of image display buffers
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	//describe the CPU access option for the back buffer for both shading and render targets
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	if (fullScreen)
		//the swap chain has two buffers if the window is full screen
		sd.BufferCount = 2;
	else
		//the swap chain has one buffer if the window is not full screen
		sd.BufferCount = 1;
	//handle to the output window, cannot be null
	sd.OutputWindow = window;
	//confirm whether or not the output window is full screen
	sd.Windowed = (BOOL)(!fullScreen);
	//describe any multi-sampling parameters
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	//describe the display mode of the backbuffer, taking into account the width, height, format, and how quickly it refreshes
	sd.BufferDesc.Width = windowWidth;
	sd.BufferDesc.Height = windowHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	//create a D3D10 device and a swap chain if the app failed
	//this includes a null pointer as there is no IDXGIAdapter
	//the specified driver type
	//a null reference to a software rasterizer
	//the unsigned integer value for flags defined above
	//the version of the SDK
	//a description of the swap chain
	//the address of the swap chain's pointer
	//the address of the D3D10 interface that will receive this new device
	if (FAILED(D3D10CreateDeviceAndSwapChain(NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		D3D10_SDK_VERSION,
		&sd,
		&m_pSwapChain,
		&m_pD3D10Device)))
		return false;

	return true;
}

bool D3D10Renderer::createInitialRenderTarget(int windowWidth, int windowHeight)
{
	//this is a 2D texture interface designed to store texture and pixel data
	ID3D10Texture2D *pBackBuffer;

	//GetBuffer accesses a buffer in the swap chain
	//0 is the index of the buffer
	//ID3D10Texture2D is the interface used
	//pBackBuffer is the pointer to the selected buffer
	if (FAILED(m_pSwapChain->GetBuffer(0,
		__uuidof(ID3D10Texture2D),
		(void**)&pBackBuffer)))
		return false;

	//this describes a 2D texture using the height, width, number of sub textures, number of textures within the array, the format, 
	//the number and quality of the subsampling textures, the methods of using the texture, the flags for referring to pipeline stages,
	//the type of CPU access allowed and any other flags for less common resource options
	D3D10_TEXTURE2D_DESC descDepth;
	descDepth.Width=windowWidth;
	descDepth.Height=windowHeight;
	descDepth.MipLevels=1;
	descDepth.ArraySize=1;
	descDepth.Format=DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count=1;
	descDepth.SampleDesc.Quality=0;
	descDepth.Usage=D3D10_USAGE_DEFAULT;
	descDepth.BindFlags=D3D10_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags=0;
	descDepth.MiscFlags=0;

	//creates an array of 2D textures based off of a pointer to a texture resource, a null value for texture subresources, 
	//and a pointer to an interface
	if (FAILED(m_pD3D10Device->CreateTexture2D(&descDepth,NULL,&m_pDepthStencilTexture)))
		return false;

	//this structure specifies the subresources from a texture that are accessed through a depth stencil view
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format=descDepth.Format;
	descDSV.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice=0;

	//this creates a depth stencil view for accessing resource data
	//it takes in a pointer to the resource that will be used as the depth stencil surface
	//and a pointer to the depth stencil view description made above
	//and it gives out an address to the depth stencil view
	if (FAILED(m_pD3D10Device->CreateDepthStencilView(m_pDepthStencilTexture, &descDSV,&m_pDepthStencilView)))
		return false;

	if (FAILED(m_pD3D10Device->CreateRenderTargetView( pBackBuffer,
		NULL,
		&m_pRenderTargetView ))){
			pBackBuffer->Release();
			return false;
	}
	pBackBuffer->Release();

	//this binds the render targets and the depth stencil buffer to the output merger stage
	//there is only 1 render target to bind
	//takes in a pointer to the array of render targets
	//and a pointer to the depth stencil view to bind to them
	m_pD3D10Device->OMSetRenderTargets(1,
		&m_pRenderTargetView,
		m_pDepthStencilView);

	//this defines the viewport's dimensions
	//including the width, height, depth, and co-ordinate of the top of the viewport
	D3D10_VIEWPORT vp;
	vp.Width = windowWidth;
	vp.Height = windowHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	//this binds the viewport array to the rasterizer
	//takes in the number of viewports and the array of viewports
	m_pD3D10Device->RSSetViewports(1, &vp);
	return true;
}

bool D3D10Renderer::createBuffer()
{
	Vertex verts[]={
		{-1.0f,-1.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{1.0f,-1.0f,0.0f}
	};

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( Vertex ) * 3;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &verts;

	if (FAILED(m_pD3D10Device->CreateBuffer(
		&bd,
		&InitData,
		&m_pTempBuffer )))
	{
		OutputDebugStringA("Can't create buffer");
	}
	return true;
}

void D3D10Renderer::clear(float r,float g,float b,float a)
{
	const float ClearColor[4] = { r, g, b, a};
	//this sets all of the render targets to one value
	//takes in a pointer to the render target and a four part array that determines what color to use
	m_pD3D10Device->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	//this clears the depth stencil resource
	//takes in the depth stencil to be cleared, the type of data to clear, and the values to fill both the depth buffer and stencil buffer with
	m_pD3D10Device->ClearDepthStencilView( m_pDepthStencilView, D3D10_CLEAR_DEPTH,1.0f,0);
}

void D3D10Renderer::present()
{
	//this presents the next set of buffers in the set of buffers owned by the swap chain
	m_pSwapChain->Present( 0, 0);
}

void D3D10Renderer::render()
{
}