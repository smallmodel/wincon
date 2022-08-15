#pragma once

#include "configurator.h"
#include "registry.h"

#include <filesystem>

#include <pugixml.hpp>

namespace Registry
{
	class IDataType;

	namespace Config
	{
		class Hive
		{
		public:
			Hive(const std::wstring_view& hiveName, const std::wstring_view& hostRootName = {}, const std::wstring_view& hostHiveName = {});

			const std::wstring& getName() const;
			const std::wstring& getRootName() const;
			const std::wstring& getRootHiveName() const;

		private:
			std::wstring name;
			std::wstring rootName;
			std::wstring rootHive;
		};

		class Key
		{
		public:
			Key(const std::wstring& keyPath);

			const std::wstring& getPath() const;

		private:
			std::wstring path;
		};

		class HostKey
		{
		public:
			HostKey(const std::wstring_view& keyPath, const std::wstring_view& targetPath = {});

			const std::wstring& getHostPath() const;
			const std::wstring& getTargetPath() const;

		private:
			std::wstring sourcePath;
			std::wstring targetPath;
		};

		class HostValue
		{
		public:
			HostValue(const std::wstring_view& valueName, const std::wstring_view& targetValueName = {});

			const std::wstring& getHostValueName() const;
			const std::wstring& getValueName() const;

		private:
			std::wstring name;
			std::wstring targetName;
		};

		class Value
		{
		public:
			Value(const std::wstring& valueName, const IDataType* dataType, const std::span<const unsigned char>& inData = std::span<const unsigned char>());

			const std::wstring& getValueName() const;
			const IDataType* getDataType() const;
			std::span<const unsigned char> getData() const;

		private:
			std::wstring name;
			const IDataType* type;
			std::span<const unsigned char> data;
		};

		class IValueVisitor
		{
		public:
			virtual ~IValueVisitor() = default;

			virtual void visit(const Value& value) = 0;
			virtual void visit(const HostValue& hostValue) = 0;
		};
		using IValueVisitorPtr = std::shared_ptr<IValueVisitor>;

		class IKeyVisitor
		{
		public:
			virtual ~IKeyVisitor() = default;

			virtual IValueVisitorPtr visit(const HostKey& hostKey) = 0;
			virtual IValueVisitorPtr visit(const Key& key) = 0;
		};
		using IKeyVisitorPtr = std::shared_ptr<IKeyVisitor>;

		class IHiveVisitor
		{
		public:
			virtual ~IHiveVisitor() = default;

			virtual IKeyVisitorPtr visit(const Hive& hive) = 0;
		};
		using IHiveVisitorPtr = std::shared_ptr<IHiveVisitor>;

		class IParser
		{
		public:
			virtual ~IParser() = default;
		};

		class KeyParser
		{
		public:
			void parse(const IValueVisitorPtr& valueVisitor, const pugi::xml_node& node);
		};

		class HiveParser
		{
		public:
			void parse(const IKeyVisitorPtr& keyVisitor, const pugi::xml_node& node);
		};

		class IndividualConfigReader
		{
		public:
			IndividualConfigReader(std::istream& inReader);

			void parse(const IHiveVisitorPtr& visitor);

		private:
			std::istream& reader;
		};

		class HivesConfigReader
		{
		public:
			HivesConfigReader(std::istream& inReader, const std::filesystem::path& inWorkingDir);

			void parse(const IHiveVisitorPtr& visitor);

		private:
			std::istream& reader;
			std::filesystem::path workingDir;
		};
	}
}
