#include "registry_configuration_visitor.h"
#include "registry_windows_platform.h"

using namespace Registry;

HiveConfigVisitor::HiveConfigVisitor(IRegistryManager& inRegistryManager, const std::filesystem::path& inWorkingDir, const std::wstring_view& hivePostfix)
	: registryManager(inRegistryManager)
	, workingDir(inWorkingDir)
	, postfix(hivePostfix)
{
}

Registry::Config::IKeyVisitorPtr HiveConfigVisitor::visit(const Config::Hive& hive)
{
	const IKeyPtr predefinedKey = Platform::Windows::PredefinedKeys::findByName(hive.getRootName().c_str());
	IKeyPtr hostSourceKey;
	try
	{
		hostSourceKey = registryManager.getKeyManager().getKey(hive.getRootHiveName().c_str(), Permission::Read, predefinedKey.get());
	}
	catch (const std::exception&) {
	}

	std::filesystem::path hiveFilename;
	if (!postfix.empty()) {
		hiveFilename = workingDir / (hive.getName() + postfix);
	}
	else {
		hiveFilename = workingDir / hive.getName();
	}

	IHivePtr hivePtr = registryManager.getHiveManager().createHive(hiveFilename);
	return std::make_shared<KeyConfigVisitor>(registryManager, hivePtr, hostSourceKey);
}

KeyConfigVisitor::KeyConfigVisitor(IRegistryManager& inRegistryManager, const IHivePtr& inHive, const IKeyPtr& inHostHive)
	: registryManager(inRegistryManager)
	, hive(inHive)
	, hostHive(inHostHive)
{
}

Registry::Config::IValueVisitorPtr KeyConfigVisitor::visit(const Config::HostKey& hostKey)
{
	IKeyManager& keyManager = registryManager.getKeyManager();
	IKeyPtr sourceKey = keyManager.getKey(hostKey.getHostPath().c_str(), Permission::Read, hostHive.get());
	IKeyPtr targetKey = keyManager.getOrCreateKey(hostKey.getTargetPath().c_str(), Permission::All, hive->getRootKey().get());

	// copy source -> target
	Helpers::copyKeysValues(registryManager, sourceKey.get(), targetKey.get());

	return std::make_shared<ValueConfigVisitor>(registryManager, targetKey);
}

Registry::Config::IValueVisitorPtr KeyConfigVisitor::visit(const Config::Key& key)
{
	IKeyManager& keyManager = registryManager.getKeyManager();
	const IKeyPtr createdKey = keyManager.getOrCreateKey(key.getPath().c_str(), Permission::All, hive->getRootKey().get());
	try
	{
		const IKeyPtr hostKey = keyManager.getKey(key.getPath().c_str(), Permission::Read, hostHive.get());
		return std::make_shared<ValueConfigVisitor>(registryManager, createdKey, hostKey);
	}
	catch(const std::exception&)
	{
		return std::make_shared<ValueConfigVisitor>(registryManager, createdKey);
	}

}

ValueConfigVisitor::ValueConfigVisitor(IRegistryManager& inRegistryManager, const IKeyPtr& inKey, const IKeyPtr& inHostKey)
	: registryManager(inRegistryManager)
	, key(inKey)
	, hostKey(inHostKey)
{

}

void ValueConfigVisitor::visit(const Config::Value& value)
{
	IValueManager& valueManager = registryManager.getValueManager();
	const IValuePtr createdValue = valueManager.newValue(value.getValueName().c_str(), *value.getDataType());
	createdValue->setRawData(value.getData());
	valueManager.setValue(key.get(), *createdValue);
}

void ValueConfigVisitor::visit(const Config::HostValue& hostValue)
{
	IValueManager& valueManager = registryManager.getValueManager();
	if (hostKey)
	{
		const IValuePtr hostValuePtr = valueManager.getValue(hostKey.get(), hostValue.getHostValueName().c_str());

		const IValuePtr createdValue = valueManager.newValue(hostValue.getValueName().c_str(), hostValuePtr->getType());
		if (hostValuePtr)
		{
			// copy value from host
			createdValue->setRawData(hostValuePtr->getRawData());
		}

		valueManager.setValue(key.get(), *createdValue);
	}
}
