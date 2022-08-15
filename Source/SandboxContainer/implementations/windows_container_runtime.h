#pragma once

#include "../interfaces/container_runtime.h"

namespace Container
{
	namespace Runtime
	{
		class WindowsContainer : public IContainer
		{
		public:
			WindowsContainer(void* inComputeSystem);
			~WindowsContainer();

			void start() override;
			void terminate() override;
			void* getComputeSystem() const;

		private:
			void* computeSystem;
		};

		class WindowsContainerManager : public IContainerManager
		{

		public:
			IContainerPtr open(const wchar_t* name) override;
			IContainerPtr create(const wchar_t* name, const ContainerOptions& options) override;
		};
	}
}
