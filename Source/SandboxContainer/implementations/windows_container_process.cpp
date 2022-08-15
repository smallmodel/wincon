#include "windows_container_process.h"
#include "windows_container_process_internals.h"
#include "windows_container_runtime.h"
#include "../json_helpers.h"

#include <stdexcept>

#include <Windows.h>
#include <computecore.h>

#include <nlohmann/json.hpp>

using namespace Container;

Runtime::WindowsProcess::WindowsProcess(const HCS_PROCESS_INFORMATION& processInfo)
	: inputStreamBuf(processInfo.StdInput)
	, inputStream(&inputStreamBuf)
	, outputStreamBuf(processInfo.StdOutput)
	, outputStream(&outputStreamBuf)
	, errorStreamBuf(processInfo.StdError)
	, errorStream(&errorStreamBuf)
{
}

Runtime::WindowsProcess::~WindowsProcess()
{

}

std::ostream* Runtime::WindowsProcess::getInputStream()
{
	return &inputStream;
}

std::istream* Runtime::WindowsProcess::getOutputStream()
{
	return &outputStream;
}

std::istream* Runtime::WindowsProcess::getErrorStream()
{
	return &errorStream;
}

Runtime::WindowsStreamBuf::WindowsStreamBuf(HANDLE hStdHandle)
	: hStdHandle(hStdHandle)
{
	data_w = 0;
	data_r = 0;
}

Runtime::WindowsStreamBuf::~WindowsStreamBuf()
{

}

/*
int Runtime::WindowsStreamBuf::overflow(int ch)
{
	data_w = static_cast<char>(ch);

	DWORD NumBytesWritten = 0;
	const BOOL bSuccess = WriteFile(this->hStdHandle, &this->data_w, sizeof(this->data_w), &NumBytesWritten, NULL);
	if (!bSuccess)
	{
		// failure
		return std::char_traits<char>::eof();
	}

	this->setp(&this->data_w, &this->data_w, &this->data_w + sizeof(this->data_w));

	return std::char_traits<char>::to_int_type(*this->pptr());
}

int Runtime::WindowsStreamBuf::underflow()
{
	this->setg(&this->data_r, &this->data_r, &this->data_r + sizeof(this->data_r));

	DWORD NumBytesRead = 0;
	const BOOL bSuccess = ReadFile(this->hStdHandle, &this->data_r, sizeof(this->data_r), &NumBytesRead, NULL);
	if (!bSuccess)
	{
		// failure
		return std::char_traits<char>::eof();
	}

	return std::char_traits<char>::to_int_type(*this->gptr());
}
*/

std::streamsize Runtime::WindowsStreamBuf::xsputn(const char_type* s, std::streamsize count)
{
	DWORD NumBytesWritten = 0;
	const BOOL bSuccess = WriteFile(this->hStdHandle, s, static_cast<DWORD>(count), &NumBytesWritten, NULL);
	if (!bSuccess)
	{
		// failure
		return 0;
	}

	FlushFileBuffers(this->hStdHandle);

	return NumBytesWritten;
}

std::streamsize Runtime::WindowsStreamBuf::xsgetn(char_type* s, std::streamsize count)
{
	DWORD NumBytesRead = 0;
	const BOOL bSuccess = ReadFile(this->hStdHandle, s, static_cast<DWORD>(count), &NumBytesRead, NULL);
	if (!bSuccess)
	{
		// failure
		return 0;
	}

	return NumBytesRead;
}

Runtime::IProcessPtr Runtime::WindowsProcessManager::createProcess(const IContainerPtr& container, const ProcessCreateOptions& options)
{
	nlohmann::json optionsJson =
	{
		{ "ApplicationName", options.applicationName.c_str() },
		{ "CommandLine", options.commandLine.c_str() },
		{ "User", options.userName.c_str() },
		{ "WorkingDirectory", options.workingDir.c_str() },
		{ "CreateStdInPipe", options.bCreateStdInPipe },
		{ "CreateStdOutPipe", options.bCreateStdOutPipe },
		{ "CreateStdErrPipe", options.bCreateStdErrPipe },
	};

	std::wstring optionsJsonData = JsonHelpers::getUnicodeJsonData(optionsJson);

	HRESULT Result;
	HCS_PROCESS process;

	WindowsContainer* winCon = static_cast<WindowsContainer*>(container.get());

	HCS_OPERATION Operation = HcsCreateOperation(nullptr, nullptr);

	Result = HcsCreateProcess(
		(HCS_SYSTEM)winCon->getComputeSystem(),
		optionsJsonData.c_str(),
		Operation,
		NULL,
		&process
	);

	PWSTR ResultDocument;
	HCS_PROCESS_INFORMATION processInfo;
	Result = HcsWaitForOperationResultAndProcessInfo(Operation, INFINITE, &processInfo, &ResultDocument);

	HcsCloseOperation(Operation);

	return IProcessPtr(new WindowsProcess(processInfo));
}

