#include "plant_pet_art.h"
#include <math.h>

namespace {

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return static_cast<uint16_t>(((r & 248u) << 8) | ((g & 252u) << 3) | (b >> 3));
}

// Persistent green identity: needs change pose and surroundings, not leaf hue.
// Natural brown soil and yellow sun; hard native pixels.
constexpr uint16_t kBackground = 0x0841;
constexpr uint16_t kOutline = rgb(8, 56, 40);
constexpr uint16_t kShadow = rgb(0, 120, 72);
constexpr uint16_t kGreen = PLANT_PET_GREEN;
constexpr uint16_t kLight = rgb(112, 255, 160);
constexpr uint16_t kGlint = rgb(200, 255, 216);
constexpr uint16_t kFace = rgb(8, 35, 26);
constexpr uint16_t kEyeGlint = kGlint;
constexpr uint16_t kSoil = rgb(104, 68, 40);
constexpr uint16_t kSoilEdge = rgb(135, 92, 47);
constexpr uint16_t kDrySoil = rgb(157, 105, 55);
constexpr uint16_t kWetSoil = rgb(48, 32, 24);
constexpr uint16_t kSoilDark = rgb(32, 22, 16);
constexpr uint16_t kUnknownSoil = rgb(88, 66, 48);
constexpr uint16_t kWater = rgb(40, 168, 224);
constexpr uint16_t kWaterGlint = rgb(160, 240, 255);
constexpr uint16_t kAir = rgb(0, 232, 255);
constexpr uint16_t kDryAir = rgb(176, 120, 255);
constexpr uint16_t kHeat = rgb(255, 0, 184);
constexpr uint16_t kCold = rgb(112, 216, 255);
constexpr uint16_t kSun = rgb(255, 224, 48);
constexpr uint16_t kMoon = rgb(208, 224, 255);
constexpr uint16_t kUnknownMarker = rgb(96, 152, 144);

struct PlantPalette {
    uint16_t green;
    uint16_t shadow;
    uint16_t stem_light;
    uint16_t accent;
    uint16_t glint;
};

PlantPalette mood_palette(PlantMood mood) {
    (void)mood;
    return {kGreen, kShadow, kLight, kLight, kGlint};
}

struct Point { int x; int y; };

int lesser(int a, int b) { return a < b ? a : b; }
int greater(int a, int b) { return a > b ? a : b; }
int magnitude(int value) { return value < 0 ? -value : value; }

class Canvas {
public:
    explicit Canvas(uint16_t* pixels) : pixels_(pixels) {}

    void pixel(int x, int y, uint16_t color) {
        if (x >= 0 && x < PLANT_PET_ART_W && y >= 0 && y < PLANT_PET_ART_H)
            pixels_[y * PLANT_PET_ART_W + x] = color;
    }

    void rect(int x, int y, int width, int height, uint16_t color) {
        const int end_x = lesser(PLANT_PET_ART_W, x + width);
        const int end_y = lesser(PLANT_PET_ART_H, y + height);
        for (int py = greater(0, y); py < end_y; ++py)
            for (int px = greater(0, x); px < end_x; ++px)
                pixels_[py * PLANT_PET_ART_W + px] = color;
    }

    void disc(Point center, int radius, uint16_t color) {
        for (int y = -radius; y <= radius; ++y)
            for (int x = -radius; x <= radius; ++x)
                // Include half-pixel edge coverage so a tiny moon reads as a
                // round disc, rather than a cross with single-pixel poles.
                if (x * x + y * y <= radius * radius + radius / 2)
                    pixel(center.x + x, center.y + y, color);
    }

    void line(Point a, Point b, uint16_t color) {
        const int dx = magnitude(b.x - a.x);
        const int dy = -magnitude(b.y - a.y);
        const int sx = a.x < b.x ? 1 : -1;
        const int sy = a.y < b.y ? 1 : -1;
        int error = dx + dy;
        for (;;) {
            pixel(a.x, a.y, color);
            if (a.x == b.x && a.y == b.y) break;
            const int twice = 2 * error;
            if (twice >= dy) { error += dy; a.x += sx; }
            if (twice <= dx) { error += dx; a.y += sy; }
        }
    }

    // All authored polygons have <=14 points. Edge intersections use a fixed
    // stack array; a half-open edge rule prevents duplicate vertex crossings.
    void polygon(const Point* points, int count, uint16_t color) {
        if (count < 3 || count > 14) return;
        int top = points[0].y;
        int bottom = top;
        for (int i = 1; i < count; ++i) {
            top = lesser(top, points[i].y);
            bottom = greater(bottom, points[i].y);
        }
        top = greater(top, 0);
        bottom = lesser(bottom, PLANT_PET_ART_H - 1);
        for (int y = top; y <= bottom; ++y) {
            int crossings[14];
            int found = 0;
            for (int i = 0, j = count - 1; i < count; j = i++) {
                const Point a = points[j];
                const Point b = points[i];
                if ((a.y <= y && b.y > y) || (b.y <= y && a.y > y))
                    crossings[found++] = a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y);
            }
            for (int i = 1; i < found; ++i) {
                const int value = crossings[i];
                int j = i;
                while (j && crossings[j - 1] > value) {
                    crossings[j] = crossings[j - 1];
                    --j;
                }
                crossings[j] = value;
            }
            for (int i = 0; i + 1 < found; i += 2)
                rect(crossings[i], y, crossings[i + 1] - crossings[i] + 1, 1, color);
        }
    }

private:
    uint16_t* pixels_;
};

struct Pose {
    Point head;
    int tilt;
    Point left;
    Point right;
    Point small_left;
    Point small_right;
    int leaf_width;
    int small_width;
    int head_width;
};

// Key silhouettes stay inside x3..50, y7..84; soil occupies y82..94.
// Unequal left/right leaves keep the character organic even while still.
Pose key_pose(PlantMood mood) {
    switch (mood) {
    case PlantMood::Content: return {{27, 27}, -1, {5, 48}, {49, 45}, {10, 69}, {45, 66}, 7, 4, 100};
    case PlantMood::Thirsty: return {{30, 36}, 3, {9, 72}, {44, 74}, {15, 79}, {40, 79}, 5, 3, 94};
    case PlantMood::Soggy: return {{24, 34}, -2, {6, 68}, {47, 65}, {12, 78}, {42, 77}, 8, 4, 104};
    case PlantMood::Cold: return {{26, 31}, 0, {15, 44}, {39, 46}, {17, 68}, {39, 69}, 6, 4, 92};
    case PlantMood::Hot: return {{29, 33}, 2, {10, 67}, {44, 59}, {12, 77}, {42, 74}, 6, 3, 98};
    case PlantMood::AirDry: return {{27, 29}, 1, {7, 59}, {47, 55}, {11, 74}, {44, 71}, 4, 2, 87};
    case PlantMood::AirHumid: return {{26, 30}, -1, {6, 53}, {47, 50}, {9, 73}, {44, 70}, 7, 4, 100};
    case PlantMood::Dim: return {{28, 22}, 1, {8, 56}, {47, 58}, {12, 71}, {42, 72}, 6, 3, 91};
    case PlantMood::Unknown: return {{27, 30}, 0, {6, 54}, {48, 53}, {11, 71}, {43, 70}, 6, 3, 100};
    }
    return {{27, 30}, 0, {6, 54}, {48, 53}, {11, 71}, {43, 70}, 6, 3, 100};
}

Pose animated_pose(PlantMood mood, uint32_t tick) {
    Pose pose = key_pose(mood);
    switch (mood) {
    case PlantMood::Content: {
        static const int8_t sway[] = {0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0};
        const int offset = sway[(tick / 2) & 15u];
        pose.head.x += offset;
        pose.left.y += (tick % 24u >= 15u && tick % 24u <= 17u) ? -1 : 0;
        pose.right.y += (tick % 24u >= 4u && tick % 24u <= 6u) ? -1 : 0;
        break;
    }
    case PlantMood::Thirsty:
        if (tick % 32u == 24u || tick % 32u == 25u) {
            ++pose.head.y; ++pose.left.y; ++pose.right.y;
        }
        break;
    case PlantMood::Soggy:
        if (tick % 28u >= 18u && tick % 28u <= 21u) ++pose.right.y;
        break;
    case PlantMood::Cold: {
        const uint32_t phase = tick % 36u;
        // A brief shiver, then a long hold; no continuous jitter.
        if (phase >= 7u && phase <= 10u) {
            const int offset = (phase & 1u) ? -1 : 1;
            pose.head.x += offset; pose.left.x += offset; pose.right.x += offset;
        }
        break;
    }
    case PlantMood::Hot: {
        static const int8_t fan[] = {0, -1, -2, -2, -1, 0};
        const uint32_t phase = tick % 24u;
        if (phase >= 5u && phase < 11u) pose.right.y += fan[phase - 5u];
        break;
    }
    case PlantMood::AirDry:
        if (tick % 30u == 18u || tick % 30u == 19u) ++pose.left.x;
        break;
    case PlantMood::AirHumid:
        if (tick % 40u >= 26u && tick % 40u <= 31u) { --pose.head.x; ++pose.left.y; }
        break;
    case PlantMood::Dim:
        if (tick % 48u >= 34u && tick % 48u <= 39u) --pose.head.y;
        break;
    case PlantMood::Unknown:
        break;
    }
    return pose;
}

int blend(int before, int after, uint8_t transition) {
    const int numerator = before * (255 - transition) + after * transition;
    return (numerator + (numerator < 0 ? -127 : 127)) / 255;
}

Point blend_point(Point before, Point after, uint8_t transition) {
    return {blend(before.x, after.x, transition), blend(before.y, after.y, transition)};
}

Pose blend_pose(const Pose& before, const Pose& after, uint8_t transition) {
    return {blend_point(before.head, after.head, transition), blend(before.tilt, after.tilt, transition),
            blend_point(before.left, after.left, transition), blend_point(before.right, after.right, transition),
            blend_point(before.small_left, after.small_left, transition),
            blend_point(before.small_right, after.small_right, transition),
            blend(before.leaf_width, after.leaf_width, transition),
            blend(before.small_width, after.small_width, transition),
            blend(before.head_width, after.head_width, transition)};
}

uint16_t blend_color(uint16_t before, uint16_t after, uint8_t transition) {
    // Interpolate RGB565 channels separately; packed-word arithmetic would
    // carry between channels and introduce unrelated hues during recovery.
    const int red = blend((before >> 11) & 31, (after >> 11) & 31, transition);
    const int green = blend((before >> 5) & 63, (after >> 5) & 63, transition);
    const int blue = blend(before & 31, after & 31, transition);
    return static_cast<uint16_t>((red << 11) | (green << 5) | blue);
}

PlantPalette blend_palette(const PlantPalette& before, const PlantPalette& after, uint8_t transition) {
    return {blend_color(before.green, after.green, transition),
            blend_color(before.shadow, after.shadow, transition),
            blend_color(before.stem_light, after.stem_light, transition),
            blend_color(before.accent, after.accent, transition),
            blend_color(before.glint, after.glint, transition)};
}

Point stem_point(const Pose& pose, int t) {
    const int u = 16 - t;
    const Point root = {27, 84};
    const Point lower = {21, 74};
    const Point upper = {pose.head.x - 3, pose.head.y + 24};
    const Point top = {pose.head.x, pose.head.y + 11};
    return {(root.x*u*u*u + 3*lower.x*u*u*t + 3*upper.x*u*t*t + top.x*t*t*t + 2048) / 4096,
            (root.y*u*u*u + 3*lower.y*u*u*t + 3*upper.y*u*t*t + top.y*t*t*t + 2048) / 4096};
}

void stem(Canvas& canvas, const Pose& pose, const PlantPalette& palette) {
    Point previous = stem_point(pose, 0);
    for (int t = 1; t <= 16; ++t) {
        const Point next = stem_point(pose, t);
        for (int offset = -2; offset <= 2; ++offset)
            canvas.line({previous.x + offset, previous.y}, {next.x + offset, next.y}, kOutline);
        for (int offset = -1; offset <= 1; ++offset)
            canvas.line({previous.x + offset, previous.y}, {next.x + offset, next.y}, palette.green);
        canvas.line({previous.x - 1, previous.y}, {next.x - 1, next.y}, palette.stem_light);
        previous = next;
    }
}

Point leaf_point(Point root, Point tip, int along, int across) {
    const int dx = tip.x - root.x;
    const int dy = tip.y - root.y;
    const int length = greater(1, greater(magnitude(dx), magnitude(dy)));
    return {root.x + dx * along / 100 - dy * across / length,
            root.y + dy * along / 100 + dx * across / length};
}

void leaf(Canvas& canvas, Point root, Point tip, int width, const PlantPalette& palette) {
    const Point shape[] = {root, leaf_point(root, tip, 26, width / 2), leaf_point(root, tip, 53, width),
        leaf_point(root, tip, 80, width / 2), tip, leaf_point(root, tip, 80, -width / 2),
        leaf_point(root, tip, 48, -width), leaf_point(root, tip, 20, -width / 2)};
    canvas.polygon(shape, 8, palette.green);
    for (int i = 0; i < 8; ++i) canvas.line(shape[i], shape[(i + 1) % 8], kOutline);
    // The lit side faces upper left rather than following a symmetric rim.
    const int lit = (-(tip.y - root.y) + (tip.x - root.x) <= 0) ? 1 : -1;
    const Point shadow[] = {leaf_point(root, tip, 28, 0), leaf_point(root, tip, 52, -lit * (width - 1)),
        leaf_point(root, tip, 78, -lit * (width / 2)), leaf_point(root, tip, 92, 0)};
    canvas.polygon(shadow, 4, palette.shadow);
    const Point highlight[] = {leaf_point(root, tip, 22, lit), leaf_point(root, tip, 48, lit * (width - 2)),
        leaf_point(root, tip, 75, lit * (width / 2)), leaf_point(root, tip, 64, lit)};
    canvas.polygon(highlight, 4, palette.accent);
    canvas.line(leaf_point(root, tip, 9, 0), leaf_point(root, tip, 83, 0), palette.accent);
}

Point crown_point(const Pose& pose, int x, int y) {
    return {pose.head.x + x * pose.head_width / 100 + y * pose.tilt / 16, pose.head.y + y};
}

void crown(Canvas& canvas, const Pose& pose, const PlantPalette& palette) {
    static const Point shape[] = {{0,-15}, {-3,-12}, {-9,-10}, {-13,-5}, {-13,3}, {-10,9}, {-5,13},
                                 {3,14}, {10,10}, {14,4}, {13,-3}, {9,-8}, {3,-10}};
    Point transformed[13];
    for (int i = 0; i < 13; ++i) transformed[i] = crown_point(pose, shape[i].x, shape[i].y);
    canvas.polygon(transformed, 13, palette.green);
    for (int i = 0; i < 13; ++i) canvas.line(transformed[i], transformed[(i + 1) % 13], kOutline);
    const Point shadow[] = {crown_point(pose, 12, -2), crown_point(pose, 13, 4), crown_point(pose, 9, 9),
        crown_point(pose, 3, 13), crown_point(pose, -4, 12), crown_point(pose, 3, 10), crown_point(pose, 8, 6)};
    canvas.polygon(shadow, 7, palette.shadow);
    const Point light[] = {crown_point(pose,-10,-5), crown_point(pose,-7,-8), crown_point(pose,-2,-10),
        crown_point(pose,0,-12), crown_point(pose,-1,-6), crown_point(pose,-7,-3), crown_point(pose,-10,1)};
    canvas.polygon(light, 7, palette.accent);
    canvas.line(crown_point(pose,-8,-6), crown_point(pose,-4,-8), palette.glint);
    canvas.pixel(crown_point(pose,0,-12).x, crown_point(pose,0,-12).y, palette.glint);
}

void face(Canvas& canvas, const Pose& pose, PlantMood mood, uint32_t tick, const PlantPalette& palette) {
    const int x = pose.head.x;
    const int y = pose.head.y;
    const bool blink = tick % 29u == 23u || tick % 29u == 24u;
    const bool weary = mood == PlantMood::Thirsty || mood == PlantMood::Hot || mood == PlantMood::AirDry;
    const bool worried = mood == PlantMood::Soggy || mood == PlantMood::Cold;
    if (blink) {
        canvas.rect(x - 6, y + 1, 3, 1, kFace);
        canvas.rect(x + 4, y + 1, 3, 1, kFace);
    } else if (weary) {
        canvas.rect(x - 6, y, 4, 1, kFace);
        canvas.rect(x + 3, y, 4, 1, kFace);
        canvas.rect(x - 5, y + 1, 2, 2, kFace);
        canvas.rect(x + 4, y + 1, 2, 2, kFace);
    } else {
        canvas.rect(x - 6, y - 1, 3, 4, kFace);
        canvas.rect(x + 4, y - 1, 3, 4, kFace);
        const int look_y = mood == PlantMood::Dim ? y - 1 : y;
        canvas.pixel(x - 6, look_y, kEyeGlint);
        canvas.pixel(x + 4, look_y, kEyeGlint);
        if (worried) {
            canvas.line({x - 7, y - 4}, {x - 4, y - 5}, kFace);
            canvas.line({x + 4, y - 5}, {x + 7, y - 4}, kFace);
        }
    }
    if (mood == PlantMood::Content) {
        canvas.pixel(x - 3, y + 6, kFace); canvas.pixel(x + 3, y + 6, kFace);
        canvas.rect(x - 2, y + 7, 5, 1, kFace);
    } else if (worried) {
        canvas.rect(x - 2, y + 6, 4, 1, kFace);
        canvas.pixel(x - 3, y + 7, kFace); canvas.pixel(x + 2, y + 7, kFace);
    } else if (mood == PlantMood::Hot) {
        canvas.rect(x - 1, y + 6, 3, 3, kFace);
        canvas.pixel(x, y + 7, palette.shadow);
    } else if (mood == PlantMood::Dim) {
        canvas.rect(x, y + 6, 2, 2, kFace);
    } else {
        canvas.rect(x - 2, y + 7, 5, 1, kFace);
    }
}

uint8_t amount(float value, float low, float high) {
    if (high <= low || value <= low) return 0;
    if (value >= high) return 255;
    return static_cast<uint8_t>((value - low) * 255.0f / (high - low) + 0.5f);
}

PlantPetRange visual_range(PlantChannel channel, const PlantPetProfile* profile) {
    PlantPetRange range = plant_pet_range(channel);
    const uint8_t index = static_cast<uint8_t>(channel);
    if (profile && index < 3) {
        range.opt_low = profile->references[index].low;
        range.opt_high = profile->references[index].high;
    }
    return range;
}
float visual_value(const PlantPetInputs* readings, PlantChannel channel, PlantBand band, bool& known,
                   const PlantPetProfile* profile = nullptr) {
    const unsigned index = static_cast<unsigned>(channel);
    const PlantPetRange range = visual_range(channel, profile);
    known = band != PlantBand::Unknown;
    if (readings) {
        const float value = readings->values[index];
        known = known && readings->valid[index] && isfinite(value);
        if (known) return value < range.min_value ? range.min_value :
            value > range.max_value ? range.max_value : value;
    }
    if (band == PlantBand::Low) return (range.min_value + range.opt_low) * 0.5f;
    if (band == PlantBand::High) return (range.opt_high + range.max_value) * 0.5f;
    return (range.opt_low + range.opt_high) * 0.5f;
}

void growing_line(Canvas& canvas, Point start, Point end, uint8_t extent, uint16_t color) {
    canvas.line(start, blend_point(start, end, extent), color);
}

void soil(Canvas& canvas, PlantBand band, uint32_t tick, const PlantPetInputs* readings,
          const PlantPetProfile* profile) {
    bool known;
    const float value = visual_value(readings, PlantChannel::Soil, band, known, profile);
    const PlantPetRange range = visual_range(PlantChannel::Soil, profile);
    const uint8_t moisture = amount(value, range.min_value, range.max_value);
    // Relative sensor level controls brown shading, not actual water volume.
    const uint16_t fill = !known ? kUnknownSoil : moisture < 128
        ? blend_color(kDrySoil, kSoil, static_cast<uint8_t>(moisture * 2))
        : blend_color(kSoil, kWetSoil, static_cast<uint8_t>((moisture - 128) * 2));
    const uint16_t edge = !known ? kSoilDark : blend_color(kSoilEdge, kWetSoil, moisture);
    canvas.rect(1, 82, 52, 13, fill);
    canvas.rect(1, 82, 52, 1, edge);
    canvas.rect(1, 93, 52, 2, kSoilDark);
    canvas.rect(8, 88, 3, 1, edge);
    canvas.rect(36, 90, 2, 1, kSoilDark);
    canvas.rect(46, 87, 3, 1, kSoilDark);
    if (!known) return;
    if (band == PlantBand::Low) {
        const uint8_t dryness = amount(range.opt_low - value, 0.0f, range.opt_low - range.min_value);
        // Small surface cracks grow downwards, then branch as drying advances.
        const uint8_t trunk = static_cast<uint8_t>(lesser(255, dryness * 2 + 1));
        growing_line(canvas, {12,82}, {15,87}, trunk, kSoilDark);
        growing_line(canvas, {38,82}, {35,86}, trunk, kSoilDark);
        if (dryness > 96) {
            const uint8_t branch = static_cast<uint8_t>((dryness - 96) * 255 / 159);
            growing_line(canvas, {15,87}, {11,90}, branch, kSoilDark);
            growing_line(canvas, {15,87}, {20,89}, branch, kSoilDark);
            growing_line(canvas, {35,86}, {39,91}, branch, kSoilDark);
        }
        if (dryness > 192)
            growing_line(canvas, {24,82}, {22,86}, static_cast<uint8_t>((dryness - 192) * 255 / 63), kSoilDark);
    } else if (band == PlantBand::High) {
        const uint8_t wetness = amount(value, range.opt_high, range.max_value);
        const int width = 1 + wetness * 25 / 255;
        const int depth = 1 + wetness * 7 / 255;
        const uint16_t water = blend_color(kWetSoil, kWater, static_cast<uint8_t>(104 + wetness * 151 / 255));
        // Two small wet patches expand and meet at the top of the scale.
        for (int pool = 0; pool < 2; ++pool) {
            const int x = 1 + pool * 26 + (26 - width) / 2;
            canvas.rect(x, 82, width, depth, water);
            const int drift = static_cast<int>((tick / 4 + pool) % 2u);
            canvas.rect(x + (width > 2 ? 1 + drift : 0), 82,
                        lesser(width > 2 ? width - 2 : width, 5), 1, kWaterGlint);
        }
    }
}

void surroundings(Canvas& canvas, const PlantPetSnapshot& pet, uint32_t tick,
                  const PlantPetInputs* readings) {
    bool light_known, temp_known, air_known;
    const float light = visual_value(readings, PlantChannel::Light, pet.bands[3], light_known);
    const float temp = visual_value(readings, PlantChannel::Temperature, pet.bands[1], temp_known);
    const float air = visual_value(readings, PlantChannel::Air, pet.bands[2], air_known);
    const PlantPetRange& light_range = plant_pet_range(PlantChannel::Light);
    const PlantPetRange& air_range = plant_pet_range(PlantChannel::Air);
    if (light_known && light >= 20.0f) {
        const uint8_t brightness = amount(light, 20.0f, light_range.max_value);
        const uint16_t sun = blend_color(rgb(192, 144, 24), kSun, brightness);
        // Four authored native sizes, readable below the LDR's 8000 lux ceiling.
        const int radius = light < 200.0f ? 1 : light < 800.0f ? 2 : light < 3000.0f ? 3 : 4;
        canvas.disc({8, 8}, radius, sun);
        const int ray = radius + 2;
        canvas.pixel(8, 8 - ray, sun); canvas.pixel(8, 8 + ray, sun);
        canvas.pixel(8 - ray, 8, sun); canvas.pixel(8 + ray, 8, sun);
    } else if (light_known) {
        // Simple low-light disc; <20 lux is an illustration cutoff, not night.
        canvas.disc({8, 8}, 3, kMoon);
    }
    // Bounded environmental particles with stable paths and no shared RNG.
    // Reserve room for air cues when temperature and air need attention.
    const bool cold = temp_known && temp < 15.0f;
    const bool hot = temp_known && !cold && temp >= 27.0f;
    const bool humid = air_known && air > air_range.opt_high;
    const bool dry = air_known && air < air_range.opt_low;
    const uint8_t air_strength = humid ? amount(air, air_range.opt_high, air_range.max_value)
        : dry ? amount(air_range.opt_low-air, 0.0f, air_range.opt_low-air_range.min_value) : 0;
    int thermal_count = cold ? lesser(3, 1 + amount(15.0f-temp,0.0f,7.0f) / 86)
        : hot ? (temp < 30.0f ? 1 : temp < 33.0f ? 2 : temp < 36.0f ? 3 : 4) : 0;
    if ((humid || dry) && thermal_count > 3) thermal_count = 3;
    const int air_count = (humid || dry) ? lesser(4 - thermal_count, air_strength < 128 ? 1 : 2) : 0;
    static const Point cold_slots[] = {{46,18},{7,25},{46,62},{7,61},{8,73},{46,73}};
    static const Point air_slots[] = {{1,35},{49,35},{1,58},{49,58}};
    for (int particle=0; particle<thermal_count; ++particle) {
        const uint32_t clock = tick + static_cast<uint32_t>(particle * (cold ? 5 : 3));
        const uint32_t period = cold ? 18u : 12u;
        const uint32_t phase = clock % period;
        if (phase >= (cold ? 14u : 10u)) continue;
        uint32_t seed = clock / period + static_cast<uint32_t>(particle * 17);
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        if (cold) {
            const Point slot = cold_slots[particle * 2 + (seed & 1u)];
            const int x = slot.x + (phase >= 8u ? 1 : 0);
            const int y = slot.y + static_cast<int>(phase / 4u);
            canvas.line({x-2,y},{x+2,y},kCold);
            canvas.line({x,y-2},{x,y+2},kCold);
            canvas.pixel(x-1,y-1,kCold); canvas.pixel(x+1,y-1,kCold);
            canvas.pixel(x-1,y+1,kCold); canvas.pixel(x+1,y+1,kCold);
        } else {
            static const Point heat_slots[]={{5,22},{5,60},{48,24},{48,70},{4,74},{4,42},{49,44},{48,10}};
            const Point slot = heat_slots[particle * 2 + (seed & 1u)];
            const int x=slot.x, y=slot.y-static_cast<int>(phase/3u);
            canvas.line({x,y},{x-1,y+2},kHeat);
            canvas.line({x-1,y+2},{x+1,y+4},kHeat);
            canvas.line({x+1,y+4},{x,y+6},kHeat);
        }
    }
    for (int particle=0; particle<air_count; ++particle) {
        const uint32_t period=22u-air_strength/64u;
        const uint32_t clock=tick+static_cast<uint32_t>(particle*7);
        const uint32_t phase=clock%period;
        if(phase>=period-5u)continue;
        uint32_t seed=clock/period+static_cast<uint32_t>(particle*23);
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        const Point slot=air_slots[particle*2+(seed&1u)];
        const int drift=static_cast<int>(phase/7u);
        const int x=slot.x+(slot.x<20 ? drift : -drift), y=slot.y;
        if(humid){
            canvas.rect(x,y,4,1,kAir); canvas.rect(x+1,y+2,3,1,kAir);
        } else {
            canvas.rect(x,y,2,1,kDryAir); canvas.pixel(x+4,y,kDryAir);
            canvas.rect(x+1,y+2,2,1,kDryAir);
        }
    }
    if (pet.unknown_mask) {
        // Small unfilled diamond marks missing evidence without a sad gesture.
        canvas.line({47, 45}, {50, 48}, kUnknownMarker);
        canvas.line({50, 48}, {47, 51}, kUnknownMarker);
        canvas.line({47, 51}, {44, 48}, kUnknownMarker);
        canvas.line({44, 48}, {47, 45}, kUnknownMarker);
    }
}

} // namespace

void plant_pet_render(uint16_t* pixels, const PlantPetSnapshot& pet, uint32_t now_ms,
                      const PlantPetInputs* readings, const PlantPetProfile* profile) {
    if (!pixels) return;
    Canvas canvas(pixels);
    canvas.rect(0, 0, PLANT_PET_ART_W, PLANT_PET_ART_H, kBackground);
    // Quantization matches the 220 ms UI cadence; all held frames are >=220 ms.
    const uint32_t tick = now_ms / 220u;
    Pose pose;
    PlantPalette palette = mood_palette(PlantMood::Content);
    PlantMood expression = pet.mood;
    uint32_t total = 0;
    for (uint8_t i=0;i<PLANT_PET_POSE_COUNT;++i) total += pet.pose_weights[i];
    if (total != 0) {
        int32_t sums[14] = {};
        uint32_t adverse_weight = 0, largest_weight = 0;
        PlantMood strongest = PlantMood::Unknown;
        for (uint8_t i=0;i<PLANT_PET_POSE_COUNT;++i) {
            const int weight = pet.pose_weights[i];
            if (!weight) continue;
            const PlantMood mood = static_cast<PlantMood>(i);
            const Pose p = animated_pose(mood, tick);
            const int values[] = {p.head.x,p.head.y,p.tilt,p.left.x,p.left.y,p.right.x,p.right.y,
                p.small_left.x,p.small_left.y,p.small_right.x,p.small_right.y,p.leaf_width,p.small_width,p.head_width};
            for (int field=0;field<14;++field) sums[field] += values[field] * weight;
            if (mood != PlantMood::Content && mood != PlantMood::Dim && mood != PlantMood::Unknown) {
                adverse_weight += weight;
                if (static_cast<uint32_t>(weight) > largest_weight) {largest_weight=weight;strongest=mood;}
            }
        }
        int values[14];
        for (int field=0;field<14;++field)
            values[field] = (sums[field] + (sums[field] < 0 ? -static_cast<int>(total/2) : static_cast<int>(total/2))) / static_cast<int>(total);
        pose = {{values[0],values[1]},values[2],{values[3],values[4]},{values[5],values[6]},
                {values[7],values[8]},{values[9],values[10]},values[11],values[12],values[13]};
        if (adverse_weight != 0) {
            expression = adverse_weight * 3u < total ? PlantMood::Content
                : adverse_weight * 3u < total * 2u ? PlantMood::Unknown : strongest;
        }
    } else {
    const Pose content = animated_pose(PlantMood::Content, tick);
    const bool previous_thermal = pet.previous_mood == PlantMood::Hot || pet.previous_mood == PlantMood::Cold;
    const bool current_thermal = pet.mood == PlantMood::Hot || pet.mood == PlantMood::Cold;
    const Pose previous = previous_thermal
        ? blend_pose(content, animated_pose(pet.previous_mood, tick), pet.previous_thermal_intensity)
        : animated_pose(pet.previous_mood, tick);
    const Pose current = current_thermal
        ? blend_pose(content, animated_pose(pet.mood, tick), pet.thermal_intensity)
        : animated_pose(pet.mood, tick);
    pose = blend_pose(previous, current, pet.transition);
    palette = blend_palette(mood_palette(pet.previous_mood),
                                               mood_palette(pet.mood), pet.transition);
    expression = pet.transition < 128u ? pet.previous_mood : pet.mood;
    const uint8_t strength = pet.transition < 128u ? pet.previous_thermal_intensity : pet.thermal_intensity;
    if (expression == PlantMood::Hot || expression == PlantMood::Cold) {
        if (strength < 85u) expression = PlantMood::Content;
        else if (strength < 170u) expression = PlantMood::Unknown; // Calm, neutral face.
    }
    }
    surroundings(canvas, pet, tick, readings);
    soil(canvas, pet.bands[0], tick, readings, profile);
    stem(canvas, pose, palette);
    leaf(canvas, stem_point(pose, 5), pose.small_left, pose.small_width, palette);
    leaf(canvas, stem_point(pose, 6), pose.small_right, pose.small_width, palette);
    leaf(canvas, stem_point(pose, 11), pose.left, pose.leaf_width, palette);
    leaf(canvas, stem_point(pose, 10), pose.right, pose.leaf_width, palette);
    crown(canvas, pose, palette);
    face(canvas, pose, expression, tick, palette);
}
