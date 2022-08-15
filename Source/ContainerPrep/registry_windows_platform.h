#pragma once

#include "registry.h"

namespace Registry
{
	namespace Platform
	{
		namespace Windows
		{
			class BaseKey;

			class KeyManager : public IKeyManager
			{
			public:
				IKeyPtr getKey(const wchar_t* keyName, Permission permissions, const IKey* parentKey) override;
				IKeyPtr getOrCreateKey(const wchar_t* keyName, Permission permissions, const IKey* parentKey) override;
				bool visitKeys(const IKey* key, const KeyVisitor& visitor, size_t maxDepth = ~0) override;

			private:
				bool visitKeysInternal(const BaseKey* winKey, const KeyVisitor& visitor, std::wstring& nameBuffer, size_t currentDepth);
			};

			class ValueManager : public IValueManager
			{
			public:
				IValuePtr getValue(const IKey* key, const wchar_t* valueName) override;
				IValuePtr newValue(const wchar_t* valueName, const IDataType& dataType) override;
				void setValue(const IKey* key, const IValue& value) override;
				bool visitKeyValues(const IKey* key, const ValueVisitor& visitor) override;
			};

			class HiveManager : public IHiveManager
			{
			public:
				IHivePtr createHive(const std::filesystem::path& hiveFileName) override;
				IHivePtr loadHive(const std::filesystem::path& hiveFileName) override;
				void saveHive(const IHive& hive, const std::filesystem::path& hiveFileName) override;
			};

			class RegistryManager : public IRegistryManager
			{
			public:
				virtual IKeyManager& getKeyManager() override;
				virtual const IKeyManager& getKeyManager() const override;

				virtual IValueManager& getValueManager() override;
				virtual const IValueManager& getValueManager() const override;

				virtual IHiveManager& getHiveManager() override;
				virtual const IHiveManager& getHiveManager() const override;

			private:
				KeyManager keyManager;
				ValueManager valueManager;
				HiveManager hiveManager;
			};

			class DataType : public IDataType
			{
			public:
				DataType(const wchar_t* name, uint32_t internalType);

				uint32_t getInternalType() const;

			private:
				uint32_t type;
			};

			class Hive : public IHive
			{
			public:
				Hive(const IKeyPtr& inKey, std::wstring&& name);
				~Hive();

				Hive(const Hive&) = delete;
				Hive& operator=(const Hive&) = delete;
				Hive(Hive&& other);
				Hive& operator=(Hive&& other);

				const wchar_t* getHiveName() const override;
				IKeyPtr getRootKey() const override;

			private:
				IKeyPtr key;
				std::wstring hiveName;
			};

			class BaseKey : public IKey
			{
			public:
				BaseKey(void* inKeyHandle);

				BaseKey(const BaseKey&) = delete;
				BaseKey& operator=(const BaseKey&) = delete;
				BaseKey(BaseKey&& other);
				BaseKey& operator=(BaseKey&& other);

				void* getKeyHandle() const;

			protected:
				void* keyHandle;
			};

			class Key : public BaseKey
			{
			public:
				Key(void* inKeyHandle, const std::wstring_view& name);
				~Key();

				Key(const Key&) = delete;
				Key& operator=(const Key&) = delete;

				Key(Key&& other);
				Key& operator=(Key&& other);

				void dispose();
				const wchar_t* getKeyName() const override;

			private:
				std::wstring name;
			};

			class PredefinedKey : public BaseKey
			{
			public:
				PredefinedKey(void* inKeyHandle, const wchar_t* inName);
				~PredefinedKey();

				PredefinedKey(const PredefinedKey&) = delete;
				PredefinedKey& operator=(const PredefinedKey&) = delete;
				PredefinedKey(PredefinedKey&&) = delete;
				PredefinedKey& operator=(PredefinedKey&&) = delete;

				const wchar_t* getKeyName() const override;

			private:
				const wchar_t* name;
			};

			class Value : public IValue
			{
			public:
				Value(const wchar_t* valueName, const IDataType& type);
				Value(const wchar_t* valueName, const IDataType& type, std::vector<unsigned char>&& data);

				Value(const Value&) = delete;
				Value& operator=(const Value&) = delete;
				Value(Value&&) = delete;
				Value& operator=(Value&&) = delete;

				const wchar_t* getValueName() const override;
				const IDataType& getType() const override;

				std::span<const unsigned char> getRawData() const override;
				void setRawData(const std::span<const unsigned char>& data) override;

			private:
				std::wstring name;
				const IDataType& dataType;
				std::vector<unsigned char> rawData;
			};

			extern const DataType StringDataType;
			extern const DataType ExpandableStringDataType;
			extern const DataType MultiStringDataType;
			extern const DataType BinaryDataType;
			extern const DataType DWordDataType;
			extern const DataType QWordDataType;

			namespace PredefinedKeys
			{
				extern const IKeyPtr ClassesRoot;
				extern const IKeyPtr CurrentUser;
				extern const IKeyPtr LocalMachine;
				extern const IKeyPtr Users;
				extern const IKeyPtr CurrentConfig;

				IKeyPtr findByName(const wchar_t* name);
			}
		}

		namespace Host
		{
			using RegistryManager = Windows::RegistryManager;

			extern const IDataType& StringDataType;
			extern const IDataType& ExpandableStringDataType;
			extern const IDataType& MultiStringDataType;
			extern const IDataType& BinaryDataType;
			extern const IDataType& DWordDataType;
			extern const IDataType& QWordDataType;
		}
	}
}
