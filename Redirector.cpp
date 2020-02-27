
#include "Redirector.h"
#include "..\Detours\detours.h"

#define REDIRECT(name, ret, ...) typedef ret(WINAPI *##name##_t)(__VA_ARGS__); \
static name##_t Original_##name; \
ret WINAPI Redirect_##name(__VA_ARGS__)

REDIRECT(GetPrivateProfileStringA, DWORD, LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
	return Original_GetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileStringW, DWORD, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
	return Original_GetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);

}

REDIRECT(GetPrivateProfileIntA, UINT, LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
{
	return Original_GetPrivateProfileIntA(lpAppName, lpKeyName, nDefault, lpFileName);
}

REDIRECT(GetPrivateProfileIntW, UINT, LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault, LPCWSTR lpFileName)
{
	return Original_GetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, lpFileName);
}

REDIRECT(GetPrivateProfileSectionA, DWORD, LPCSTR lpAppName, LPSTR  lpReturnedString, DWORD  nSize, LPCSTR lpFileName)
{
	return Original_GetPrivateProfileSectionA(lpAppName, lpReturnedString, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileSectionW, DWORD, LPCWSTR lpAppName, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
	return Original_GetPrivateProfileSectionW(lpAppName, lpReturnedString, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileStructA, BOOL, LPCSTR lpszSection, LPCSTR lpszKey, LPVOID lpStruct, UINT   uSizeStruct, LPCSTR szFile)
{
	return Original_GetPrivateProfileStructA(lpszSection, lpszKey, lpStruct, uSizeStruct, szFile);
}

REDIRECT(GetPrivateProfileStructW, BOOL, LPCWSTR lpszSection, LPCWSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct, LPCWSTR szFile)
{
	return Original_GetPrivateProfileStructW(lpszSection, lpszKey, lpStruct, uSizeStruct, szFile);
}

REDIRECT(GetPrivateProfileSectionNamesA, DWORD, LPSTR  lpszReturnBuffer, DWORD  nSize, LPCSTR lpFileName)
{
	return Original_GetPrivateProfileSectionNamesA(lpszReturnBuffer, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileSectionNamesW, DWORD, LPWSTR  lpszReturnBuffer, DWORD   nSize, LPCWSTR lpFileName)
{
	return Original_GetPrivateProfileSectionNamesW(lpszReturnBuffer, nSize, lpFileName);

}


#define ADD_REDIRECT(name) Original_##name = (name##_t)GetProcAddress(kernel32, #name);\
AddRedirection(&(void*)Original_##name, (void*)Redirect_##name, L#name)

#define ADD_REDIRECTAW(name) ADD_REDIRECT(name##A);\
ADD_REDIRECT(name##W)


void CreareRedirections(void) {
	HMODULE kernel32 = GetModuleHandleW(L"kernel32");
	ADD_REDIRECTAW(GetPrivateProfileSection);
	ADD_REDIRECTAW(GetPrivateProfileString);
	ADD_REDIRECTAW(GetPrivateProfileInt);
	ADD_REDIRECTAW(GetPrivateProfileStruct);
	ADD_REDIRECTAW(GetPrivateProfileSectionNames);
}

Redirection* Redirections = NULL;

void AddRedirection(void** orig, void* redirect, wchar_t* name) {
	Redirection* current = (Redirection*) calloc(1, sizeof(Redirection));
	current->next = Redirections;
	current->original = orig;
	current->redirection = redirect;
	current->name = name;
	Redirections = current;
}

#pragma comment(lib, "detours.lib")
void InstallRedirects(void) {
	if (Redirections == NULL) return;
	//Add Detours
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	for (Redirection* red = Redirections; red != NULL; red = red->next) {
		DetourAttach(red->original, red->redirection);
		_MESSAGE("Redirecting %ls", red->name);
	}
	if (DetourTransactionCommit() != NO_ERROR)
		_MESSAGE("Fail detour transaction commit");

	_MESSAGE("Detour transaction commit complete");
}