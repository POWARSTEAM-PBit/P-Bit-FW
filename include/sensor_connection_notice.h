#pragma once

#include <stdint.h>

#include "sensor_zone.h"

enum SensorConnectionNoticeKind : uint8_t {
    SENSOR_NOTICE_CONNECTED,
    SENSOR_NOTICE_DISCONNECTED
};

void sensor_connection_notice_note_sample(SzSensorId sensor, bool connected);
void sensor_connection_notice_service(bool blocked);
bool sensor_connection_notice_current(SzSensorId* out_sensor, SensorConnectionNoticeKind* out_kind);
