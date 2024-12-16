#pragma once
#include "GraphicsEngine/public/IModel.h"

#include <memory>


class ModelObject
{
public:
	ModelObject();
	virtual ~ModelObject();

public:
	void Update();
	void SetModel(std::shared_ptr<Ideal::IModel> Model);

};