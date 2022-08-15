#pragma once

#include <string>
#include <cstdint>
#include <span>
#include <memory>
#include <filesystem>

namespace Container
{
	namespace Storage
	{
		struct StorageId
		{
			std::uint32_t data1;
			std::uint16_t data2;
			std::uint16_t data3;
			std::uint8_t data4[8];
		};

		struct StorageOptions
		{
			StorageId guid;

			/** Path to the storage. */
			std::filesystem::path path;

			/** The size of the storage, in bytes. */
			uintmax_t size;
		};

		enum class pathType_e : unsigned char
		{
			AbsolutePath,
			VirtualSmbPipeName
		};

		struct LayerOptions
		{
			/** ID of the layer. */
			StorageId layerGuid;

			/** Path to the layer. */
			std::filesystem::path path;

			/** Type of the path. */
			pathType_e pathType;
		};

		struct AttachOptions
		{
			std::span<const LayerOptions> layers;
		};

		class IStorage
		{
		public:
			virtual ~IStorage() = default;

			virtual void attach(const AttachOptions& options) = 0;
			virtual void detach() = 0;
			virtual std::filesystem::path getAttachedStorageMountPath() const = 0;
		};
		using IStoragePtr = std::shared_ptr<IStorage>;

		class IStorageManager
		{
		public:
			virtual ~IStorageManager() = default;

			virtual IStoragePtr create(const StorageOptions& options) = 0;
		};
	}
}
