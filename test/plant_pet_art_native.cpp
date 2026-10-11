#include "plant_pet_art.h"
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <math.h>

static PlantPetSnapshot pose(PlantMood mood) {
    PlantPetSnapshot p = {};
    p.mood = p.previous_mood = mood; p.culprit = PlantChannel::None; p.transition = 255;
    p.thermal_intensity = p.previous_thermal_intensity = 255;
    for(int i=0;i<4;++i) p.bands[i]=PlantBand::Normal;
    int c=-1; PlantBand b=PlantBand::Low;
    switch(mood) {
      case PlantMood::Thirsty: c=0; break;
      case PlantMood::Soggy: c=0;b=PlantBand::High;break;
      case PlantMood::Cold: c=1;break;
      case PlantMood::Hot: c=1;b=PlantBand::High;break;
      case PlantMood::AirDry: c=2;break;
      case PlantMood::AirHumid: c=2;b=PlantBand::High;break;
      case PlantMood::Dim: c=3;break;
      case PlantMood::Unknown: p.unknown_mask=15;for(int i=0;i<4;++i)p.bands[i]=PlantBand::Unknown;break;
      default:break;
    }
    if(c>=0){p.culprit=static_cast<PlantChannel>(c);p.bands[c]=b;p.affected_mask=1<<c;}
    return p;
}
int main() {
    const int size=PLANT_PET_ART_W*PLANT_PET_ART_H;
    uint16_t guarded[size+32];
    for(int m=0;m<9;++m) for(int t=0;t<45;++t) for(int blend=0;blend<3;++blend){
        std::fill(guarded,guarded+size+32,0xA55A);
        PlantPetSnapshot p=pose(static_cast<PlantMood>(m));p.previous_mood=PlantMood::Content;p.transition=blend==0?0:blend==1?128:255;
        plant_pet_render(guarded+16,p,t*220);
        for(int i=0;i<16;++i)if(guarded[i]!=0xA55A||guarded[size+16+i]!=0xA55A)return 2;
    }
    puts("Plant raster: 1215 guarded pose/phase/transition checks passed");
    // A raw reading can change inside one band: soil color must darken without
    // needing a new mood. Compare an unobstructed patch below roots and water.
    PlantPetSnapshot p = pose(PlantMood::Content);
    PlantPetInputs readings = {{0,22,58,1200},{true,true,true,true}};
    uint16_t previous = 0xFFFF;
    for(int level=0;level<=100;++level) {
        readings.values[0]=static_cast<float>(level);
        p.bands[0]=level<30?PlantBand::Low:level>70?PlantBand::High:PlantBand::Normal;
        std::fill(guarded,guarded+size+32,0xA55A);
        plant_pet_render(guarded+16,p,2200,&readings);
        for(int i=0;i<16;++i)if(guarded[i]!=0xA55A||guarded[size+16+i]!=0xA55A)return 3;
        const uint16_t soil=guarded[16+92*PLANT_PET_ART_W+2];
        if(((soil>>11)&31)>((previous>>11)&31)||((soil>>5)&63)>((previous>>5)&63)
           ||(soil&31)>(previous&31))return 4;
        previous=soil;
    }
    // Missing/nonfinite evidence must not reuse a dry or wet picture.
    const float invalid[]={NAN,INFINITY,-INFINITY};
    uint16_t neutral=0;
    readings.valid[0]=false;
    plant_pet_render(guarded+16,p,2200,&readings);
    neutral=guarded[16+92*PLANT_PET_ART_W+2];
    readings.valid[0]=true;
    for(float value:invalid) {
        readings.values[0]=value;
        plant_pet_render(guarded+16,p,2200,&readings);
        if(guarded[16+92*PLANT_PET_ART_W+2]!=neutral)return 5;
    }
    puts("Plant soil: 101 progressive levels and 4 missing/nonfinite cases passed");
    return 0;
}
