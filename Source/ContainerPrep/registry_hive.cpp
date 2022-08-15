#include "registry_hive.h"
#include "privilege_manager.h"

#include <chrono>
#include <Windows.h>

constexpr std::wstring_view keyPrefix(L"tmp_container_");

Hive::Hive(const std::wstring& name, const std::filesystem::path& hiveDirectory)
	: hiveName(name)
	, dir(hiveDirectory)
{
	std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
	std::chrono::nanoseconds nanoTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch());
	std::wstring nanoTimeString = std::to_wstring(nanoTime.count());

	keyName.reserve(keyPrefix.length() + hiveName.length() + 1 + nanoTimeString.length());
	keyName += L"tmp_container_";
	keyName += hiveName;
	keyName += L"_";
	keyName += std::move(nanoTimeString);

	HKEY hCurrentUser;
	RegOpenCurrentUser(KEY_ALL_ACCESS, &hCurrentUser);

	Privilege backupPrivilege(SE_BACKUP_NAME);
	Privilege restorePrivilege(SE_RESTORE_NAME);

	HKEY hSubKey;
	RegCreateKeyEx(hCurrentUser, keyName.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE | REG_OPTION_BACKUP_RESTORE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL);
	RegCloseKey(hCurrentUser);

	keyHandle = hSubKey;
}

Hive::~Hive()
{
	flush();
}

Hive Hive::create(const std::wstring& name, const std::filesystem::path& hiveDirectory)
{
	return Hive(name, hiveDirectory);
}

void* Hive::getHandle() const
{
	return keyHandle;
}

void Hive::flush()
{
	// Temporarily enable the backup privileges to be able to save the key
	Privilege privilege(SE_BACKUP_NAME);

	const std::filesystem::path hiveFilename = dir / hiveName;
	std::filesystem::remove(hiveFilename);

	LSTATUS Status = RegSaveKeyExW(static_cast<HKEY>(keyHandle), hiveFilename.native().c_str(), NULL, REG_LATEST_FORMAT);

	HKEY hCurrentUser;
	Status = RegOpenCurrentUser(KEY_ALL_ACCESS, &hCurrentUser);
	Status = RegDeleteTree(hCurrentUser, keyName.c_str());
	Status = RegCloseKey(hCurrentUser);

	RegCloseKey(static_cast<HKEY>(keyHandle));
}

