#include "hard_link_iterator.h"
#include "registry_hive.h"
#include "privilege_manager.h"

#include "registry.h"
#include "registry_windows_platform.h"
#include "registry_configuration.h"
#include "registry_configuration_visitor.h"
#include "files_configuration_visitor.h"

#include <tclap/CmdLine.h>

#include <filesystem>
#include <iostream>
#include <fstream>

#include <Windows.h>

static const std::filesystem::path DefaultContainerDirectory = L"\\ProgramData\\Containers";
static const std::filesystem::path DefaultSettingsDirectory = L".\\Settings";

int main(int argc, const char* argv[])
{
	std::filesystem::path containerPath;
	std::filesystem::path settingsDir;

	TCLAP::CmdLine cmd("Container preparation tool", ' ');
	cmd.setExceptionHandling(false);

	try
	{
		TCLAP::ValueArg<std::string> containerDrivePathArg("p", "condir", "The container directory on the system drive", false, "", "string");
		TCLAP::ValueArg<std::string> containerNameArg("c", "name", "Container name", true, "", "string");
		TCLAP::ValueArg<std::string> settingsDirArg("s", "settings", "Settings path", false, "", "string");

		cmd.add(containerDrivePathArg);
		cmd.add(containerNameArg);
		cmd.add(settingsDirArg);

		cmd.parse(argc, argv);

		std::filesystem::path containerDir;
		const char* systemDrive = std::getenv("SystemDrive");
		std::wstring systemDriveW(systemDrive, systemDrive + std::strlen(systemDrive));

		if (containerDrivePathArg.isSet())
		{
			containerDir = systemDrive + containerDrivePathArg.getValue();
		}
		else
		{
			// default container directory
			containerDir = systemDrive / DefaultContainerDirectory;
		}

		containerPath = containerDir / containerNameArg.getValue();

		if (settingsDirArg.isSet())
		{
			settingsDir = settingsDirArg.getValue();
		}
		else {
			settingsDir = DefaultSettingsDirectory;
		}
	}
	catch (const TCLAP::ArgException& e)
	{
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "error: " << e.what() << std::endl;
		return 2;
	}
	catch (...)
	{
		return 3;
	}

	Registry::Platform::Host::RegistryManager registryManager;

	const std::filesystem::path containerFilesPath = containerPath / L"Files";
	const std::filesystem::path containerHivesPath = containerPath / L"Hives";

	std::filesystem::create_directories(containerFilesPath);
	std::filesystem::create_directories(containerHivesPath);

	// Read hives configuration
	std::ifstream hivesConf(settingsDir / L"hives.xml", std::ios::in | std::ios::binary);
	Registry::Config::HivesConfigReader hivesReader(hivesConf, settingsDir);

	Registry::Config::IHiveVisitorPtr visitor(new Registry::HiveConfigVisitor(registryManager, containerHivesPath, L"_BASE"));
	hivesReader.parse(visitor);

	// Read files configuration
	std::ifstream filesConf(settingsDir / L"file_groups.xml", std::ios::in | std::ios::binary);
	Files::Config::FilesGroupReader filesReader(filesConf, settingsDir);

	Files::FilesVisitorPtr fileVisitor(new Files::FilesVisitor(containerFilesPath));
	filesReader.parse(fileVisitor, fileVisitor);

	return 0;
}