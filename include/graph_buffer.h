#pragma once
#include <stddef.h>
#include <Arduino.h>  // portMUX_TYPE

// Number of samples kept per sensor (1 sample/s → ~2 min 40 s of history).
// Also equals the usable graph width in pixels.
constexpr size_t GRAPH_BUFFER_SIZE = 160;

struct GraphBuffer {
    float  data[GRAPH_BUFFER_SIZE];
    size_t head;   // index of next write slot
    size_t count;  // valid samples in buffer (0 .. GRAPH_BUFFER_SIZE)
};

// Push one sample.  Call only from the sensor task, under g_graph_mux.
void graph_buffer_push(GraphBuffer& buf, float value);

// Copy up to out_size samples into out[], oldest-first (chronological order).
// Returns the number of samples actually written.
size_t graph_buffer_get(const GraphBuffer& buf, float* out, size_t out_size);

// Shared instances (defined in graph_buffer.cpp, filled by io.cpp).
extern GraphBuffer  g_graph_temp;
extern GraphBuffer  g_graph_humidity;
extern GraphBuffer  g_graph_ds18;
extern GraphBuffer  g_graph_light;
extern GraphBuffer  g_graph_sound;
extern GraphBuffer  g_graph_soil;
extern portMUX_TYPE g_graph_mux;
