#pragma once

#include <filesystem>
#include <string>
#include <span>
#include <memory>

#include <pugixml.hpp>

namespace Files
{
	namespace Config
	{
		class Component
		{
		public:
			Component(const std::wstring_view& componentName);

			const std::wstring& getComponentName() const;

		private:
			std::wstring name;
		};

		class Directory
		{
		public:
			Directory(const std::filesystem::path& inPath);

			const std::filesystem::path& getPath() const;

		private:
			std::filesystem::path path;
		};

		class HostDirectory
		{
		public:
			HostDirectory(const std::filesystem::path& inSourcePath, const std::filesystem::path& inTargetPath = {});

			const std::filesystem::path& getSourcePath() const;
			const std::filesystem::path& getTargetPath() const;

		private:
			std::filesystem::path sourcePath;
			std::filesystem::path targetPath;
		};

		class HostSxsFile
		{
		public:
			HostSxsFile(const std::filesystem::path& inSourcePath, const std::filesystem::path& inTargetPath = {});

			const std::filesystem::path& getSourcePath() const;
			const std::filesystem::path& getTargetPath() const;

		private:
			std::filesystem::path sourcePath;
			std::filesystem::path targetPath;
		};

		class HostSxs
		{
		public:
			HostSxs(const wchar_t* inNamePart);

			const std::wstring& getNamePart() const;
		private:
			std::wstring namePart;
		};

		class HostFile
		{
		public:
			HostFile(const std::filesystem::path& inSourceFile, const std::filesystem::path& inTargetFile = {});

			const std::filesystem::path& getSourceFile() const;
			const std::filesystem::path& getTargetFile() const;

		private:
			std::filesystem::path sourceFile;
			std::filesystem::path targetFile;
		};

		class IDirectoryVisitor
		{
		public:
			virtual ~IDirectoryVisitor() = default;
			virtual void visit(const HostDirectory& directory) = 0;
			virtual void visit(const HostDirectory& directory, const std::span<Component>& components) = 0;
		};
		using IDirectoryVisitorPtr = std::shared_ptr<IDirectoryVisitor>;

		class IFileVisitor
		{
		public:
			virtual ~IFileVisitor() = default;
			virtual void visit(const HostFile& file) = 0;
			virtual void visit(const HostSxs& sxs, const std::span<const HostSxsFile>& files) = 0;
		};
		using IFileVisitorPtr = std::shared_ptr<IFileVisitor>;

		class IndividualFileGroupReader
		{
		public:
			IndividualFileGroupReader(std::istream& inStream, const std::filesystem::path& inWorkingDir);

			void parse(const IFileVisitorPtr& fileVisitor, const IDirectoryVisitorPtr& directoryVisitor);

		private:
			std::istream& stream;
			std::filesystem::path workingDir;
		};

		class FilesGroupReader
		{
		public:
			FilesGroupReader(std::istream& inStream, const std::filesystem::path& inWorkingDir);

			void parse(const IFileVisitorPtr& fileVisitor, const IDirectoryVisitorPtr& directoryVisitor);

		private:
			std::istream& stream;
			std::filesystem::path workingDir;
		};

		class FileGroupParser
		{
		public:
			void parse(const IFileVisitorPtr& fileVisitor, const IDirectoryVisitorPtr& directoryVisitor, const pugi::xml_node& node);
			void parseSxsFiles(const IFileVisitorPtr& fileVisitor, const pugi::xml_node& node, const HostSxs& hostSxs);
		};

		extern FileGroupParser fileGroupParser;
	}
}
