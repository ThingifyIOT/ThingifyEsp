#pragma once

#include <Conti.h>

class IModule
{	
	const char* _error;
protected:
	void SetError(const char *error)
	{
		_error = error;
	}
	static bool ValueChangedHandler(void* context, Node *node)
	{		
		auto module = reinterpret_cast<IModule*>(context);
		return module->OnValueChanged(node);
	}
	static NodeValue FunctionExecutionHandler(void* context, FunctionArguments& args)
	{
		auto module = reinterpret_cast<IModule*>(context);
		return module->OnFunctionExecuted(context, args);
	}
	virtual bool OnValueChanged(Node *node) { return true; }
	virtual NodeValue OnFunctionExecuted(void* context, FunctionArguments& args) { return NodeValue::Boolean(false); }
public:

	IModule()
	{
		_error = "";
	}
	const char* GetError() const
	{
		return _error;
	}
	virtual const char* GetName() = 0;
	virtual bool Init() = 0;
	virtual bool Tick() = 0;
	virtual ~IModule()
	{
	}
};