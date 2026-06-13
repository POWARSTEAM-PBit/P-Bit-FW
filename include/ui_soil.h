#pragma once
#include <stdint.h>

// Internal states for the Soil calibration flow.
enum SoilCalibrationState : uint8_t {
    SOIL_CAL_IDLE = 0,
    SOIL_CAL_MENU,
    SOIL_CAL_NO_SENSOR,
    SOIL_CAL_WAIT_DRY,
    SOIL_CAL_WAIT_WET,
    SOIL_CAL_REVIEW_SAVE,
    SOIL_CAL_THRESH_DRY,
    SOIL_CAL_THRESH_MOIST,
    SOIL_CAL_EDIT_MARKS,
    SOIL_CAL_EDIT_ALERTS,
    SOIL_CAL_RESET_CONFIRM,
    SOIL_CAL_DONE,
    SOIL_CAL_RESET_DONE,
    SOIL_CAL_THRESH_DONE,
    SOIL_CAL_MARKS_DONE,
    SOIL_CAL_ALERTS_DONE,
    SOIL_CAL_ERROR,
};

// Public API
bool soilCalibrationIsActive();
SoilCalibrationState getSoilCalibrationState();
void startSoilCalibration();
bool cancelSoilCalibration();
void setSoilCalibrationInputValue(int value);
int getSoilCalibrationEncoderMin();
int getSoilCalibrationEncoderMax();
int getSoilCalibrationEncoderValue();
uint8_t handleSoilCalibrationButton();

void draw_soil_screen(bool screen_changed, bool data_changed);
