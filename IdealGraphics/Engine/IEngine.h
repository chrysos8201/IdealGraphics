#pragma once

class IEngine
{
public:
	virtual ~IEngine() {};

	virtual bool Init() abstract;
	virtual void Tick() abstract;
	virtual void Render() abstract;
};

