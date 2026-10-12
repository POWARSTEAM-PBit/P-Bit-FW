#pragma once

#include <stdint.h>
#include "plant_pet.h"

// Native pixels in the existing Planta Lab terrarium, placed at TFT (3, 28).
constexpr int PLANT_PET_ART_W = 54;
constexpr int PLANT_PET_ART_H = 95;
constexpr uint16_t PLANT_PET_GREEN = 0x374B; // rgb(48, 232, 88), shared with the badge.

// Software-only RGB565 rasterizer: caller owns/reuses the complete buffer.
// No TFT calls, allocation, text, or change to the model/sensor state.
// Optional readings drive continuous visual detail; bands still gate needs.
// Null uses representative band values for standalone pose previews.
void plant_pet_render(uint16_t* pixels, const PlantPetSnapshot& pet, uint32_t now_ms,
                      const PlantPetInputs* readings = nullptr,
                      const PlantPetProfile* profile = nullptr);
