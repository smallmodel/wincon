#pragma once

#include "registry_data.h"
#include "registry_hive.h"

#include <span>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace Registry
{
	enum Permission
	{
		None = 0,
		Read = (1 << 0),
		Write = (1 << 1),
		Execute = (1 << 2),
		All = ~0ul
	};

	class IKeyVisitor
	{
	public:
		virtual ~IKeyVisitor() = default;
		virtual bool operator()(const IKey* parentKey, const wchar_t* subKeyName) const = 0;
	};

	class IValueVisitor
	{
	public:
		virtual ~IValueVisitor() = default;
		virtual bool operator()(const IValue& value) const = 0;
	};

	using KeyVisitor = std::function<bool(const IKey* parentKey, const wchar_t* subKeyName)>;
	using ValueVisitor = std::function<bool(const IValue& value)>;

	class IKeyManager
	{
	public:
		virtual ~IKeyManager() = default;

		/**
		 * Retrieves the specified key. NULL if it doesn't exist.
		 */
		virtual IKeyPtr getKey(const wchar_t* keyName, Permission permissions, const IKey* parentKey = nullptr) = 0;

		/**
		 * Gets or create a key.
		 */
		virtual IKeyPtr getOrCreateKey(const wchar_t* keyName, Permission permissions, const IKey* parentKey = nullptr) = 0;

		/**
		 * Enumerate keys and subkeys.
		 */
		virtual bool visitKeys(const IKey* key, const KeyVisitor& visitor, size_t maxDepth = ~0) = 0;
	};

	class IValueManager
	{
	public:
		virtual ~IValueManager() = default;

		/**
		 * Retrieves the specified value.
		 */
		virtual IValuePtr getValue(const IKey* key, const wchar_t* valueName = nullptr) = 0;

		/**
		 * Creates a new value to be assigned.
		 */
		virtual IValuePtr newValue(const wchar_t* valueName, const IDataType& dataType) = 0;

		/**
		 * Sets the value in the specified key.
		 */
		virtual void setValue(const IKey* key, const IValue& value) = 0;

		/**
		 * Enumerate all values of a key.
		 */
		virtual bool visitKeyValues(const IKey* key, const ValueVisitor& visitor) = 0;
	};

	class IRegistryManager
	{
	public:
		virtual ~IRegistryManager() = default;

		virtual IKeyManager& getKeyManager() = 0;
		virtual const IKeyManager& getKeyManager() const = 0;

		virtual IValueManager& getValueManager() = 0;
		virtual const IValueManager& getValueManager() const = 0;

		virtual IHiveManager& getHiveManager() = 0;
		virtual const IHiveManager& getHiveManager() const = 0;
	};

	namespace Helpers
	{
		/**
		 * Copy keys and values from source to target key
		 */
		void copyKeysValues(IRegistryManager& registryManager, const IKey* sourceKey, const IKey* targetKey, size_t maxDepth = ~0ul);
		
		/**
		 * Copy values from source key to target key
		 */
		void copyValues(IValueManager& valueManager, const IKey* sourceKey, const IKey* targetKey);
	}

	namespace Data
	{
		class IValueData
		{
		public:
			IValueData(const IValuePtr& inValue);

			IValue& getValue();
			const IValue& getValue() const;

		private:
			IValuePtr value;
		};

		class StringValue : public IValueData
		{
		public:
			using IValueData::IValueData;
	
			std::wstring_view get() const;
			void set(const std::wstring_view& stringValue);

		private:
			std::wstring stringData;
		};

		class BinaryData : public IValueData
		{
		public:
			using IValueData::IValueData;

			std::span<const unsigned char> get() const;
			void set(const std::span<const unsigned char>& binaryData);
		};

		class DWordData : public IValueData
		{
		public:
			using IValueData::IValueData;

			uint32_t get() const;
			void set(uint32_t intValue);
		};

		class QWordData : public IValueData
		{
		public:
			using IValueData::IValueData;

			uint64_t get() const;
			void set(uint64_t intValue);
		};
	}
}
