#pragma once
class CRPCPlugin
{
	static CRPCPlugin* singleInstance;
	CRPCPlugin();
	void BindFunctions();
public:
	static CRPCPlugin* Get();

	RPC4 rpc;
	~CRPCPlugin();
};

