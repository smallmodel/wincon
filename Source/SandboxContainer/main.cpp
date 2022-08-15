#include "implementations/windows_storage.h"
#include "implementations/windows_container_runtime.h"
#include "implementations/windows_container_process.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <fstream>
#include <filesystem>

#include <conio.h>

static const std::filesystem::path DefaultContainerDirectory = L"\\ProgramData\\Containers";

Container::Storage::IStoragePtr createLayer()
{
	Container::Storage::WindowsStorageManager storageManager;
	Container::Storage::StorageOptions storageOptions;
	storageOptions.guid = {};
	storageOptions.path = L"C:\\ProgramData\\Containers\\TestContainer\\disk.vhdx";
	storageOptions.size = 34359738368;

	return storageManager.create(storageOptions);
}

void createContainer(const Container::Storage::IStoragePtr& storage, const std::wstring& containerName, const std::filesystem::path& containerPath)
{
	Container::Runtime::WindowsContainerManager containerManager;

	Container::Runtime::IContainerPtr container = containerManager.open(L"Sample");
	if (container) {
		container->terminate();
	}

	Container::Storage::LayerOptions layerOptions[] = {
		{
			{},
			containerPath,
			Container::Storage::pathType_e::AbsolutePath
		}
	};

	Container::Storage::AttachOptions attachOptions;
	attachOptions.layers = layerOptions;
	storage->attach(attachOptions);

	Container::Runtime::ContainerOptions containerOptions;
	containerOptions.guestOptions.hostName = containerName.c_str();
	containerOptions.memoryOptions.size = 2048;
	containerOptions.processorOptions.count = 1;

	containerOptions.storageOptions.layers = layerOptions;
	containerOptions.storageOptions.mountPath = storage->getAttachedStorageMountPath();

	container = containerManager.create(containerName.c_str(), containerOptions);

	// Start the container!
	container->start();

	Container::Runtime::WindowsProcessManager processManager;

	Container::Runtime::ProcessCreateOptions processCreateOptions;
	processCreateOptions.applicationName = L"C:\\Windows\\system32\\cmd.exe";
	processCreateOptions.userName = L"NT AUTHORITY\\SYSTEM";
	processCreateOptions.workingDir = L"C:\\Windows\\system32";
	processCreateOptions.bCreateStdInPipe = true;
	processCreateOptions.bCreateStdOutPipe = true;
	processCreateOptions.bCreateStdErrPipe = true;

	Container::Runtime::IProcessPtr process = processManager.createProcess(container, processCreateOptions);

	std::thread inThread([process]
		{
			std::ostream* inStream = process->getInputStream();

			while (!inStream->fail())
			{
				std::string line;
				std::getline(std::cin, line);

				inStream->write(line.c_str(), line.length());
				// write carriage return and line-feed
				inStream->write("\r\n", 2);
			}
		});

	std::thread outThread([process]
		{
			std::istream* stream = process->getOutputStream();

			while(!stream->fail())
			{
				char ch;
				stream->read(&ch, 1);

				std::cout << ch;
			}
		});

	inThread.join();
	outThread.join();

	/*
	HANDLE hCurrentStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hCurrentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	std::thread outThread([processInfo, hCurrentStdOut]
		{
			for (;;)
			{
				uint8_t data;
				ReadFile(processInfo.StdOutput, &data, 1, NULL, NULL);
				WriteFile(hCurrentStdOut, &data, 1, NULL, NULL);
			}
		});

	for (;;)
	{
		uint8_t data;
		ReadFile(hCurrentStdIn, &data, 1, NULL, NULL);
		WriteFile(processInfo.StdInput, &data, 1, NULL, NULL);
	}
	*/
}

int main(int argc, const char* argv[])
{
	std::filesystem::path containerPath;
	std::wstring containerHostName;

	TCLAP::CmdLine cmd("Container preparation tool", ' ');
	cmd.setExceptionHandling(false);

	try
	{
		TCLAP::ValueArg<std::string> containerDrivePathArg("p", "condir", "The container directory on the system drive", false, "", "string");
		TCLAP::ValueArg<std::string> containerNameArg("c", "name", "Container name", true, "", "string");
		TCLAP::ValueArg<std::string> containerHostNameArg("n", "hostname", "Container host name", false, "", "string");

		cmd.add(containerDrivePathArg);
		cmd.add(containerNameArg);
		cmd.add(containerHostNameArg);

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

		if (containerHostNameArg.isSet()) {
			const std::string val = containerHostNameArg.getValue();
			containerHostName = std::wstring(val.begin(), val.end());
		}
		else {
			const std::string val = containerNameArg.getValue();
			containerHostName = std::wstring(val.begin(), val.end());
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

	Container::Storage::IStoragePtr storage = createLayer();
	createContainer(storage, containerHostName, containerPath);
}
