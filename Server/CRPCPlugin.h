#pragma once
class CRPCPlugin: public RPC4
{
	static CRPCPlugin* singleInstance;
	CRPCPlugin();
public:
	static CRPCPlugin* Get();
	~CRPCPlugin();
};

