#pragma once

#include "registry_windows_platform.h"
#include "privilege_manager.h"

#include <chrono>
#include <fstream>
#include <system_error>

#include <Windows.h>

using namespace Registry;

const Platform::Windows::DataType Platform::Windows::StringDataType(L"REG_SZ", REG_SZ);
const Platform::Windows::DataType Platform::Windows::ExpandableStringDataType(L"REG_EXPAND_SZ", REG_EXPAND_SZ);
const Platform::Windows::DataType Platform::Windows::MultiStringDataType(L"REG_MULTI_SZ", REG_MULTI_SZ);
const Platform::Windows::DataType Platform::Windows::BinaryDataType(L"REG_BINARY", REG_BINARY);
const Platform::Windows::DataType Platform::Windows::DWordDataType(L"REG_DWORD", REG_DWORD);
const Platform::Windows::DataType Platform::Windows::QWordDataType(L"REG_QWORD", REG_QWORD);

const IDataType& Platform::Host::StringDataType = Platform::Windows::StringDataType;
const IDataType& Platform::Host::ExpandableStringDataType = Platform::Windows::ExpandableStringDataType;
const IDataType& Platform::Host::MultiStringDataType = Platform::Windows::MultiStringDataType;
const IDataType& Platform::Host::BinaryDataType = Platform::Windows::BinaryDataType;
const IDataType& Platform::Host::DWordDataType = Platform::Windows::DWordDataType;
const IDataType& Platform::Host::QWordDataType = Platform::Windows::QWordDataType;

static Platform::Windows::PredefinedKey PKClassesRoot(HKEY_CLASSES_ROOT, L"HKEY_CLASSES_ROOT");
static Platform::Windows::PredefinedKey PKCurrentUser(HKEY_CURRENT_USER, L"HKEY_CURRENT_USER");
static Platform::Windows::PredefinedKey PKLocalMachine(HKEY_LOCAL_MACHINE, L"HKEY_LOCAL_MACHINE");
static Platform::Windows::PredefinedKey PKUsers(HKEY_USERS, L"HKEY_USERS");
static Platform::Windows::PredefinedKey PKCurrentConfig(HKEY_CURRENT_CONFIG, L"HKEY_CURRENT_CONFIG");

static void NoDeleter(IKey* key) {}

const IKeyPtr Platform::Windows::PredefinedKeys::ClassesRoot(&PKClassesRoot, NoDeleter);
const IKeyPtr Platform::Windows::PredefinedKeys::CurrentUser(&PKCurrentUser, NoDeleter);
const IKeyPtr Platform::Windows::PredefinedKeys::LocalMachine(&PKLocalMachine, NoDeleter);
const IKeyPtr Platform::Windows::PredefinedKeys::Users(&PKUsers, NoDeleter);
const IKeyPtr Platform::Windows::PredefinedKeys::CurrentConfig(&PKCurrentConfig, NoDeleter);

static const Platform::Windows::DataType& getType(uint32_t dwType)
{
	switch (dwType)
	{
	case REG_SZ:
		return Platform::Windows::StringDataType;
	case REG_EXPAND_SZ:
		return Platform::Windows::ExpandableStringDataType;
	case REG_MULTI_SZ:
		return Platform::Windows::MultiStringDataType;
	case REG_BINARY:
		return Platform::Windows::BinaryDataType;
	case REG_DWORD:
		return Platform::Windows::DWordDataType;
	case REG_QWORD:
		return Platform::Windows::QWordDataType;
	default:
		return Platform::Windows::BinaryDataType;
	}
}

IKeyPtr Platform::Windows::PredefinedKeys::findByName(const wchar_t* name)
{
	if (!_wcsicmp(name, L"HKLM")) {
		return LocalMachine;
	}
	else if (!_wcsicmp(name, L"HKU")) {
		return Users;
	}
	else if (!_wcsicmp(name, L"HKCR")) {
		return ClassesRoot;
	}
	else if (!_wcsicmp(name, L"HKCU")) {
		return CurrentUser;
	}
	else if (!_wcsicmp(name, L"HKCC")) {
		return CurrentConfig;
	}
	else {
		return nullptr;
	}
}

Platform::Windows::Hive::Hive(const IKeyPtr& inKey, std::wstring&& name)
	: key(inKey)
	, hiveName(std::move(name))
{
}

Platform::Windows::Hive::~Hive()
{
}

Platform::Windows::Hive::Hive(Hive&& other)
	: key(std::move(other.key))
	, hiveName(std::move(other.hiveName))
{
}

Platform::Windows::Hive& Platform::Windows::Hive::operator=(Hive&& other)
{
	key = std::move(other.key);
	hiveName = std::move(other.hiveName);

	return *this;
}

const wchar_t* Platform::Windows::Hive::getHiveName() const
{
	return hiveName.c_str();
}

IKeyPtr Platform::Windows::Hive::getRootKey() const
{
	return key;
}

Platform::Windows::BaseKey::BaseKey(void* inKeyHandle)
	: keyHandle(inKeyHandle)
{

}

Platform::Windows::BaseKey::BaseKey(BaseKey&& other)
	: keyHandle(other.keyHandle)
{
	other.keyHandle = nullptr;
}

Platform::Windows::BaseKey& Platform::Windows::BaseKey::operator=(BaseKey&& other)
{
	keyHandle = other.keyHandle;
	other.keyHandle = nullptr;

	return *this;
}

void* Platform::Windows::BaseKey::getKeyHandle() const
{
	return keyHandle;
}

Platform::Windows::Key::Key(void* inKeyHandle, const std::wstring_view& keyName)
	: BaseKey(inKeyHandle)
	, name(keyName)
{
}

Platform::Windows::Key::~Key()
{
	dispose();
}

Platform::Windows::Key::Key(Key&& other)
	: BaseKey(std::move(other))
	, name(std::move(other.name))
{
}

const wchar_t* Platform::Windows::Key::getKeyName() const
{
	return name.c_str();
}

Platform::Windows::Key& Platform::Windows::Key::operator=(Key&& other)
{
	keyHandle = other.keyHandle;
	name = std::move(other.name);
	other.keyHandle = nullptr;
	return *this;
}

void Platform::Windows::Key::dispose()
{
	if (keyHandle)
	{
		RegCloseKey(static_cast<HKEY>(keyHandle));
		keyHandle = nullptr;
	}
}

Platform::Windows::PredefinedKey::PredefinedKey(void* inKeyHandle, const wchar_t* inName)
	: BaseKey(inKeyHandle)
	, name(inName)
{
}

Platform::Windows::PredefinedKey::~PredefinedKey()
{

}

const wchar_t* Platform::Windows::PredefinedKey::getKeyName() const
{
	return name;
}

/*
Platform::Values::WindowsValueImpl::WindowsValueImpl(const wchar_t* inValueName)
	: valueName(inValueName)
{

}

const wchar_t* Platform::Values::WindowsValueImpl::getValue()
{
	return valueName.c_str();
}

void Platform::Values::WindowsValueImpl::setData(IKey* key, const unsigned char* data, uint32_t dataSize, uint32_t dwType)
{
	Platform::WindowsKey* windowsKey = static_cast<Platform::WindowsKey*>(key);
	RegSetValueEx(static_cast<HKEY>(windowsKey->getKeyHandle()), valueName.c_str(), 0, dwType, data, dataSize);
}

uint32_t Platform::Values::WindowsValueImpl::getData(IKey* key, std::vector<unsigned char>& out) const
{
	Platform::WindowsKey* windowsKey = static_cast<Platform::WindowsKey*>(key);
	HKEY hKey = static_cast<HKEY>(windowsKey->getKeyHandle());

	DWORD dwType = 0;
	DWORD cbData = 0;
	if (RegQueryValueExW(hKey, valueName.c_str(), 0, &dwType, NULL, &cbData) == ERROR_MORE_DATA)
	{
		out.resize(cbData);
		RegQueryValueExW(hKey, valueName.c_str(), 0, &dwType, out.data(), &cbData);
	}

	return (uint32_t)dwType;
}

Platform::WindowsDataType::WindowsDataType(const uint32_t type)
{

}

uint32_t Platform::WindowsDataType::getType() const
{

}
*/

Platform::Windows::Value::Value(const wchar_t* valueName, const IDataType& type)
	: name(valueName)
	, dataType(type)
{

}

Platform::Windows::Value::Value(const wchar_t* valueName, const IDataType& type, std::vector<unsigned char>&& data)
	: name(valueName)
	, dataType(type)
	, rawData(std::move(data))
{

}

const wchar_t* Platform::Windows::Value::getValueName() const
{
	return name.c_str();
}

const IDataType& Platform::Windows::Value::getType() const
{
	return dataType;
}

std::span<const unsigned char> Platform::Windows::Value::getRawData() const
{
	return rawData;
}

void Platform::Windows::Value::setRawData(const std::span<const unsigned char>& data)
{
	rawData.assign(data.begin(), data.end());
}

REGSAM getPermission(Permission permissions)
{
	REGSAM regSam = 0;

	if (permissions == Permission::All) {
		regSam = KEY_ALL_ACCESS;
	}
	else
	{
		if (permissions & Permission::Read) {
			regSam |= KEY_READ;
		}

		if (permissions & Permission::Write) {
			regSam |= KEY_WRITE;
		}

		if (permissions & Permission::Execute) {
			regSam |= KEY_EXECUTE;
		}
	}

	return regSam;
}

IKeyPtr Platform::Windows::KeyManager::getKey(const wchar_t* keyName, Permission permissions, const IKey* parentKey)
{
	const REGSAM regSam = getPermission(permissions);

	HKEY parentKeyHandle = NULL;
	if (parentKey)
	{
		const Windows::BaseKey* winKey = static_cast<const Windows::Key*>(parentKey);
		parentKeyHandle = static_cast<HKEY>(winKey->getKeyHandle());
	}

	HKEY keyHandle = NULL;
	const LSTATUS Status = RegOpenKeyEx(parentKeyHandle, keyName, NULL, regSam, &keyHandle);
	if (Status != ERROR_SUCCESS)
	{
		throw std::system_error(std::error_code(Status, std::system_category()), "couldn't open registry key");
	}

	return std::make_shared<Windows::Key>(keyHandle, keyName);
}

IKeyPtr Platform::Windows::KeyManager::getOrCreateKey(const wchar_t* keyName, Permission permissions, const IKey* parentKey)
{
	const REGSAM regSam = getPermission(permissions);

	HKEY parentKeyHandle = NULL;
	if (parentKey)
	{
		const Windows::BaseKey* winKey = static_cast<const Windows::Key*>(parentKey);
		parentKeyHandle = static_cast<HKEY>(winKey->getKeyHandle());
	}

	HKEY keyHandle = NULL;
	const LSTATUS Status = RegCreateKeyEx(parentKeyHandle, keyName, NULL, NULL, 0, regSam, NULL, &keyHandle, NULL);
	if (Status != ERROR_SUCCESS)
	{
		throw std::system_error(std::error_code(Status, std::system_category()), "couldn't open registry key");
	}

	return std::make_shared<Windows::Key>(keyHandle, keyName);
}

IValuePtr Platform::Windows::ValueManager::getValue(const IKey* key, const wchar_t* valueName)
{
	if (!key) {
		throw std::invalid_argument("null key");
	}

	const Windows::BaseKey* winKey = static_cast<const Windows::Key*>(key);
	IValuePtr result;
	
	DWORD dwType = 0;
	DWORD cbData = 0;
	// get the key length for the first time
	LSTATUS Status = RegQueryValueEx(
		static_cast<HKEY>(winKey->getKeyHandle()),
		valueName,
		NULL,
		&dwType,
		NULL,
		&cbData
	);

	if (Status == ERROR_SUCCESS)
	{
		if (cbData)
		{
			std::vector<unsigned char> data;
			data.resize(cbData);

			Status = RegQueryValueEx(
				static_cast<HKEY>(winKey->getKeyHandle()),
				valueName,
				NULL,
				&dwType,
				data.data(),
				&cbData
			);

			if (Status != ERROR_SUCCESS) {
				throw std::system_error(std::error_code(Status, std::system_category()), "couldn't open registry key");
			}

			result = std::make_shared<Windows::Value>(valueName, getType(dwType), std::move(data));
		}
		else {
			result = std::make_shared<Windows::Value>(valueName, getType(dwType));
		}
	}
	else {
		throw std::system_error(std::error_code(Status, std::system_category()), "couldn't open registry key");
	}

	return result;
}

IValuePtr Platform::Windows::ValueManager::newValue(const wchar_t* valueName, const IDataType& dataType)
{
	return std::make_shared<Windows::Value>(valueName, dataType);
}

void Platform::Windows::ValueManager::setValue(const IKey* key, const IValue& value)
{
	const Windows::BaseKey* winKey = static_cast<const Windows::Key*>(key);
	const Platform::Windows::Value& winVal = *static_cast<const Platform::Windows::Value*>(&value);
	const Platform::Windows::DataType* winType = dynamic_cast<const Platform::Windows::DataType*>(&winVal.getType());
	if (!winType) {
		throw std::invalid_argument("invalid type");
	}

	const std::span<const unsigned char> rawData = winVal.getRawData();
	if (!rawData.empty())
	{
		RegSetValueExW(
			static_cast<HKEY>(winKey->getKeyHandle()),
			value.getValueName(),
			NULL,
			winType->getInternalType(),
			rawData.data(),
			static_cast<DWORD>(rawData.size())
		);
	}
	else
	{
		RegSetValueExW(
			static_cast<HKEY>(winKey->getKeyHandle()),
			value.getValueName(),
			NULL,
			winType->getInternalType(),
			NULL,
			0
		);
	}
}

bool Platform::Windows::KeyManager::visitKeys(const IKey* key, const KeyVisitor& visitor, size_t maxDepth)
{
	const Windows::BaseKey* winKey = static_cast<const Windows::Key*>(key);

	// use a single buffer for enumerating all keys
	std::wstring keyNameBuf;
	keyNameBuf.resize(255);

	return visitKeysInternal(winKey, visitor, keyNameBuf, maxDepth);
}

bool Platform::Windows::KeyManager::visitKeysInternal(const BaseKey* winKey, const KeyVisitor& visitor, std::wstring& nameBuffer, size_t currentDepth)
{
	LSTATUS Status;

	// enumerate subkeys
	DWORD dwIndex = 0;
	DWORD cchName = static_cast<DWORD>(nameBuffer.size());
	while ((Status = RegEnumKeyEx(
		static_cast<HKEY>(winKey->getKeyHandle()),
		dwIndex,
		nameBuffer.data(),
		&cchName,
		NULL,
		NULL,
		NULL,
		NULL
	)) != ERROR_NO_MORE_ITEMS)
	{
		if (Status == ERROR_INVALID_HANDLE) {
			break;
		}

		if (Status == ERROR_SUCCESS)
		{
			if (!std::invoke(visitor, winKey, nameBuffer.data()))
			{
				// stop enumerating
				return false;
			}

			if (currentDepth > 0)
			{
				const IKeyPtr enumeratingKey = getKey(nameBuffer.data(), Permission::Read, winKey);
				const Windows::BaseKey* winEnumeratingKey = static_cast<const Windows::Key*>(enumeratingKey.get());
				if (!visitKeysInternal(winEnumeratingKey, visitor, nameBuffer, currentDepth - 1))
				{
					// stop enumerating
					return false;
				}
			}
		}

		dwIndex++;
		cchName = static_cast<DWORD>(nameBuffer.size());
	}

	return true;
}

bool Platform::Windows::ValueManager::visitKeyValues(const IKey* key, const ValueVisitor& visitor)
{
	const Windows::BaseKey* winKey = static_cast<const Windows::Key*>(key);
	HKEY hKey = static_cast<HKEY>(winKey->getKeyHandle());

	std::wstring valueNameBuf;
	valueNameBuf.resize(16384);

	LSTATUS Status;
	DWORD dwIndex = 0;
	DWORD dwType = 0;
	DWORD cbData = 0;
	DWORD cchName = 32767;
	while ((Status = RegEnumValue(
		hKey,
		dwIndex,
		valueNameBuf.data(),
		&cchName,
		NULL,
		&dwType,
		NULL,
		&cbData
	)) != ERROR_NO_MORE_ITEMS)
	{
		if (cbData)
		{
			std::vector<unsigned char> dataBuf;
			dataBuf.resize(cbData);

			cchName = 32767;
			Status = RegEnumValue(
				hKey,
				dwIndex,
				valueNameBuf.data(),
				&cchName,
				NULL,
				&dwType,
				(LPBYTE)dataBuf.data(),
				&cbData
			);

			const Platform::Windows::Value value(valueNameBuf.data(), getType(dwType), std::move(dataBuf));
			if (!std::invoke(visitor, value)) {
				return false;
			}
		}
		else
		{
			// empty value
			const Platform::Windows::Value value(valueNameBuf.data(), getType(dwType));
			if (!std::invoke(visitor, value)) {
				return false;
			}
		}

		cchName = 32767;
		cbData = 0;
		dwIndex++;
	}

	return true;
}

Platform::Windows::DataType::DataType(const wchar_t* name, uint32_t internalType)
	:  IDataType(name)
	, type(internalType)
{

}

uint32_t Platform::Windows::DataType::getInternalType() const
{
	return type;
}

IKeyManager& Platform::Windows::RegistryManager::getKeyManager()
{
	return keyManager;
}

const IKeyManager& Platform::Windows::RegistryManager::getKeyManager() const
{
	return keyManager;
}

IValueManager& Platform::Windows::RegistryManager::getValueManager()
{
	return valueManager;
}

const IValueManager& Platform::Windows::RegistryManager::getValueManager() const
{
	return valueManager;
}

IHiveManager& Platform::Windows::RegistryManager::getHiveManager()
{
	return hiveManager;
}

const IHiveManager& Platform::Windows::RegistryManager::getHiveManager() const
{
	return hiveManager;
}

IHivePtr Platform::Windows::HiveManager::createHive(const std::filesystem::path& hiveFileName)
{
	std::wstring hiveName = hiveFileName.stem();

	/*
	constexpr std::wstring_view keyPrefix(L"tmp_container_");

	std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
	std::chrono::nanoseconds nanoTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch());
	std::wstring nanoTimeString = std::to_wstring(nanoTime.count());

	std::wstring keyName;
	keyName.reserve(keyPrefix.length() + hiveName.length() + 1 + nanoTimeString.length());
	keyName += keyPrefix;
	keyName += hiveName;
	keyName += L"_";
	keyName += std::move(nanoTimeString);

	if (Status != ERROR_SUCCESS) {
		throw std::system_error(std::error_code(Status, std::system_category()), "could not save key");
	}
	*/

	std::filesystem::remove(hiveFileName);

	HKEY hSubKey;
	const LSTATUS Status = RegLoadAppKey(hiveFileName.native().c_str(), &hSubKey, KEY_ALL_ACCESS, REG_PROCESS_APPKEY, NULL);
	if (Status != ERROR_SUCCESS) {
		throw std::system_error(std::error_code(Status, std::system_category()), "couldn't load key");
	}

	// The loaded key has no name, and it will be unloaded automatically when being closed
	const IKeyPtr key = std::make_shared<Platform::Windows::Key>(static_cast<HKEY>(hSubKey), L"");

	return std::make_shared<Platform::Windows::Hive>(key, std::move(hiveName));
}

IHivePtr Platform::Windows::HiveManager::loadHive(const std::filesystem::path& hiveFileName)
{
	HKEY hSubKey;
	const LSTATUS Status = RegLoadAppKey(hiveFileName.native().c_str(), &hSubKey, KEY_ALL_ACCESS, REG_PROCESS_APPKEY, NULL);
	if (Status != ERROR_SUCCESS) {
		throw std::system_error(std::error_code(Status, std::system_category()), "couldn't load key");
	}

	const IKeyPtr key = std::make_shared<Platform::Windows::Key>(static_cast<HKEY>(hSubKey), nullptr);

	return std::make_shared<Platform::Windows::Hive>(key, hiveFileName.stem().c_str());
}

void Platform::Windows::HiveManager::saveHive(const IHive& hive, const std::filesystem::path& hiveFileName)
{
	const Platform::Windows::Hive& winHive = static_cast<const Platform::Windows::Hive&>(hive);
	Platform::Windows::Key* winKey = static_cast<Platform::Windows::Key*>(hive.getRootKey().get());
	if (!winKey) {
		throw std::invalid_argument("invalid hive key");
	}

	// Temporarily enable the backup privileges to be able to save the key
	Privilege privilege(SE_BACKUP_NAME);

	std::filesystem::remove(hiveFileName);

	LSTATUS Status;
	Status = RegSaveKeyExW(static_cast<HKEY>(winKey->getKeyHandle()), hiveFileName.native().c_str(), NULL, REG_LATEST_FORMAT);
	if (Status != ERROR_SUCCESS) {
		throw std::system_error(std::error_code(Status, std::system_category()), "could not save hive to file");
	}

	// close the key before deleting it
	winKey->dispose();

	HKEY hCurrentUser;
	Status = RegOpenCurrentUser(KEY_ALL_ACCESS, &hCurrentUser);
	Status = RegDeleteTree(hCurrentUser, winKey->getKeyName());
	Status = RegCloseKey(hCurrentUser);
}
