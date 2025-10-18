#include "misc.h"
#include <Arduino.h>

static const char *quotes[] = {
    "Keep it simple.",
    "Make it work, make it right, make it fast.",
    "Stay curious.",
    "Code. Debug. Repeat.",
    "Dream big. Code bigger.",
    "Be the change you debug.",
    "Every bug is a lesson.",
    "Perfection is iteration.",
    "Think twice, code once.",
    "Simplicity is power."
};

constexpr size_t QUOTE_COUNT = sizeof(quotes) / sizeof(quotes[0]);

const char * get_qotd() {
    return quotes[random(QUOTE_COUNT)];
}
