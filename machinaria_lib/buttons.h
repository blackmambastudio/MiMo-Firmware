/**
 * MiMo 0.3 Material Module
 * by @agar3s
*/
#ifndef buttons_h
#define buttons_h

#include <Keyboard.h>

struct Button {
  bool on;
  bool mode;
  bool locked;
  bool pressed;
  bool switch_active;
  uint8_t ID;
  uint32_t light;
  uint8_t light_index;
  char letter;
};

void readInput(Button *current);
void setColorLight(Button *current, uint32_t color);

#endif