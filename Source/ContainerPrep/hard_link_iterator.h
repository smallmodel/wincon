#pragma once

#include <filesystem>

class hard_link_iterator {
public:
	hard_link_iterator(const std::filesystem::path& p);
	~hard_link_iterator();

	hard_link_iterator& operator++();
	hard_link_iterator& operator++(int);
	void increment();

	operator bool() const;
	const std::filesystem::path& operator*() const;
	const std::filesystem::path* operator->() const;

private:
	std::filesystem::path hard_link_path;
	void* hStream;
};
