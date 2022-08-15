#pragma once

#include "../interfaces/storage.h"

#include <nlohmann/json.hpp>

namespace Container
{
	namespace Storage
	{
		class WindowsDisk : public IStorage
		{
		public:
			WindowsDisk(void* inVhdHandle);
			~WindowsDisk();

			void attach(const AttachOptions& options) override;
			void detach() override;
			std::filesystem::path getAttachedStorageMountPath() const override;

		private:
			void* vhdHandle;
		};

		class WindowsStorageManager : public IStorageManager
		{

		public:
			IStoragePtr create(const StorageOptions& options) override;
		};

		std::vector<nlohmann::json> parseLayerOptions(const std::span<const LayerOptions>& options);
	}
}
