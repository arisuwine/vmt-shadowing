#pragma once
#include <Windows.h>
#include <unordered_map>

class VMTShadowing {
private:
	void*		m_pObject;
	uintptr_t*	m_pVftOrig;
	uintptr_t*	m_pUserVft;

	size_t		m_iVftSize;
	bool		m_bIsInit;

	std::unordered_map<size_t, uintptr_t> m_OriginalFuncs;

	size_t	GetFunctionCount();

public:
	VMTShadowing() : m_pObject(nullptr), m_pVftOrig(nullptr), m_pUserVft(nullptr), m_iVftSize(0), m_bIsInit(FALSE) {};
	VMTShadowing(void* object);
	~VMTShadowing();

	template <typename T>
	T			      Hook(size_t index, void* hook_func);

	bool		    UnHook(size_t index);
	bool		    UnHookAll();
	bool		    Shutdown();

	inline bool IsInitialized() const { return m_bIsInit; };
};

template <typename T>
T VMTShadowing::Hook(size_t index, void* hook_func) {
	if (!hook_func)
		return nullptr;

	if (!m_bIsInit)
		return nullptr;

	if (index > m_iVftSize)
		return nullptr;

	if (m_OriginalFuncs.find(index) == m_OriginalFuncs.end()) {
		m_OriginalFuncs[index] = m_pUserVft[index];
	}

	m_pUserVft[index] = reinterpret_cast<uintptr_t>(hook_func);

	return reinterpret_cast<T>(m_OriginalFuncs[index]);
}
