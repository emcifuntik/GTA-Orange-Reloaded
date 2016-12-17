#pragma once
template<typename T>
class pgPtrCollection 
{
private:
	T ** m_pData;
	uint16_t m_pCount;
	uint16_t m_pSize;
public:
	T ** begin() { return m_pData; }
	T ** end() { return (m_pData + m_pCount); }
	T * at(uint16_t index) { return m_pData[index]; }
	T * operator[](uint16_t index) { return at(index); }
	int16_t count() { return m_pCount; }
	void set(uint16_t index, T * ptr) { m_pData[index] = ptr; }
};
