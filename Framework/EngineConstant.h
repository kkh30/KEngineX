#pragma once
#include <memory>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
extern HINSTANCE APP_INSTANCE;
extern int CMD_SHOW;
#endif // _WIN32




//Consts
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int SWAP_CHAIN_COUNT = 3;

//Alias
template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;