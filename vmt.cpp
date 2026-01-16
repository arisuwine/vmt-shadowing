#include "vmt.hpp"

VMTShadowing::Shadowing(void* object) : m_pObject(nullptr), m_pVftOrig(nullptr), m_pUserVft(nullptr), m_iVftSize(0), m_bIsInit(FALSE) {
	if (!object)
		return;

	m_pObject = object;
	m_pVftOrig = *reinterpret_cast<uintptr_t**>(m_pObject);

	m_iVftSize = GetFunctionCount();
	if (m_iVftSize == 0)
		return;

	m_pShadowAlloc = new uintptr_t[m_iVftSize + 1];
	memcpy(m_pShadowAlloc, m_pVftOrig - 1, (m_iVftSize + 1) * sizeof(void*));

	m_pUserVft = m_pShadowAlloc + 1;
	*reinterpret_cast<void**>(object) = m_pUserVft;

	m_bIsInit = TRUE;
}

size_t VMTShadowing::GetFunctionCount() {
	if (!m_pVftOrig)
		return 0;

	size_t vft_size = 0;
	MEMORY_BASIC_INFORMATION memory_info;
	while (true) {
		uintptr_t func_ptr = m_pVftOrig[vft_size];
		if (!func_ptr)
			break;

		if (!VirtualQuery((LPCVOID)func_ptr, &memory_info, sizeof(memory_info)))
			break;

		#define PAGE_EXECUTABLE (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
		#define PAGE_SECURE (PAGE_GUARD | PAGE_NOACCESS)

		if (memory_info.State != MEM_COMMIT || memory_info.Protect & PAGE_SECURE || !(memory_info.Protect & PAGE_EXECUTABLE))
			break;

		vft_size++;
	}

	return vft_size;
}

bool VMTShadowing::UnHook(size_t index) {
	if (!m_bIsInit)
		return FALSE;

	if (index >= m_iVftSize)
		return FALSE;

	if (m_OriginalFuncs.find(index) == m_OriginalFuncs.end())
		return FALSE;

	m_pUserVft[index] = m_OriginalFuncs[index];
	m_OriginalFuncs.erase(index);
	return TRUE;
}

bool VMTShadowing::UnHookAll() {
	if (!m_bIsInit)
		return FALSE;

	for (auto& [index, func] : m_OriginalFuncs)
		m_pUserVft[index] = func;

	m_OriginalFuncs.clear();
	return TRUE;
}

bool VMTShadowing::Shutdown() {
	if (!m_bIsInit)
		return TRUE;

	*reinterpret_cast<uintptr_t**>(m_pObject) = m_pVftOrig;

	m_pObject = nullptr;
	m_pVftOrig = nullptr;

	delete[] m_pShadowAlloc;
	m_pShadowAlloc = nullptr;
	m_pUserVft = nullptr;

	m_OriginalFuncs.clear();
	m_bIsInit = FALSE;
	return TRUE;
}

VMTShadowing::~VMTShadowing() {
	Shutdown();
}
