#include "files_configuration_visitor.h"
#include "hard_link_iterator.h"
#include "privilege_manager.h"

#include <Windows.h>

#include <cstdlib>

const std::filesystem::path systemDrive = std::getenv("SystemDrive");
const std::filesystem::path systemRoot = std::getenv("SystemRoot");
const std::filesystem::path sxsDir = systemRoot / L"WinSxS";

using namespace Files;

static void linkTo(
	const std::filesystem::path& target,
	const std::filesystem::path& linkPath
)
{
	std::error_code ec;
	const uintmax_t count = std::filesystem::hard_link_count(linkPath, ec);
	if (count == static_cast<uintmax_t>(-1) || !count)
	{
		const std::filesystem::path directory = linkPath.parent_path();

		std::filesystem::create_directories(directory);
		std::filesystem::create_hard_link(target, linkPath, ec);
	}
}

FilesVisitor::FilesVisitor(const std::filesystem::path& inWorkingDir)
	: workingDir(inWorkingDir)
{
}

void FilesVisitor::visit(const Config::HostFile& file)
{
	// requires this privilege to create hard links
	Privilege privilege(SE_RESTORE_NAME);

	const std::filesystem::path sourcePath = systemDrive / file.getSourceFile();
	const std::filesystem::path targetPath = workingDir / (file.getTargetFile().native().c_str() + 1);

	linkTo(sourcePath, targetPath);
}

void FilesVisitor::visit(const Config::HostSxs& sxs, const std::span<const Config::HostSxsFile>& files)
{
	// requires this privilege to create hard links
	Privilege privilege(SE_RESTORE_NAME);

	const std::filesystem::path sxsDirName = sxsDir / sxs.getNamePart();

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(sxsDir, std::filesystem::directory_options::skip_permission_denied))
	{
		try
		{
			if (!entry.is_directory()) {
				continue;
			}
		}
		catch (const std::filesystem::filesystem_error&)
		{
			// Skip files that cannot be accessed
			continue;
		}

		const std::filesystem::path& entryPath = entry.path();
		if (!entryPath.native().find(workingDir)) {
			continue;
		}

		if (!entryPath.native().find(sxsDirName))
		{
			// found a matching sxs component

			for (const std::filesystem::directory_entry& fileEntry : std::filesystem::directory_iterator(entryPath, std::filesystem::directory_options::skip_permission_denied))
			{
				if (!fileEntry.is_regular_file()) {
					continue;
				}

				const std::filesystem::path& filePath = fileEntry.path();
				const std::filesystem::path fileRelPath = L"\\" / std::filesystem::relative(filePath, entryPath);

				for (const Config::HostSxsFile& sxsFile : files)
				{
					if (sxsFile.getSourcePath() == fileRelPath)
					{
						const std::filesystem::path linkPath = workingDir / (sxsFile.getTargetPath().native().c_str() + 1);

						linkTo(filePath, linkPath);
					}
				}
			}
		}
	}
}

void FilesVisitor::visit(const Config::HostDirectory& directory, const std::span<Config::Component>& components)
{
	// requires this privilege to create hard links
	Privilege privilege(SE_RESTORE_NAME);

	const std::filesystem::path sourcePath = systemDrive / directory.getSourcePath();
	const std::filesystem::path targetPath = workingDir / (directory.getTargetPath().native().c_str() + 1);

	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sourcePath, std::filesystem::directory_options::skip_permission_denied))
	{
		try
		{
			if (!entry.is_regular_file()) {
				continue;
			}
		}
		catch (const std::filesystem::filesystem_error&)
		{
			// Skip files that cannot be accessed
			continue;
		}

		const std::filesystem::path& entryPath = entry.path();
		if (!entryPath.native().find(workingDir)) {
			continue;
		}

		std::wstring sxs;
		for (hard_link_iterator iterator(entryPath); iterator; ++iterator)
		{
			std::wstring tmp = iterator->native();
			if (!tmp.find(L"\\Windows\\WinSxS"))
			{
				for (const Config::Component& component : components)
				{
					if (!tmp.find(L"\\Windows\\WinSxS\\" + component.getComponentName()))
					{
						// found an SXS component
						sxs = systemDrive / tmp;
						break;
					}
				}
			}
		}

		if (!sxs.empty())
		{
			const std::filesystem::path relPath = std::filesystem::relative(entryPath, sourcePath);
			const std::filesystem::path linkPath = targetPath / relPath;

			// create a new hard link if it doesn't exist
			linkTo(entryPath, linkPath);
		}
	}
}

void FilesVisitor::visit(const Config::HostDirectory& directory)
{
	// requires this privilege to create hard links
	Privilege privilege(SE_RESTORE_NAME);

	const std::filesystem::path sourcePath = systemDrive / directory.getSourcePath();
	const std::filesystem::path targetPath = workingDir / (directory.getTargetPath().native().c_str() + 1);

	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sourcePath, std::filesystem::directory_options::skip_permission_denied))
	{
		try
		{
			if (!entry.is_regular_file()) {
				continue;
			}
		}
		catch (const std::filesystem::filesystem_error&)
		{
			// Skip files that cannot be accessed
			continue;
		}

		const std::filesystem::path& entryPath = entry.path();
		if (!entryPath.native().find(workingDir)) {
			continue;
		}

		const std::filesystem::path relPath = std::filesystem::relative(entryPath, sourcePath);
		const std::filesystem::path linkPath = targetPath / relPath;

		// create a new hard link if it doesn't exist
		linkTo(entryPath, linkPath);
	}
}

