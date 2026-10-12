#pragma once
#include <cstddef>
#include <cstdint>
template <class T> T constrain(T value, T low, T high) { return value < low ? low : value > high ? high : value; }
using portMUX_TYPE = int;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(mux) ((void)(mux))
#define portEXIT_CRITICAL(mux) ((void)(mux))
