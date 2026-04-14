#include "graph_buffer.h"

GraphBuffer  g_graph_temp     = {{}, 0, 0};
GraphBuffer  g_graph_humidity = {{}, 0, 0};
GraphBuffer  g_graph_ds18     = {{}, 0, 0};
GraphBuffer  g_graph_light    = {{}, 0, 0};
GraphBuffer  g_graph_sound    = {{}, 0, 0};
GraphBuffer  g_graph_soil     = {{}, 0, 0};
portMUX_TYPE g_graph_mux      = portMUX_INITIALIZER_UNLOCKED;

void graph_buffer_push(GraphBuffer& buf, float value) {
    buf.data[buf.head] = value;
    buf.head = (buf.head + 1) % GRAPH_BUFFER_SIZE;
    if (buf.count < GRAPH_BUFFER_SIZE) ++buf.count;
}

size_t graph_buffer_get(const GraphBuffer& buf, float* out, size_t out_size) {
    const size_t n = (buf.count < out_size) ? buf.count : out_size;
    if (n == 0) return 0;
    // Oldest sample sits at (head - count) wrapped around.
    const size_t start = (buf.head + GRAPH_BUFFER_SIZE - buf.count) % GRAPH_BUFFER_SIZE;
    for (size_t i = 0; i < n; ++i) {
        out[i] = buf.data[(start + i) % GRAPH_BUFFER_SIZE];
    }
    return n;
}
