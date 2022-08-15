#pragma once

class Privilege
{
public:
	Privilege(const wchar_t* name);
	~Privilege();

private:
	const wchar_t* privilegeName;
	unsigned long previousState;
};
