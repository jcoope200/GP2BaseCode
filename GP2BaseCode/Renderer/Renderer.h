#pragma once

class IRenderer
{
public:
	virtual ~IRenderer(){};
	virtual bool init(void *pWindowHandle)=0;
	virtual void clear(float r,float g,float b,float a)=0;
	virtual void present()=0;
};