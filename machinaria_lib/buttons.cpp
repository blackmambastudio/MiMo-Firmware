/**
 * MiMo 0.3 Material Module
 * by @agar3s
*/
#include <Keyboard.h>
#include <buttons.h>

void readInput(Button *current)
{
  if(current->locked) {
    return;
  }
  bool high = digitalRead(current->ID)==HIGH;
  
  if(!current->pressed && high) {
    // on pressed
    current->pressed = true;
    current->switch_active = !current->switch_active;
    Keyboard.press(current->letter);
  }else if(current->pressed && !high) {
    // on released
    current->pressed = false;
    Keyboard.release(current->letter);
  }

  current->on = (!current->mode&&current->switch_active)||(current->mode&&current->pressed);
}

void setColorLight(Button *current, uint32_t color)
{
  current->light = color;
}