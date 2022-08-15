#include "registry_configuration.h"
#include "registry_windows_platform.h"

#include <set>
#include <fstream>

using namespace Registry;

Config::KeyParser keyParser;
Config::HiveParser hiveParser;

Config::IndividualConfigReader::IndividualConfigReader(std::istream& inReader)
	: reader(inReader)
{
}

void Config::IndividualConfigReader::parse(const IHiveVisitorPtr& hiveVisitor)
{
	IKeyVisitorPtr keyVisitor;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load(reader);

	pugi::xml_node hiveNode = doc.child("Hive");

	const char* hiveNameA = hiveNode.attribute("Name").value();
	std::wstring hiveName(hiveNameA, hiveNameA + std::strlen(hiveNameA));

	pugi::xml_attribute rootAttr = hiveNode.attribute("HostRoot");
	pugi::xml_attribute pathAttr = hiveNode.attribute("HostPath");
	if (!rootAttr.empty() && !pathAttr.empty())
	{
		const char* rootAttrA = rootAttr.value();
		const char* pathAttrA = pathAttr.value();
		std::wstring rootAttrStr(rootAttrA, rootAttrA + std::strlen(rootAttrA));
		std::wstring pathAttrStr(pathAttrA, pathAttrA + std::strlen(pathAttrA));

		keyVisitor = hiveVisitor->visit(Hive(hiveName, rootAttrStr, pathAttrStr));
	}
	else
	{
		keyVisitor = hiveVisitor->visit(Hive(hiveName));
	}

	hiveParser.parse(keyVisitor, hiveNode);
}

Config::HivesConfigReader::HivesConfigReader(std::istream& inReader, const std::filesystem::path& inWorkingDir)
	: reader(inReader)
	, workingDir(inWorkingDir)
{
}

void Config::HivesConfigReader::parse(const IHiveVisitorPtr& visitor)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load(reader);

	pugi::xml_node rootNode = doc.child("Hives");
	for (pugi::xml_node::iterator subNode = rootNode.begin(); subNode != rootNode.end(); subNode++)
	{
		if (!std::strcmp(subNode->name(), "Hive"))
		{
			pugi::xml_attribute targetAttr = subNode->attribute("Target");
			const char* targetAttrA = targetAttr.value();
			const std::wstring targetPath(targetAttrA, targetAttrA + std::strlen(targetAttrA));

			const std::filesystem::path path = workingDir / targetPath;

			std::ifstream fileStream(path, std::ios::in | std::ios::binary);
			if (!fileStream.fail())
			{
				IndividualConfigReader config(fileStream);
				config.parse(visitor);
			}
		}
	}
}

Config::Hive::Hive(const std::wstring_view& hiveName, const std::wstring_view& hostRootName, const std::wstring_view& hostHiveName)
	: name(hiveName)
	, rootName(hostRootName)
	, rootHive(hostHiveName)
{

}

const std::wstring& Config::Hive::getName() const
{
	return name;
}

const std::wstring& Config::Hive::getRootName() const
{
	return rootName;
}

const std::wstring& Config::Hive::getRootHiveName() const
{
	return rootHive;
}

Config::Key::Key(const std::wstring& keyPath)
	: path(keyPath)
{

}

const std::wstring& Config::Key::getPath() const
{
	return path;
}

Config::HostKey::HostKey(const std::wstring_view& hostPath, const std::wstring_view& targetPath)
	: sourcePath(hostPath)
	, targetPath(targetPath)
{

}

const std::wstring& Config::HostKey::getHostPath() const
{
	return sourcePath;
}

const std::wstring& Config::HostKey::getTargetPath() const
{
	return targetPath.empty() ? sourcePath : targetPath;
}

Config::HostValue::HostValue(const std::wstring_view& valueName, const std::wstring_view& targetValueName)
	: name(valueName)
	, targetName(targetValueName)
{

}

const std::wstring& Config::HostValue::getHostValueName() const
{
	return name;
}

const std::wstring& Config::HostValue::getValueName() const
{
	return !targetName.empty() ? targetName : name;
}

Config::Value::Value(const std::wstring& valueName, const IDataType* dataType, const std::span<const unsigned char>& inData)
	: name(valueName)
	, type(dataType)
	, data(inData)
{

}

const std::wstring& Config::Value::getValueName() const
{
	return name;
}

const Registry::IDataType* Config::Value::getDataType() const
{
	return type;
}

std::span<const unsigned char> Config::Value::getData() const
{
	return std::span<const unsigned char>(data.begin(), data.end());
}

void Config::HiveParser::parse(const IKeyVisitorPtr& keyVisitor, const pugi::xml_node& node)
{
	IValueVisitorPtr valueVisitor;

	for (pugi::xml_node::iterator subNode = node.begin(); subNode != node.end(); subNode++)
	{
		if (!std::strcmp(subNode->name(), "Key"))
		{
			pugi::xml_attribute pathAttribute = subNode->attribute("Path");
			const char* pathAttributeA = pathAttribute.value();
			if (*pathAttributeA == '\\' || *pathAttributeA == '/') pathAttributeA++;

			Key key(std::wstring(pathAttributeA, pathAttributeA + std::strlen(pathAttributeA)));
			valueVisitor = keyVisitor->visit(key);

			keyParser.parse(valueVisitor, *subNode);
		}
		else if (!std::strcmp(subNode->name(), "HostKey"))
		{
			pugi::xml_attribute pathAttribute = subNode->attribute("Path");
			const char* pathAttributeA = pathAttribute.value();
			if (*pathAttributeA == '\\' || *pathAttributeA == '/') pathAttributeA++;

			pugi::xml_attribute targetPathAttribute = subNode->attribute("TargetPath");

			if (targetPathAttribute.empty())
			{
				HostKey hostKey(std::wstring(pathAttributeA, pathAttributeA + strlen(pathAttributeA)));
				valueVisitor = keyVisitor->visit(hostKey);
			}
			else
			{
				const char* targetPathAttributeA = targetPathAttribute.value();
				if (*targetPathAttributeA == '\\' || *targetPathAttributeA == '/') targetPathAttributeA++;
				HostKey hostKey(
					std::wstring(pathAttributeA, pathAttributeA + std::strlen(pathAttributeA)),
					std::wstring(targetPathAttributeA, targetPathAttributeA + std::strlen(targetPathAttributeA))
				);

				valueVisitor = keyVisitor->visit(hostKey);
			}

			keyParser.parse(valueVisitor, *subNode);
		}
	}
}

void Config::KeyParser::parse(const IValueVisitorPtr& valueVisitor, const pugi::xml_node& node)
{
	for (pugi::xml_node::iterator subNode = node.begin(); subNode != node.end(); subNode++)
	{
		if (!std::strcmp(subNode->name(), "HostValue"))
		{
			pugi::xml_attribute nameAttribute = subNode->attribute("Name");
			const char* nameAttributeA = nameAttribute.value();

			pugi::xml_attribute targetAttribute = subNode->attribute("Target");

			if (targetAttribute.empty())
			{
				valueVisitor->visit(
					HostValue(std::wstring(nameAttributeA, nameAttributeA + std::strlen(nameAttributeA)))
				);
			}
			else
			{
				const char* targetAttributeA = targetAttribute.value();

				valueVisitor->visit(
					HostValue(
						std::wstring(nameAttributeA, nameAttributeA + std::strlen(nameAttributeA)),
						std::wstring(targetAttributeA, targetAttributeA + std::strlen(targetAttributeA))
					)
				);
			}
		}
		else if (!std::strcmp(subNode->name(), "Value"))
		{
			// parse value
			pugi::xml_attribute nameAttribute = subNode->attribute("Name");
			const char* nameAttributeA = nameAttribute.value();

			pugi::xml_attribute dataTypeAttr = subNode->attribute("DataType");
			const char* dataTypeAttrA = dataTypeAttr.value();
			std::wstring dataType(dataTypeAttrA, dataTypeAttrA + std::strlen(dataTypeAttrA));
			const IDataType* dt = IDataType::findDataType(dataType.c_str());

			pugi::xml_attribute dataAttr = subNode->attribute("Data");
			const char* dataAttrA = dataAttr.value();
			std::wstring strData;
			union {
				uint32_t dwordData;
				uint64_t qwordData;
			};
			std::span<const unsigned char> data;

			if (dt == &Platform::Host::StringDataType
				|| dt == &Platform::Host::MultiStringDataType
				|| dt == &Platform::Host::ExpandableStringDataType)
			{
				strData = std::wstring(dataAttrA, dataAttrA + std::strlen(dataAttrA));
				data = std::span<const unsigned char>(
					(const unsigned char*)strData.c_str(),
					(const unsigned char*)(strData.c_str() + strData.length() + 1)
				);
			}
			else if (dt == &Platform::Host::DWordDataType)
			{
				dwordData = std::stoi(dataAttrA);
				data = std::span<const unsigned char>((const unsigned char*)&dwordData, sizeof(dwordData));
			}
			else if (dt == &Platform::Host::QWordDataType)
			{
				qwordData = std::stoi(dataAttrA);
				data = std::span<const unsigned char>((const unsigned char*)&qwordData, sizeof(qwordData));
			}
			else
			{
				strData = std::wstring(dataAttrA, dataAttrA + std::strlen(dataAttrA));
				data = std::span<const unsigned char>(
					(const unsigned char*)strData.c_str(),
					(const unsigned char*)(strData.c_str() + strData.length() + 1)
				);
			}
			
			valueVisitor->visit(
				Value(
					std::wstring(nameAttributeA, nameAttributeA + std::strlen(nameAttributeA)),
					dt,
					data
				)
			);
		}
	}
}
