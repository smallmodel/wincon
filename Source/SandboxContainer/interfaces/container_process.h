#pragma once

#include "container_runtime.h"

#include <string>
#include <memory>

namespace Container
{
	namespace Runtime
	{
		class IContainer;

		struct ProcessCreateOptions
		{
			/** Path to the executable, locally in the container. */
			std::wstring applicationName;

			/** Command line. */
			std::wstring commandLine;

			/** The user of the application. */
			std::wstring userName;

			/** The working directory of the application. */
			std::wstring workingDir;

			/** Whether or not to create in, out and/or err pipes. */
			bool bCreateStdInPipe;
			bool bCreateStdOutPipe;
			bool bCreateStdErrPipe;
		};

		class IProcess
		{
		public:
			virtual ~IProcess() = default;

			virtual std::ostream* getInputStream() = 0;
			virtual std::istream* getOutputStream() = 0;
			virtual std::istream* getErrorStream() = 0;
		};
		using IProcessPtr = std::shared_ptr<IProcess>;

		class IProcessManager
		{
		public:
			virtual ~IProcessManager() = default;

			virtual IProcessPtr createProcess(const IContainerPtr& container, const ProcessCreateOptions& options) = 0;
		};
	}
}
