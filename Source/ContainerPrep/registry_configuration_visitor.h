#pragma once

#include "registry.h"
#include "registry_configuration.h"

namespace Registry
{
	class ValueConfigVisitor : public Config::IValueVisitor
	{
	public:
		ValueConfigVisitor(IRegistryManager& inRegistryManager, const IKeyPtr& inKey, const IKeyPtr& inHostKey = nullptr);

		void visit(const Config::Value& value) override;
		void visit(const Config::HostValue& hostValue) override;

	private:
		IKeyPtr key;
		IKeyPtr hostKey;
		IRegistryManager& registryManager;
	};

	class KeyConfigVisitor : public Config::IKeyVisitor
	{
	public:
		KeyConfigVisitor(IRegistryManager& inRegistryManager, const IHivePtr& inHive, const IKeyPtr& inHostHive);

		Config::IValueVisitorPtr visit(const Config::HostKey& hostKey) override;
		Config::IValueVisitorPtr visit(const Config::Key& key) override;

	private:
		IHivePtr hive;
		IKeyPtr hostHive;
		IRegistryManager& registryManager;
	};

	class HiveConfigVisitor : public Config::IHiveVisitor
	{
	public:
		HiveConfigVisitor(IRegistryManager& inRegistryManager, const std::filesystem::path& inWorkingDir, const std::wstring_view& hivePostfix = nullptr);

		Config::IKeyVisitorPtr visit(const Config::Hive& hive) override;

	private:
		std::filesystem::path workingDir;
		std::wstring postfix;
		IRegistryManager& registryManager;
	};
}
