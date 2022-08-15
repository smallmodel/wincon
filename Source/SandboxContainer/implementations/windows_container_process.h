#pragma once

#include "../interfaces/container_process.h"

namespace Container
{
	namespace Runtime
	{
		class WindowsProcessManager : public IProcessManager
		{
		public:
			IProcessPtr createProcess(const IContainerPtr& container, const ProcessCreateOptions& options) override;
		};
	}
}
