#pragma once

//The header file for the renderer interface
#include "../Renderer/Renderer.h"

#include <Windows.h>

#include <D3D10.h>
#include <D3DX10.h>

#define _XM_NO_INTRINSICS_
#include <xnamath.h>

class D3D10Renderer:public IRenderer
{
public:
        D3D10Renderer();
        ~D3D10Renderer();
        
        bool init(void *pWindowHandle,bool fullScreen);
        void clear(float r,float g,float b,float a);
        void present();
        void render();
private:
        bool createDevice(HWND pWindowHandle,int windowWidth, int windowHeight,
bool fullScreen);
        bool createInitialRenderTarget(int windowWidth, int windowHeight);

        //temp for the exercise
        bool loadEffectFromMemory(const char *pMem);
        bool loadEffectFromFile(const char *pFilename);
        bool createBuffer();
        bool creatVertexLayout();

        void createCamera(XMVECTOR &position, XMVECTOR &focu,XMVECTOR &up,float fov,float aspectRatio,float nearClip,float farClip);
        void setSquarePosition(float x,float y,float z);

        bool loadBaseTexture(char *pFilename);
private:
        ID3D10Device * m_pD3D10Device;
        IDXGISwapChain * m_pSwapChain;
        ID3D10RenderTargetView * m_pRenderTargetView;
        ID3D10DepthStencilView * m_pDepthStencelView;
        ID3D10Texture2D *m_pDepthStencilTexture;

       
        ID3D10Effect * m_pTempEffect;
        ID3D10EffectTechnique* m_pTempTechnique;

    
        ID3D10Buffer * m_pTempBuffer;
		ID3D10Buffer * m_pTempIndexBuffer;

       
        ID3D10InputLayout* m_pTempVertexLayout;

 
        XMMATRIX m_View;
        XMMATRIX m_Projection;
		XMMATRIX m_World;

		XMFLOAT3 lightDirection;
		XMFLOAT4 diffuseMaterial;
		XMFLOAT4 diffuseLightColour;

        ID3D10EffectMatrixVariable * m_pWorldEffectVariable;
        ID3D10EffectMatrixVariable * m_pProjectionEffectVariable;
        ID3D10EffectMatrixVariable * m_pViewEffectVariable;

        ID3D10ShaderResourceView * m_pBaseTextureMap;
        ID3D10EffectShaderResourceVariable *m_pBaseTextureEffectVariable;

		ID3D10EffectVariable * lightDirectionVariable;
		ID3D10EffectVariable * diffuseMaterialVariable;
		ID3D10EffectVariable * diffuseLightColourVariable;


};