#include "windows_storage.h"
#include "../json_helpers.h"

#include <Windows.h>
#include <Objbase.h>

#include <computestorage.h>
#include <virtdisk.h>

#include <filesystem>
#include <nlohmann/json.hpp>

using namespace Container;

Storage::WindowsDisk::WindowsDisk(void* inVhdHandle)
	: vhdHandle(inVhdHandle)
{

}

Storage::WindowsDisk::~WindowsDisk()
{

}

Storage::IStoragePtr Storage::WindowsStorageManager::create(const StorageOptions& options)
{
	HRESULT Result = S_OK;
	DWORD Win32Result = 0;

	VIRTUAL_STORAGE_TYPE StorageType;
	StorageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;

	GUID RandomGuid;
	UuidCreate(static_cast<UUID*>(&RandomGuid));

	CREATE_VIRTUAL_DISK_PARAMETERS Parameters{ 0 };
	Parameters.Version = CREATE_VIRTUAL_DISK_VERSION_2;
	Parameters.Version2.UniqueId = RandomGuid;
	Parameters.Version2.MaximumSize = options.size;
	Parameters.Version2.BlockSizeInBytes = CREATE_VIRTUAL_DISK_PARAMETERS_DEFAULT_BLOCK_SIZE;
	Parameters.Version2.SectorSizeInBytes = CREATE_VIRTUAL_DISK_PARAMETERS_DEFAULT_SECTOR_SIZE;
	Parameters.Version2.ParentPath = NULL;
	Parameters.Version2.SourcePath = NULL;
	Parameters.Version2.OpenFlags = OPEN_VIRTUAL_DISK_FLAG_NONE;

	std::filesystem::remove(options.path.native().c_str());

	// Step 1: Create the virtual hard disk and format it
	HANDLE VhdHandle;
	Win32Result = CreateVirtualDisk(
		&StorageType,
		options.path.c_str(),
		VIRTUAL_DISK_ACCESS_NONE,
		NULL,
		CREATE_VIRTUAL_DISK_FLAG_NONE,
		0,
		&Parameters,
		NULL,
		&VhdHandle
	);

	HcsFormatWritableLayerVhd(VhdHandle);

	return IStoragePtr(new WindowsDisk(VhdHandle));
}

void Storage::WindowsDisk::attach(const AttachOptions& options)
{
	HRESULT Result;
	DWORD Win32Result;

	// Step 2: Mount the partition
	Win32Result = AttachVirtualDisk(
		vhdHandle,
		NULL,
		ATTACH_VIRTUAL_DISK_FLAG_NONE,
		0,
		NULL,
		NULL
	);

	PWSTR MountPath;
	Result = HcsGetLayerVhdMountPath(vhdHandle, &MountPath);

	nlohmann::json layerJson =
	{
		{ "SchemaVersion", {
				{ "Major", 2 },
				{ "Minor", 1 }
			}
		}
	};

	layerJson.emplace("Layers", parseLayerOptions(options.layers));

	// Step 3: Initialize the volume (overlay, sandbox state, hives...)
	// In accordance to the base layer
	const std::wstring layerDataStr = JsonHelpers::getUnicodeJsonData(layerJson);
	Result = HcsInitializeWritableLayer(MountPath, layerDataStr.c_str(), NULL);
	// Step 4: Mount the volume with the base layer (unified view)
	Result = HcsAttachLayerStorageFilter(MountPath, layerDataStr.c_str());

	LocalFree(MountPath);
}

void Storage::WindowsDisk::detach()
{
	HRESULT Result;

	PWSTR MountPath;
	Result = HcsGetLayerVhdMountPath(vhdHandle, &MountPath);

	Result = HcsDestroyLayer(MountPath);
	Result = HcsDetachLayerStorageFilter(MountPath);

	LocalFree(MountPath);
}

std::filesystem::path Storage::WindowsDisk::getAttachedStorageMountPath() const
{
	HRESULT Result;

	PWSTR MountPath;
	Result = HcsGetLayerVhdMountPath(vhdHandle, &MountPath);

	const std::filesystem::path mountPath = MountPath;
	LocalFree(MountPath);

	return mountPath;
}

std::vector<nlohmann::json> Storage::parseLayerOptions(const std::span<const LayerOptions>& options)
{
	std::vector<nlohmann::json> layersArray;
	layersArray.reserve(options.size());

	for (const LayerOptions& layerOptions : options)
	{
		LPOLESTR lpsz;
		StringFromCLSID(*(const IID*)&layerOptions.layerGuid, &lpsz);

		std::wstring idView(lpsz + 1, std::wcslen(lpsz) - 2);

		nlohmann::json layer = {
			{ "Id", idView.c_str() },
			{ "Path", layerOptions.path.wstring().c_str()},
		};

		CoTaskMemFree(lpsz);

		switch (layerOptions.pathType)
		{
		case pathType_e::AbsolutePath:
			layer.emplace("PathType", "AbsolutePath");
			break;
		case pathType_e::VirtualSmbPipeName:
			layer.emplace("PathType", "VirtualSmbPipeName");
			break;
		}

		layersArray.push_back(std::move(layer));
	}

	return layersArray;
}
