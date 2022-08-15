#include "windows_container_runtime.h"
#include "windows_storage.h"
#include "../json_helpers.h"

#include <Windows.h>
#include <computecore.h>

#include <stdexcept>

using namespace Container;

Runtime::IContainerPtr Runtime::WindowsContainerManager::open(const wchar_t* name)
{
	HRESULT Result;

	PWSTR ResultDocument = NULL;
	HCS_OPERATION Operation = HcsCreateOperation(nullptr, nullptr);

	HCS_SYSTEM ComputeSystem = NULL;
	Result = HcsOpenComputeSystem(name, GENERIC_ALL, &ComputeSystem);
	if (Result == S_OK) {
		return IContainerPtr(new WindowsContainer(ComputeSystem));
	}

	return nullptr;
}

Runtime::IContainerPtr Runtime::WindowsContainerManager::create(const wchar_t* name, const ContainerOptions& options)
{
	HRESULT Result;

	PWSTR ResultDocument = NULL;
	HCS_OPERATION Operation = HcsCreateOperation(nullptr, nullptr);

	nlohmann::json guestOsOptionsJson =
	{
		{ "HostName", options.guestOptions.hostName.c_str() }
	};

	nlohmann::json storageOptionsJson =
	{
		{ "Layers", parseLayerOptions(options.storageOptions.layers) },
		{ "Path", options.storageOptions.mountPath.c_str() }
	};

	nlohmann::json memoryOptionsJson =
	{
		{ "SizeInMB", options.memoryOptions.size }
	};

	nlohmann::json processorOptionsJson =
	{
		{ "Count", options.processorOptions.count }
	};

	nlohmann::json containerOptionsJson =
	{
		{ "GuestOs", guestOsOptionsJson },
		{ "Storage", storageOptionsJson },
		{ "Memory", memoryOptionsJson },
		{ "Processor", processorOptionsJson },
	};

	nlohmann::json hcsOptionsJson =
	{
		{ "SchemaVersion", {
				{ "Major", 2 },
				{ "Minor", 1 }
			}
		},
		{ "Owner", name },
		{ "ShouldTerminateOnLastHandleClosed", true },
		{ "Container", containerOptionsJson }
	};

	const std::wstring hcsOptionsJsonStr = JsonHelpers::getUnicodeJsonData(hcsOptionsJson);

	HCS_SYSTEM ComputeSystem = NULL;
	Result = HcsCreateComputeSystem(
		L"Sample",
		hcsOptionsJsonStr.c_str(),
		Operation,
		NULL,
		&ComputeSystem
	);
	Result = HcsWaitForOperationResult(Operation, INFINITE, &ResultDocument);

	return IContainerPtr(new WindowsContainer(ComputeSystem));
}

Runtime::WindowsContainer::WindowsContainer(void* inComputeSystem)
	: computeSystem(inComputeSystem)
{
}

Runtime::WindowsContainer::~WindowsContainer()
{
	HcsCloseComputeSystem((HCS_SYSTEM)computeSystem);
}

void Runtime::WindowsContainer::start()
{
	HRESULT Result;
	PWSTR ResultDocument;

	HCS_OPERATION Operation = HcsCreateOperation(nullptr, nullptr);

	Result = HcsStartComputeSystem((HCS_SYSTEM)computeSystem, Operation, NULL);
	Result = HcsWaitForOperationResult(Operation, INFINITE, &ResultDocument);

	HcsCloseOperation(Operation);
}

void Runtime::WindowsContainer::terminate()
{
	HRESULT Result;
	PWSTR ResultDocument;

	HCS_OPERATION Operation = HcsCreateOperation(nullptr, nullptr);

	Result = HcsTerminateComputeSystem((HCS_SYSTEM)computeSystem, Operation, NULL);
	Result = HcsWaitForOperationResult(Operation, INFINITE, &ResultDocument);

	HcsCloseOperation(Operation);
}

void* Runtime::WindowsContainer::getComputeSystem() const
{
	return computeSystem;
}
