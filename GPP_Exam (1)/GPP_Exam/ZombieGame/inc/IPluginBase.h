#pragma once

struct PluginInfo;
class IBaseInterface;

class IPluginBase
{
public:
	IPluginBase(){};
	virtual ~IPluginBase(){};

	virtual void Initialize(IBaseInterface *pInterface, PluginInfo& info) = 0;
	virtual void DllInit() = 0;
	virtual void DllShutdown() = 0;
};

