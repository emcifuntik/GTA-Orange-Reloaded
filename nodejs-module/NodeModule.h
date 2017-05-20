#pragma once

enum CallbackID
{
	CALLBACK_ON_RESOURCE_LOAD,
	CALLBACK_ON_EVENT,
	CALLBACKS_COUNT
};

struct CallbackInfo
{
	uv_callback_t* callback;
	Persistent<Function>* function;
};

struct OnEventCallbackStruct
{
	char * event;
	std::vector<MValue>* args;
};

class NodeModule
{
public:
	NodeModule();
	~NodeModule();
	bool Init();
	void OnTick();
	void OnResourceLoad(const char* resource);
	void OnEvent(const char* e, std::vector<MValue> *args);
	inline node::Environment* GetEnvironment() { return this->m_env; };
	inline v8::Platform* GetPlatform() { return this->m_platform; };
	inline Isolate* GetIsolate() { return this->m_isolate; };
	inline static NodeModule* GetModule() { return m_module; };
	inline void SetCallback(CallbackID callbackID, CallbackInfo* callback) { m_callbacks[callbackID] = callback; };
	inline CallbackInfo* GetCallback(CallbackID callbackID) { return m_callbacks[callbackID]; }
private:
	static NodeModule* m_module;
	node::Environment* m_env;
	v8::Isolate* m_isolate;
	v8::Platform* m_platform;
	uv_loop_t* m_loop;
	CallbackInfo **m_callbacks;
};

