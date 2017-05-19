#include "stdafx.h"

NodeModule* NodeModule::m_module;

NodeModule::NodeModule()
{
	m_module = this;
	m_callbacks = new CallbackInfo*[CALLBACKS_COUNT];
}


NodeModule::~NodeModule()
{
}


bool NodeModule::Init()
{
	int argc = 2;
	char* argvv[] = { "", "./modules/nodejs-module/main.js" };
	char** argv = uv_setup_args(argc, argvv);
	uv_loop_t* event_loop = uv_default_loop();

	Platform* platform = v8::platform::CreateDefaultPlatform(4);
	V8::InitializePlatform(platform);
	V8::Initialize();

	int exec_argc;
	const char** exec_argv;
	node::Init(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);
	Isolate::CreateParams create_params;
	create_params.array_buffer_allocator =
		v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	Isolate* isolate = Isolate::New(create_params);
	Isolate::Scope isolate_scope(isolate);
	HandleScope handleScope(isolate);

	v8::Local<v8::Context> context = Context::New(isolate);
	node::Environment* env = node::CreateEnvironment(node::CreateIsolateData(isolate, event_loop), context, argc, argv, exec_argc, exec_argv);

	context->GetIsolate()->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);
	Context::Scope context_scope(context);

	node::LoadEnvironment(env);

	//SealHandleScope seal(isolate);

	v8::Local<v8::Array> test = v8::Array::New(isolate, 1);
	
	this->m_env = env;
	this->m_platform = platform;
	this->m_isolate = isolate;
	this->m_loop = event_loop;
	return true;
}

bool more;

void NodeModule::OnTick() {
	Isolate::Scope isolateScope(m_isolate);
	v8::platform::PumpMessageLoop(this->m_platform, this->m_env->isolate());
	more = uv_run(m_env->event_loop(), UV_RUN_NOWAIT);

	if (more == false) {
		v8::platform::PumpMessageLoop(this->m_platform, this->m_env->isolate());
		node::EmitBeforeExit(m_env);

		more = uv_loop_alive(m_env->event_loop());
		if (uv_run(m_env->event_loop(), UV_RUN_NOWAIT) != 0)
		{
			more = true;
		}
	}
}

void NodeModule::OnResourceLoad(const char * resource)
{
	CallbackInfo* callbackInfo = GetCallback(CALLBACK_ON_RESOURCE_LOAD);
	char* res = new char[strlen(resource) + 1];
	strcpy(res, resource);
	uv_callback_fire(callbackInfo->callback, (void*)res, NULL);
	OnTick();
}

void NodeModule::OnEvent(const char * e, std::vector<MValue>* args)
{
	printf("Event: %s\n", e);
	CallbackInfo* callbackInfo = GetCallback(CALLBACK_ON_EVENT);
	char* e_c = new char[strlen(e) + 1];
	strcpy(e_c, e);
	OnEventCallbackStruct* callback = new OnEventCallbackStruct();
	printf("before vector\n");
	std::vector<MValue>* args_c = new std::vector<MValue>(*args);
	printf("after vector\n");
	callback->event = e_c;
	callback->args = args_c;
	uv_callback_fire(callbackInfo->callback, (void*)callback, NULL);
	OnTick();
}
