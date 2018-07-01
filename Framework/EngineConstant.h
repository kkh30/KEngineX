#pragma once
#include <memory>



//Consts
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

//Alias
template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;