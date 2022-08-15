#pragma once

#include "storage.h"

#include <span>
#include <memory>

namespace Container
{
	namespace Runtime
	{
		struct GuestOsOptions
		{
			std::wstring hostName;
		};

		struct StorageOptions
		{
			std::span<const Storage::LayerOptions> layers;
			std::wstring mountPath;
		};

		struct MemoryOptions
		{
			/** Memory size in MB. */
			uint32_t size;
		};

		struct ProcessorOptions
		{
			/** Number of CPUs. */
			uint32_t count;
		};

		struct ContainerOptions
		{
			GuestOsOptions guestOptions;
			StorageOptions storageOptions;
			MemoryOptions memoryOptions;
			ProcessorOptions processorOptions;
		};

		class IContainer
		{
		public:
			virtual ~IContainer() = default;

			virtual void start() = 0;
			virtual void terminate() = 0;
		};
		using IContainerPtr = std::shared_ptr<IContainer>;

		class IContainerManager
		{
		public:
			virtual ~IContainerManager() = default;

			virtual IContainerPtr open(const wchar_t* name) = 0;
			virtual IContainerPtr create(const wchar_t* name, const ContainerOptions& options) = 0;
		};
	}
}
