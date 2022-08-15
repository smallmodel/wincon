#pragma once

#include "files_configuration.h"

namespace Files
{
	class FilesVisitor : public Config::IFileVisitor, public Config::IDirectoryVisitor
	{
	public:
		FilesVisitor(const std::filesystem::path& inWorkingDir);

		void visit(const Config::HostFile& file) override;
		void visit(const Config::HostSxs& sxs, const std::span<const Config::HostSxsFile>& files) override;
		void visit(const Config::HostDirectory& directory) override;
		void visit(const Config::HostDirectory& directory, const std::span<Config::Component>& components) override;

	private:
		std::filesystem::path workingDir;
	};
	using FilesVisitorPtr = std::shared_ptr<FilesVisitor>;
}
