#pragma once
#include "IPluginBase.h"

struct SteeringPlugin_Output;
struct GameDebugParams;

class IExamPlugin:public IPluginBase
{
public:
	IExamPlugin(){};
	~IExamPlugin(){};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override = 0;
	void DllInit() override = 0;
	void DllShutdown() override = 0;

	virtual void InitGameDebugParams(GameDebugParams& params){};
	virtual void Update(float dt) {};

	virtual SteeringPlugin_Output UpdateSteering(float dt) = 0;
	virtual void Render(float dt) const = 0;
};

