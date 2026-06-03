#include "sensor_connection_notice.h"

#include <Arduino.h>

#include "runtime_events.h"

namespace {

constexpr uint32_t kNoticeDurationMs = 1500UL;
constexpr uint8_t kTrackedCount = 2;
constexpr uint8_t kQueueSize = 4;

struct Notice {
    SzSensorId sensor;
    SensorConnectionNoticeKind kind;
};

portMUX_TYPE g_notice_mux = portMUX_INITIALIZER_UNLOCKED;
bool g_seen_sample[kTrackedCount] = { false, false };
bool g_was_connected[kTrackedCount] = { false, false };
Notice g_queue[kQueueSize] = {};
uint8_t g_queue_head = 0;
uint8_t g_queue_count = 0;
bool g_active = false;
SzSensorId g_active_sensor = SZ_SOIL;
SensorConnectionNoticeKind g_active_kind = SENSOR_NOTICE_CONNECTED;
uint32_t g_active_until_ms = 0;

int tracked_index(SzSensorId sensor) {
    switch (sensor) {
        case SZ_SOIL: return 0;
        case SZ_DS18: return 1;
        default:      return -1;
    }
}

bool sensor_pending_locked(SzSensorId sensor, SensorConnectionNoticeKind kind) {
    if (g_active && g_active_sensor == sensor && g_active_kind == kind) return true;
    for (uint8_t i = 0; i < g_queue_count; ++i) {
        const uint8_t index = (uint8_t)((g_queue_head + i) % kQueueSize);
        if (g_queue[index].sensor == sensor && g_queue[index].kind == kind) return true;
    }
    return false;
}

bool enqueue_locked(SzSensorId sensor, SensorConnectionNoticeKind kind) {
    if (g_queue_count >= kQueueSize || sensor_pending_locked(sensor, kind)) return false;
    const uint8_t tail = (uint8_t)((g_queue_head + g_queue_count) % kQueueSize);
    g_queue[tail] = { sensor, kind };
    g_queue_count++;
    return true;
}

bool dequeue_locked(Notice* notice) {
    if (g_queue_count == 0 || !notice) return false;
    *notice = g_queue[g_queue_head];
    g_queue_head = (uint8_t)((g_queue_head + 1) % kQueueSize);
    g_queue_count--;
    return true;
}

} // namespace

void sensor_connection_notice_note_sample(SzSensorId sensor, bool connected) {
    const int index = tracked_index(sensor);
    if (index < 0) return;

    bool queued = false;
    portENTER_CRITICAL(&g_notice_mux);
    if (!g_seen_sample[index]) {
        g_seen_sample[index] = true;
        g_was_connected[index] = connected;
    } else if (connected != g_was_connected[index]) {
        const SensorConnectionNoticeKind kind = connected
            ? SENSOR_NOTICE_CONNECTED
            : SENSOR_NOTICE_DISCONNECTED;
        queued = enqueue_locked(sensor, kind);
        g_was_connected[index] = connected;
    }
    portEXIT_CRITICAL(&g_notice_mux);

    if (queued) {
        runtime_request_ui_full_redraw();
    }
}

void sensor_connection_notice_service(bool blocked) {
    bool redraw = false;
    const uint32_t now = millis();

    portENTER_CRITICAL(&g_notice_mux);
    if (blocked) {
        if (g_active) {
            g_active_until_ms = now + kNoticeDurationMs;
        }
        portEXIT_CRITICAL(&g_notice_mux);
        return;
    }

    if (g_active && (int32_t)(now - g_active_until_ms) >= 0) {
        g_active = false;
        redraw = true;
    }

    if (!g_active) {
        Notice next = { SZ_SOIL, SENSOR_NOTICE_CONNECTED };
        if (dequeue_locked(&next)) {
            g_active = true;
            g_active_sensor = next.sensor;
            g_active_kind = next.kind;
            g_active_until_ms = now + kNoticeDurationMs;
            redraw = true;
        }
    }
    portEXIT_CRITICAL(&g_notice_mux);

    if (redraw) {
        runtime_request_ui_full_redraw();
    }
}

bool sensor_connection_notice_current(SzSensorId* out_sensor, SensorConnectionNoticeKind* out_kind) {
    portENTER_CRITICAL(&g_notice_mux);
    const bool active = g_active;
    if (active && out_sensor) {
        *out_sensor = g_active_sensor;
    }
    if (active && out_kind) {
        *out_kind = g_active_kind;
    }
    portEXIT_CRITICAL(&g_notice_mux);
    return active;
}
