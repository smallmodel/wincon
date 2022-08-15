#pragma once

#include "registry_data.h"

#include <string>
#include <filesystem>
#include <memory>

class Hive
{
public:
	Hive(const std::wstring& name, const std::filesystem::path& hiveDirectory);
	~Hive();

	static Hive create(const std::wstring& name, const std::filesystem::path& hiveDirectory);

	void* getHandle() const;
	void flush();

private:
	std::wstring hiveName;
	std::wstring keyName;
	std::filesystem::path dir;
	void* keyHandle;
};

namespace Registry
{
	class IHive
	{
	public:
		virtual ~IHive() = default;

		virtual const wchar_t* getHiveName() const = 0;
		virtual IKeyPtr getRootKey() const = 0;
	};
	using IHivePtr = std::shared_ptr<IHive>;

	class IHiveManager
	{
	public:
		virtual ~IHiveManager() = default;

		virtual IHivePtr createHive(const std::filesystem::path& hiveFileName) = 0;
		virtual IHivePtr loadHive(const std::filesystem::path& hiveFileName) = 0;
		virtual void saveHive(const IHive& hive, const std::filesystem::path& hiveFileName) = 0;
	};
}
