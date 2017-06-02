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

	v8::Platform* platform = v8::platform::CreateDefaultPlatform(4);
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();

	int exec_argc;
	const char** exec_argv;
	node::Init(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator =
		v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate* isolate = v8::Isolate::New(create_params);
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handleScope(isolate);

	v8::Local<v8::Context> context = v8::Context::New(isolate);
	m_context.Reset(isolate, context);
	node::Environment* env = node::CreateEnvironment(node::CreateIsolateData(isolate, event_loop), context, argc, argv, exec_argc, exec_argv);
	context->GetIsolate()->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);
	v8::Context::Scope context_scope(context);
	//env->set_abort_on_uncaught_exception(false);

	{
		node::Environment::AsyncCallbackScope callback_scope(env);
		//node::Environment::AsyncHooks::ExecScope exec_scope(env, 1, 0);
		node::LoadEnvironment(env);
	}

	this->m_env = env;
	this->m_platform = platform;
	this->m_isolate = isolate;
	this->m_loop = event_loop;
	return true;
}

bool more;

void NodeModule::OnTick() {
	v8::Isolate::Scope isolateScope(m_isolate);
	v8::SealHandleScope seal(m_isolate);
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
	strcpy_s(res, strlen(resource) + 1, resource);
	uv_callback_fire(callbackInfo->callback, (void*)res, NULL);
	OnTick();
}

void NodeModule::OnEvent(const char * e, std::vector<MValue>* args)
{
	CallbackInfo* callbackInfo = GetCallback(CALLBACK_ON_EVENT);
	char* e_c = new char[strlen(e) + 1];
	strcpy_s(e_c, strlen(e) + 1, e);
	OnEventCallbackStruct* callback = new OnEventCallbackStruct();
	std::vector<MValue>* args_c = new std::vector<MValue>(*args);
	callback->event = e_c;
	callback->args = args_c;
	uv_callback_fire(callbackInfo->callback, (void*)callback, NULL);
	OnTick();
}

bool NodeModule::OnPlayerCommand(long playerid, const char * command)
{
	CallbackInfo* callbackInfo = GetCallback(CALLBACK_ON_PLAYER_COMMAND);
	char* cmd = new char[strlen(command) + 1];
	strcpy_s(cmd, strlen(command) + 1, command);
	long* pid = new long(playerid);
	OnPlayerCommandCallbackStruct* callback = new OnPlayerCommandCallbackStruct();
	callback->command = cmd;
	callback->playerid = pid;
	uv_callback_fire(callbackInfo->callback, (void*)callback, NULL);
	OnTick();
	return false;
}

bool NodeModule::OnServerCommand(std::string command)
{
	CallbackInfo* callbackInfo = GetCallback(CALLBACK_ON_SERVER_COMMAND);
	char* cmd = new char[command.length() + 1];
	strcpy_s(cmd, command.length() + 1, command.c_str());
	uv_callback_fire(callbackInfo->callback, (void*)cmd, NULL);
	OnTick();
	return false;
}

bool NodeModule::OnPlayerText(long playerId, const char * text)
{
	//TODO! Maybe should not do until CEF and CEF chat will be released!
	return false;
}
