#pragma once
#include <memory>



//Consts
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

//Alias
template<typename T>
using UniquePtr = std::unique_ptr<T>;