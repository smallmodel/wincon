#include "hard_link_iterator.h"

#include <Windows.h>

hard_link_iterator::hard_link_iterator(const std::filesystem::path& p)
{
	DWORD StringLength = 0;
	hStream = FindFirstFileNameW(p.native().c_str(), 0, &StringLength, NULL);

	if (hStream == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_MORE_DATA)
		{
			PWSTR String = new WCHAR[StringLength];
			hStream = FindFirstFileNameW(p.native().c_str(), 0, &StringLength, String);
			hard_link_path = String;

			delete[] String;
		}
		else
		{
			hStream = NULL;
		}
	}
}

hard_link_iterator::~hard_link_iterator()
{
	if (hStream)
	{
		FindClose(hStream);
		hStream = NULL;
	}
}

hard_link_iterator& hard_link_iterator::operator++()
{
	increment();
	return *this;
}

hard_link_iterator& hard_link_iterator::operator++(int)
{
	increment();
	return *this;
}

void hard_link_iterator::increment()
{
	DWORD StringLength = 0;

	if (!FindNextFileNameW((HANDLE)hStream, &StringLength, NULL) && GetLastError() == ERROR_MORE_DATA)
	{
		PWSTR String = new WCHAR[StringLength];
		FindNextFileNameW(hStream, &StringLength, String);

		hard_link_path = String;

		delete[] String;
	}
	else
	{
		FindClose(hStream);
		hStream = NULL;
	}
}

hard_link_iterator::operator bool() const
{
	return hStream != NULL;
}

const std::filesystem::path& hard_link_iterator::operator*() const
{
	return hard_link_path;
}

const std::filesystem::path* hard_link_iterator::operator->() const
{
	return &hard_link_path;
}
