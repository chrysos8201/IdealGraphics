#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "Engine/IEngine.h"

class GraphicsEngine;

class Engine3 : public IEngine
{
public:
	Engine3();
	virtual ~Engine3();

public:
	bool Init() override;
	void Tick() override;
	void Render() override;

	
};

