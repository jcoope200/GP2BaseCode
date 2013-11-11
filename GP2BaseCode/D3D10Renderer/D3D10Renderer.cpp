#include "D3D10Renderer.h"


struct Vertex
{
        float x,y,z;
        float tu,tv;
		float u,v,w;
};

const D3D10_INPUT_ELEMENT_DESC VerexLayout[] =
{                
    { "POSITION",
        0, 
        DXGI_FORMAT_R32G32B32_FLOAT, 
        0, 
        0, 
        D3D10_INPUT_PER_VERTEX_DATA, 
        0 }, 

        { "TEXCOORD", 
        0, 
        DXGI_FORMAT_R32G32_FLOAT, 
        0, 
        12, 
        D3D10_INPUT_PER_VERTEX_DATA, 
        0 }, 

		{ "NORMAL", 
        0, 
        DXGI_FORMAT_R32G32B32_FLOAT, 
        0, 
        20, 
        D3D10_INPUT_PER_VERTEX_DATA, 
        0 }, 
};

const char basicEffect[]=\
        "float4 VS( float4 Pos : POSITION ) : SV_POSITION"\
        "{"\
        "        return Pos;"\
        "}"\
        "float4 PS( float4 Pos : SV_POSITION ) : SV_Target"\
        "{"\
        "        return float4( 1.0f, 1.0f, 0.0f, 1.0f );"\
        "}"\
        "technique10 Render"\
        "{"\
        "        pass P0"\
        "        {"\
        "                SetVertexShader( CompileShader( vs_4_0, VS() ) );"\
        "                SetGeometryShader( NULL );"\
        "                SetPixelShader( CompileShader( ps_4_0, PS() ) );"\
        "        }"\
        "}";

D3D10Renderer::D3D10Renderer()
{
        m_pD3D10Device=NULL;
        m_pRenderTargetView=NULL;
        m_pSwapChain=NULL;
        m_pDepthStencelView=NULL;
        m_pDepthStencilTexture=NULL;

        m_pTempEffect=NULL;
        m_pTempBuffer=NULL;
		m_pTempIndexBuffer=NULL;
        m_pTempVertexLayout=NULL;
}

D3D10Renderer::~D3D10Renderer()
{
        if (m_pD3D10Device)
                m_pD3D10Device->ClearState();

        if (m_pBaseTextureMap)
                m_pBaseTextureMap->Release();
        if (m_pTempEffect)
                m_pTempEffect->Release();
        if (m_pTempVertexLayout)
                m_pTempVertexLayout->Release();
        if (m_pTempBuffer)
                m_pTempBuffer->Release();
		if (m_pTempIndexBuffer)
			m_pTempIndexBuffer->Release();

        if (m_pRenderTargetView)
                m_pRenderTargetView->Release();
        if (m_pDepthStencelView)
                m_pDepthStencelView->Release();
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

        //if (!loadEffectFromMemory(basicEffect))
        //        return false;
        if (!loadEffectFromFile("Effect/specular.fx"))
                return false;
        if (!creatVertexLayout())
                return false;
        if (!createBuffer())
                return false;
        if (!loadBaseTexture("Texture/face.png"))
                return false;

        XMFLOAT3 cameraPos=XMFLOAT3(3.0f,5.0f,-10.0f);
        XMFLOAT3 focusPos=XMFLOAT3(0.0f,0.0f,0.0f);
        XMFLOAT3 up=XMFLOAT3(0.0f,1.0f,0.0f);

		XMFLOAT3 m_lightDirection=XMFLOAT3(3.0f,5.0f,-10.0f);
		XMFLOAT4 m_diffuseMaterial=XMFLOAT4(0.8f,0.8f,0.8f,1.0f);
		XMFLOAT4 m_diffuseLightColour=XMFLOAT4(0.0f,0.0f,0.0f,1.0f);

		XMFLOAT4 m_specularMaterial=XMFLOAT4(0.8f,0.8f,0.8f,1.0f);
		XMFLOAT4 m_specularLightColour=XMFLOAT4(0.0f,0.0f,0.0f,1.0f);

        createCamera(XMLoadFloat3(&cameraPos),XMLoadFloat3(&focusPos),XMLoadFloat3(&up),XM_PI/4,(float)width/(float)height,0.1f,100.0f);
        setSquarePosition(0.0f,0.0f,0.0f);
        return true;
}

void D3D10Renderer::createCamera(XMVECTOR &position, XMVECTOR &focus,XMVECTOR &up,float fov,float aspectRatio,float nearClip,float farClip)
{
        m_View=XMMatrixIdentity();
        m_Projection=XMMatrixIdentity();
		

        m_View=XMMatrixLookAtLH(position,focus,up);
        m_Projection=XMMatrixPerspectiveFovLH(fov,aspectRatio,nearClip,farClip);
}

void D3D10Renderer::setSquarePosition(float x,float y,float z)
{
        m_World=XMMatrixIdentity();

        m_World=XMMatrixTranslation(x,y,z);
}

bool D3D10Renderer::loadBaseTexture(char *pFilename)
{
        if(FAILED(D3DX10CreateShaderResourceViewFromFileA(m_pD3D10Device,pFilename,NULL,NULL,&m_pBaseTextureMap,NULL)))
                return false;

        return true;
}

bool D3D10Renderer::createDevice(HWND window,int windowWidth, int windowHeight,bool fullScreen)
{
        UINT createDeviceFlags=0;
#ifdef _DEBUG
        createDeviceFlags|=D3D10_CREATE_DEVICE_DEBUG;
#endif

        DXGI_SWAP_CHAIN_DESC sd;
       ZeroMemory( &sd, sizeof( sd ) );
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        if (fullScreen)
                sd.BufferCount = 2;
        else
                sd.BufferCount=1;
        sd.OutputWindow = window;
        sd.Windowed = (BOOL)(!fullScreen);
       sd.SampleDesc.Count = 1;
       sd.SampleDesc.Quality = 0;
       sd.BufferDesc.Width = windowWidth;
       sd.BufferDesc.Height = windowHeight;
       sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
       sd.BufferDesc.RefreshRate.Numerator = 60;
       sd.BufferDesc.RefreshRate.Denominator = 1;

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
        ID3D10Texture2D *pBackBuffer;
        
        if (FAILED(m_pSwapChain->GetBuffer(0,
                __uuidof(ID3D10Texture2D),
                (void**)&pBackBuffer)))
                return false;

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

        if (FAILED(m_pD3D10Device->CreateTexture2D(&descDepth,NULL,
                        &m_pDepthStencilTexture)))
                return false;

        D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
        descDSV.Format=descDepth.Format;
        descDSV.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice=0;

        if (FAILED(m_pD3D10Device->CreateDepthStencilView(m_pDepthStencilTexture,
                   &descDSV,&m_pDepthStencelView)))
                return false;

        if (FAILED(m_pD3D10Device->CreateRenderTargetView( pBackBuffer,
                NULL,
                &m_pRenderTargetView ))){
             pBackBuffer->Release();
                return false;
        }
       pBackBuffer->Release();

        m_pD3D10Device->OMSetRenderTargets(1,
                &m_pRenderTargetView,                
                m_pDepthStencelView);
        
        D3D10_VIEWPORT vp;
           vp.Width = windowWidth;
    vp.Height = windowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    
        m_pD3D10Device->RSSetViewports( 1
                , &vp );
        return true;
}

bool D3D10Renderer::loadEffectFromMemory(const char *pMem)
{
        DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
     
        dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

        ID3D10Blob * pErrorBuffer=NULL;
        if (FAILED(D3DX10CreateEffectFromMemory(pMem,
                strlen(pMem),
                NULL,
                NULL,
                NULL,
                "fx_4_0",
                dwShaderFlags,
                0,
				m_pD3D10Device, 
                NULL, 
                NULL, 
                &m_pTempEffect, 
                &pErrorBuffer, 
                NULL )))
        {
                OutputDebugStringA((char*)pErrorBuffer->GetBufferPointer());
                return false;
        }

        m_pTempTechnique=m_pTempEffect->GetTechniqueByName("Render");
        return true;
}

bool D3D10Renderer::loadEffectFromFile(const char *pFilename)
{
        DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
   
        dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

        ID3D10Blob * pErrorBuffer=NULL;
        if (FAILED(D3DX10CreateEffectFromFileA(pFilename,
                NULL,
                NULL,
                "fx_4_0", 
                dwShaderFlags, 
                0,
				m_pD3D10Device, 
                NULL, 
                NULL, 
                &m_pTempEffect, 
                &pErrorBuffer, 
                NULL )))
        {
                OutputDebugStringA((char*)pErrorBuffer->GetBufferPointer());
                return false;
        }

        m_pTempTechnique=m_pTempEffect->GetTechniqueByName("Render");

        m_pWorldEffectVariable=m_pTempEffect->GetVariableByName("matWorld")->AsMatrix();
        m_pProjectionEffectVariable=m_pTempEffect->GetVariableByName("matProjection")->AsMatrix();
        m_pViewEffectVariable=m_pTempEffect->GetVariableByName("matView")->AsMatrix();
        m_pBaseTextureEffectVariable=m_pTempEffect->GetVariableByName("diffuseMap")->AsShaderResource();
		m_pLightDirectionVariable=m_pTempEffect->GetVariableByName("lightDirection")->AsVector();
		m_pDiffuseMaterialVariable=m_pTempEffect->GetVariableByName("diffuseMaterial")->AsVector();
		m_pDiffuseLightColourVariable=m_pTempEffect->GetVariableByName("diffuseLightColour")->AsVector();
		m_pSpecularMaterialVariable=m_pTempEffect->GetVariableByName("specularMaterial")->AsVector();
		m_pSpecularLightColourVariable=m_pTempEffect->GetVariableByName("specularLightColour")->AsVector();
        return true;
}

bool D3D10Renderer::createBuffer()
{
        Vertex verts[]={
                {-1.0f,-1.0f,1.0f,0.0f,1.0f,0.0f,0.5f,0.5f},
                {-1.0f,1.0f,1.0f,0.0f,0.0f,0.0f,0.5f,0.5f},
                {1.0f,-1.0f,1.0f,1.0f,1.0f,0.0f,-0.5f,0.5f},
                {1.0f,1.0f,1.0f,1.0f,0.0f,0.0f,-0.5f,0.5f},
				{1.0f,-1.0f,-1.0f,0.0f,1.0f,0.0f,0.5f,-0.5f},
                {1.0f,1.0f,-1.0f,0.0f,0.0f,0.0f,0.5f,-0.5f},
                {-1.0f,-1.0f,-1.0f,1.0f,1.0f,0.5f,-0.5f,-0.5f},
                {-1.0f,1.0f,-1.0f,1.0f,0.0f,0.0f,-0.5f,-0.5f}
        };
        //Buffer desc
        D3D10_BUFFER_DESC bd;
        bd.Usage = D3D10_USAGE_DEFAULT;
        bd.ByteWidth = sizeof( Vertex ) * 8;
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
		int indices[] = {
			0,1,2,1,3,2, //front
			4,5,6,5,7,6, //back
			6,7,0,7,1,0, //left
			2,3,4,3,5,4, //right
			1,7,3,7,5,3, //top
			6,0,4,0,2,4  //bottom
		};

		D3D10_BUFFER_DESC indexBD;
		indexBD.Usage = D3D10_USAGE_DEFAULT;
		indexBD.ByteWidth = sizeof(int)*36;
		indexBD.BindFlags = D3D10_BIND_INDEX_BUFFER;
		indexBD.CPUAccessFlags = 0;
		indexBD.MiscFlags = 0;

		D3D10_SUBRESOURCE_DATA InitlBData;
		InitlBData.pSysMem = &indices;

		if(FAILED(m_pD3D10Device->CreateBuffer(
			&indexBD,
			&InitlBData,
			&m_pTempIndexBuffer )))
		{
			OutputDebugStringA("Can't create buffer");
		}
        return true;
}

bool D3D10Renderer::creatVertexLayout()
{
  
    UINT numElements = sizeof( VerexLayout ) / sizeof(D3D10_INPUT_ELEMENT_DESC);
       
    D3D10_PASS_DESC PassDesc;
    m_pTempTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
        
    if (FAILED(m_pD3D10Device->CreateInputLayout( VerexLayout, 
                numElements, 
                PassDesc.pIAInputSignature,
        PassDesc.IAInputSignatureSize, 
                &m_pTempVertexLayout ))) 
        {
                OutputDebugStringA("Can't create layout");
        }
        return true;
}

void D3D10Renderer::clear(float r,float g,float b,float a)
{
    
    const float ClearColor[4] = { r, g, b, a};
    m_pD3D10Device->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
        m_pD3D10Device->ClearDepthStencilView(m_pDepthStencelView,D3D10_CLEAR_DEPTH,1.0f,0);
}

void D3D10Renderer::render()
{

        m_pWorldEffectVariable->SetMatrix((float*)&m_World);
        m_pProjectionEffectVariable->SetMatrix((float*)&m_Projection);
        m_pViewEffectVariable->SetMatrix((float*)&m_View);
        m_pBaseTextureEffectVariable->SetResource(m_pBaseTextureMap);
		m_pLightDirectionVariable->SetFloatVector((float*)&m_lightDirection);
		m_pDiffuseMaterialVariable->SetFloatVector((float*)&m_diffuseMaterial);
		m_pDiffuseLightColourVariable->SetFloatVector((float*)&m_diffuseLightColour);
		m_pSpecularMaterialVariable->SetFloatVector((float*)&m_specularMaterial);
		m_pSpecularLightColourVariable->SetFloatVector((float*)&m_specularLightColour);

        
        m_pD3D10Device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

       
        m_pD3D10Device->IASetInputLayout(m_pTempVertexLayout);

    
    UINT stride = sizeof( Vertex );
   
    UINT offset = 0;
    
    m_pD3D10Device->IASetVertexBuffers(
                0,
                1, 
                &m_pTempBuffer,
                &stride, 
                &offset );

	m_pD3D10Device->IASetIndexBuffer(m_pTempIndexBuffer, DXGI_FORMAT_R32_UINT,0);

        D3D10_TECHNIQUE_DESC techniqueDesc;
        m_pTempTechnique->GetDesc(&techniqueDesc);

        for (unsigned int i=0;i<techniqueDesc.Passes;++i)
        {
                ID3D10EffectPass *pCurrentPass=m_pTempTechnique->GetPassByIndex(i);
                pCurrentPass->Apply(0);
                m_pD3D10Device->DrawIndexed(36,0,0);
        }
}

void D3D10Renderer::present()
{
    m_pSwapChain->Present( 0, 0 );
}