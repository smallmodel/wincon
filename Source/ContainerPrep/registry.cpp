#include "registry.h"

using namespace Registry;

IDataType* IDataType::head = nullptr;
IDataType* IDataType::last = nullptr;

IDataType::IDataType(const wchar_t* name)
	: typeName(name)
{
	if (!head) head = this;
	if (last) last->next = this;
	prev = last;
	next = nullptr;
	last = this;
}

IDataType::~IDataType()
{
	if (next) next->prev = prev;
	if (prev) prev->next = next;
	if (head == this) head = next;
	if (last == this) last = prev;
}

const IDataType* IDataType::findDataType(const wchar_t* name)
{
	for (const IDataType* dt = head; dt; dt = dt->next)
	{
		if (!_wcsicmp(dt->typeName, name)) {
			return dt;
		}
	}

	return nullptr;
}

IValue::IValue()
{
}

IValue::~IValue()
{
}

Data::IValueData::IValueData(const IValuePtr& inValue)
	: value(inValue)
{
}

const IValue& Data::IValueData::getValue() const
{
	return *value.get();
}

IValue& Data::IValueData::getValue()
{
	return *value.get();
}

std::wstring_view Data::StringValue::get() const
{
	const std::span<const unsigned char> data = getValue().getRawData();
	return std::wstring_view((const wchar_t*)data.data(), (const wchar_t*)(data.data() + data.size()));
}

void Data::StringValue::set(const std::wstring_view& stringValue)
{
	getValue().setRawData(std::span<const unsigned char>(
		(const unsigned char*)stringValue.data(),
		(const unsigned char*)(stringValue.data() + stringValue.length())
		)
	);
}

std::span<const unsigned char> Data::BinaryData::get() const
{
	return getValue().getRawData();
}

void Data::BinaryData::set(const std::span<const unsigned char>& binaryData)
{
	getValue().setRawData(binaryData);
}

uint32_t Data::DWordData::get() const
{
	const std::span<const unsigned char> data = getValue().getRawData();
	return *(const uint32_t*)data.data();
}

void Data::DWordData::set(uint32_t intValue)
{
	getValue().setRawData(std::span<const unsigned char>(
		(const unsigned char*)&intValue,
		sizeof(intValue)
		)
	);
}

uint64_t Data::QWordData::get() const
{
	const std::span<const unsigned char> data = getValue().getRawData();
	return *(const uint64_t*)data.data();
}

void Data::QWordData::set(uint64_t intValue)
{
	getValue().setRawData(std::span<const unsigned char>(
		(const unsigned char*)&intValue,
		sizeof(intValue)
		)
	);
}

void Helpers::copyKeysValues(IRegistryManager& registryManager, const IKey* sourceKey, const IKey* targetKey, size_t maxDepth)
{
	// copy values for the root key
	IKeyManager& keyManager = registryManager.getKeyManager();
	IValueManager& valueManager = registryManager.getValueManager();

	copyValues(valueManager, sourceKey, targetKey);

	keyManager.visitKeys(sourceKey, [&registryManager, &keyManager, targetKey](const IKey* parentKey, const wchar_t* keyName) -> bool
		{
			try
			{
				const IKeyPtr subSourceKey = keyManager.getKey(keyName, Permission::Read, parentKey);
				const IKeyPtr subTargetKey = keyManager.getOrCreateKey(keyName, Permission::Write, targetKey);

				copyKeysValues(registryManager, subSourceKey.get(), subTargetKey.get());
			}
			catch (const std::exception&)
			{
			}

			return true;
		}, 0);
}

void Helpers::copyValues(IValueManager& valueManager, const IKey* sourceKey, const IKey* targetKey)
{
	valueManager.visitKeyValues(sourceKey, [&valueManager, targetKey](const IValue& value) -> bool
		{
			// set the same value to the target key
			valueManager.setValue(targetKey, value);

			return true;
		});
}
