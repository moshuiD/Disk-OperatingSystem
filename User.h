#pragma once
#include <Windows.h>
struct User {
	static constexpr DWORD ROOT = ~0;
	static DWORD currentUser;
};