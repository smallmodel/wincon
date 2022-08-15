#include "privilege_manager.h"
#include <system_error>

#include <Windows.h>

Privilege::Privilege(const wchar_t* name)
	: privilegeName(name)
{
	TOKEN_PRIVILEGES NewState;
	NewState.PrivilegeCount = 1;
	NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!LookupPrivilegeValue(NULL, name, &NewState.Privileges[0].Luid)) {
		throw std::system_error(std::error_code(GetLastError(), std::system_category()), "could not lookup for a privilege value");
	}

	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		throw std::system_error(std::error_code(GetLastError(), std::system_category()), "could not open process token");
	}

	TOKEN_PRIVILEGES PreviousState;
	DWORD PreviousStateLength = sizeof(PreviousState);
	if (!AdjustTokenPrivileges(hToken, FALSE, &NewState, sizeof(NewState), &PreviousState, &PreviousStateLength))
	{
		CloseHandle(hToken);
		throw std::system_error(std::error_code(GetLastError(), std::system_category()), "could not adjust the process token privilege");
	}

	previousState = 0;
	if (PreviousState.PrivilegeCount) {
		previousState = PreviousState.Privileges[0].Attributes;
	}

	CloseHandle(hToken);
}

Privilege::~Privilege()
{
	TOKEN_PRIVILEGES NewState;
	NewState.PrivilegeCount = 1;
	NewState.Privileges[0].Attributes = previousState;
	if (!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &NewState.Privileges[0].Luid))
	{
		return;
	}

	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	if (!hToken) {
		return;
	}

	AdjustTokenPrivileges(hToken, FALSE, &NewState, sizeof(NewState), NULL, NULL);
	CloseHandle(hToken);
}

