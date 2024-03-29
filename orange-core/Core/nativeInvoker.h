#pragma once

class scrNativeCallContext {
protected:

	void *		m_pReturn;
	uint32_t	m_nArgCount;
	void *		m_pArgs;
	uint32_t	m_nDataCount;
public:
	template<typename T>
	inline T GetArgument(int idx)
	{
		intptr_t * arguments = (intptr_t*)m_pArgs;
		return *(T*)&arguments[idx];
	}

	template<typename T>
	inline void SetResult(int idx, T value) 
	{
		intptr_t * returnValues = (intptr_t*)m_pReturn;
		*(T*)&returnValues[idx] = value;
	}

	inline int GetArgumentCount() {
		return m_nArgCount;
	}

	template<typename T>
	inline T GetResult(int idx)
	{
		intptr_t * returnValues = (intptr_t*)m_pReturn;
		return *(T*)&returnValues[idx];
	}
};

class NativeContext : public scrNativeCallContext {
private:
	enum
	{
		MaxNativeParams = 16,
		ArgSize = 8,
	};
	uint8_t m_TempStack[MaxNativeParams * ArgSize];

public:

	inline NativeContext() 
	{
		m_pArgs = &m_TempStack;
		m_pReturn = &m_TempStack;
		m_nArgCount = 0;
		m_nDataCount = 0;
	}

	template <typename T>
	inline void Push(T value) 
	{
		if (sizeof(T) > ArgSize)
			throw "Argument has an invalid size";
		else if (sizeof(T) < ArgSize) 
			*reinterpret_cast<uintptr_t*>(m_TempStack + ArgSize * m_nArgCount) = 0;
		*reinterpret_cast<T*>(m_TempStack + ArgSize * m_nArgCount) = value;
		m_nArgCount++;
	}

	inline void Reverse() {
		uintptr_t tempValues[MaxNativeParams];
		uintptr_t * args = (uintptr_t*)m_pArgs;

		for (uint32_t i = 0; i < m_nArgCount; i++) 
		{
			int target = m_nArgCount - i - 1;
			tempValues[target] = args[i];
		}
		memcpy(m_TempStack, tempValues, sizeof(m_TempStack));
	}

	template <typename T>
	inline T GetResult() 
	{
		return *reinterpret_cast<T*>(m_TempStack);
	}
};

struct pass 
{
	template<typename ...T> pass(T...) {}
};

class NativeInvoke 
{
private:
	static void Invoke(NativeContext * cxt, uint64_t hash);
public:
	template<typename R, typename... Args>
	static inline R Invoke(uint64_t Hash, Args... args) {
		NativeContext cxt;
		pass{ ([&]() {
			cxt.Push(args);
		}(), 1)... };
		cxt.Reverse();
		Invoke(&cxt, Hash);
		return cxt.GetResult<R>();
	}
};
