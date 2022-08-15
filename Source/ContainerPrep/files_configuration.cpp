#include "files_configuration.h"

#include <fstream>

#include <pugixml.hpp>

using namespace Files;

Config::FileGroupParser Config::fileGroupParser;

Config::FilesGroupReader::FilesGroupReader(std::istream& inStream, const std::filesystem::path& inWorkingDir)
	: stream(inStream)
	, workingDir(inWorkingDir)
{

}

void Config::FilesGroupReader::parse(const IFileVisitorPtr& fileVisitor, const IDirectoryVisitorPtr& directoryVisitor)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load(stream);

	pugi::xml_node rootNode = doc.child("FilesGroups");
	for (pugi::xml_node::iterator subNode = rootNode.begin(); subNode != rootNode.end(); subNode++)
	{
		if (!std::strcmp(subNode->name(), "Group"))
		{
			const char* targetAttrA = subNode->attribute("Target").value();
			std::wstring targetAttr(targetAttrA, targetAttrA + std::strlen(targetAttrA));

			const std::filesystem::path path = workingDir / targetAttr;

			std::ifstream fileStream(path, std::ios::in | std::ios::binary);
			if (!fileStream.fail())
			{
				IndividualFileGroupReader config(fileStream, workingDir);
				config.parse(fileVisitor, directoryVisitor);
			}
		}
	}
}

Config::IndividualFileGroupReader::IndividualFileGroupReader(std::istream& inStream, const std::filesystem::path& inWorkingDir)
	: stream(inStream)
	, workingDir(inWorkingDir)
{
}

void Config::IndividualFileGroupReader::parse(const IFileVisitorPtr& fileVisitor, const IDirectoryVisitorPtr& directoryVisitor)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load(stream);

	fileGroupParser.parse(fileVisitor, directoryVisitor, doc.child("FileGroup"));
}

void Config::FileGroupParser::parse(const IFileVisitorPtr& fileVisitor, const IDirectoryVisitorPtr& directoryVisitor, const pugi::xml_node& node)
{
	for (pugi::xml_node::iterator subNode = node.begin(); subNode != node.end(); ++subNode)
	{
		if (!std::strcmp(subNode->name(), "HostDirectory"))
		{
			const char* sourceAttrA = subNode->attribute("Path").value();
			const char* targetAttrA = subNode->attribute("Target").value();
			std::wstring sourceAttr;
			std::wstring targetAttr;

			if (sourceAttrA) {
				sourceAttr = std::wstring(sourceAttrA, sourceAttrA + std::strlen(sourceAttrA));
			}

			if (targetAttrA) {
				targetAttr = std::wstring(targetAttrA, targetAttrA + std::strlen(targetAttrA));
			}

			HostDirectory hostDir(sourceAttr, targetAttr);
			std::vector<Component> components;

			size_t numComponents = 0;
			pugi::xml_object_range<pugi::xml_node_iterator> childComponents = subNode->children();
			// count the number of nodes first
			for (pugi::xml_node nodeComp : childComponents) {
				++numComponents;
			}

			components.reserve(numComponents);
			for (pugi::xml_node nodeComp : childComponents)
			{
				const char* compNameA = nodeComp.attribute("Name").value();

				components.push_back({
					std::wstring(compNameA, compNameA + std::strlen(compNameA))
				});
			}

			if (!components.size())
			{
				// no components
				directoryVisitor->visit(hostDir);
			}
			else {
				directoryVisitor->visit(hostDir, components);
			}
		}
		else if (!std::strcmp(subNode->name(), "HostFile"))
		{
			const char* sourceAttrA = subNode->attribute("Path").value();
			const char* targetAttrA = subNode->attribute("Target").value();
			std::wstring sourceAttr;
			std::wstring targetAttr;

			if (sourceAttrA) {
				sourceAttr = std::wstring(sourceAttrA, sourceAttrA + std::strlen(sourceAttrA));
			}

			if (targetAttrA) {
				targetAttr = std::wstring(targetAttrA, targetAttrA + std::strlen(targetAttrA));
			}

			HostFile hostFile(sourceAttr, targetAttr);
			fileVisitor->visit(hostFile);
		}
		else if (!std::strcmp(subNode->name(), "HostSxs"))
		{
			const char* namePartA = subNode->attribute("NamePart").value();
			const std::wstring namePart(namePartA, namePartA + std::strlen(namePartA));

			HostSxs hostSxs(namePart.c_str());
			parseSxsFiles(fileVisitor, *subNode, hostSxs);
		}
	}
}

void Config::FileGroupParser::parseSxsFiles(const IFileVisitorPtr& fileVisitor, const pugi::xml_node& node, const HostSxs& hostSxs)
{
	std::vector<HostSxsFile> files;

	uint64_t numFiles = 0;
	for (pugi::xml_node::iterator subNode = node.begin(); subNode != node.end(); ++subNode)
	{
		if (!std::strcmp(subNode->name(), "File")) {
			numFiles++;
		}
	}
	
	files.reserve(numFiles);
	for (pugi::xml_node::iterator subNode = node.begin(); subNode != node.end(); ++subNode)
	{
		if (!std::strcmp(subNode->name(), "File"))
		{
			const char* pathAttrA = subNode->attribute("Path").value();
			const char* targetAttrA = subNode->attribute("Target").value();
			const std::wstring sourceAttr(pathAttrA, pathAttrA + std::strlen(pathAttrA));
			const std::wstring targetAttr(targetAttrA, targetAttrA + std::strlen(targetAttrA));

			files.emplace_back(sourceAttr, targetAttrA);
		}
	}

	fileVisitor->visit(hostSxs, files);
}

Files::Config::Component::Component(const std::wstring_view& componentName)
	: name(componentName)
{
}

const std::wstring& Files::Config::Component::getComponentName() const
{
	return name;
}

Files::Config::Directory::Directory(const std::filesystem::path& inPath)
	: path(inPath)
{
}

const std::filesystem::path& Files::Config::Directory::getPath() const
{
	return path;
}

Files::Config::HostDirectory::HostDirectory(const std::filesystem::path& inSourcePath, const std::filesystem::path& inTargetPath)
	: sourcePath(inSourcePath)
	, targetPath(inTargetPath)
{

}

const std::filesystem::path& Files::Config::HostDirectory::getSourcePath() const
{
	if (!sourcePath.empty()) {
		return sourcePath;
	}
	else {
		return targetPath;
	}
}

const std::filesystem::path& Files::Config::HostDirectory::getTargetPath() const
{
	if (!targetPath.empty()) {
		return targetPath;
	}
	else {
		return sourcePath;
	}
}

Files::Config::HostFile::HostFile(const std::filesystem::path& inSourceFile, const std::filesystem::path& inTargetFile /*= {}*/)
	: sourceFile(inSourceFile)
	, targetFile(inTargetFile)
{
}

const std::filesystem::path& Files::Config::HostFile::getSourceFile() const
{
	if (!sourceFile.empty()) {
		return sourceFile;
	}
	else {
		return targetFile;
	}
}

const std::filesystem::path& Files::Config::HostFile::getTargetFile() const
{
	if (!targetFile.empty()) {
		return targetFile;
	}
	else {
		return sourceFile;
	}
}

Config::HostSxs::HostSxs(const wchar_t* inNamePart)
	: namePart(inNamePart)
{

}

const std::wstring& Config::HostSxs::getNamePart() const
{
	return namePart;
}

Config::HostSxsFile::HostSxsFile(const std::filesystem::path& inSourcePath, const std::filesystem::path& inTargetPath)
	: sourcePath(inSourcePath)
	, targetPath(inTargetPath)
{
}

const std::filesystem::path& Config::HostSxsFile::getSourcePath() const
{
	return sourcePath;
}

const std::filesystem::path& Config::HostSxsFile::getTargetPath() const
{
	return targetPath;
}
