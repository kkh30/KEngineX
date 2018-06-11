file(GLOB DOT_H *.h)
file(GLOB DOT_CPP *.cpp)


add_library(Framework STATIC ${DOT_H} ${DOT_CPP})