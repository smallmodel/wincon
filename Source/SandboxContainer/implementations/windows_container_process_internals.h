#pragma once

#include "../interfaces/container_process.h"

#include <streambuf>

#include <Windows.h>
#include <computecore.h>

namespace Container
{
	namespace Runtime
	{
		class WindowsStreamBuf : public std::streambuf
		{
		public:
			WindowsStreamBuf(HANDLE hStdHandle);
			~WindowsStreamBuf();

		protected:
			std::streamsize xsputn(const char_type* s, std::streamsize count) override;
			std::streamsize xsgetn(char_type* s, std::streamsize count) override;

		private:
			HANDLE hStdHandle;
			char data_r;
			char data_w;
		};

		class WindowsProcess : public IProcess
		{
		public:
			WindowsProcess(const HCS_PROCESS_INFORMATION& processInfo);
			~WindowsProcess();

			std::ostream* getInputStream() override;
			std::istream* getOutputStream() override;
			std::istream* getErrorStream() override;

		private:
			WindowsStreamBuf inputStreamBuf;
			std::ostream inputStream;
			WindowsStreamBuf outputStreamBuf;
			std::istream outputStream;
			WindowsStreamBuf errorStreamBuf;
			std::istream errorStream;
		};

	}
}
