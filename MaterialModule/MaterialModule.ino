  /**
   MiMo 0.3 Material Module
   by @agar3s


# material leds ids: 0-27
# optimization leds ids: 28-96 // 0 - 68

# buttons ids
# +----- mat ----+---- opt ----+---- mat ----+
# | button_a  0  |             | button_d  7 |
# | button_b  1  |             | button_e  6 |
# | button_c  2  | 08  10  12  | button_f  5 |
# |              |   09  11    |             |
# | button_no 3  | button_0-4  | button_ok 4 |
# +--------------+-------------+-------------+
*/

#include <Keyboard.h>
#include <Adafruit_NeoPixel.h>
#include <buttons.h>
#define LIGHTS  10
#define N_LEDS 28
#define BUTTON_A 6
#define BUTTON_B 7
#define BUTTON_C 8
#define BUTTON_D 4
#define BUTTON_E 3
#define BUTTON_F 2
#define BUTTON_OK 5
#define BUTTON_NO 9


Adafruit_NeoPixel lights = Adafruit_NeoPixel(N_LEDS, LIGHTS, NEO_GRB + NEO_KHZ800);

Button button_a = {false, true, false, false, false, BUTTON_A, lights.Color(255, 0, 0), 0, 'q'};
Button button_b = {false, true, false, false, false, BUTTON_B, lights.Color(0, 255, 0), 1, 'a'};
Button button_c = {false, true, false, false, false, BUTTON_C, lights.Color(0, 0, 255), 2, 'z'};
Button button_d = {false, true, false, false, false, BUTTON_D, lights.Color(255, 0, 255), 4, 'o'};
Button button_e = {false, true, false, false, false, BUTTON_E, lights.Color(255, 255, 185), 5, 'k'};
Button button_f = {false, true, false, false, false, BUTTON_F, lights.Color(255, 0, 0), 6, 'm'};
Button button_ok = {false, true, false, false, false, BUTTON_OK, lights.Color(120, 255, 120), 7, 'i'};
Button button_no = {false, true, false, false, false, BUTTON_NO, lights.Color(255, 120, 120), 3, 'w'};

byte ack[4] = {0x7E, 0x01, 0x01, 0xFF};
const byte ID = 0x66;

const byte TOTAL_BUTTONS = 8;
Button* buttons[TOTAL_BUTTONS] = {&button_a, &button_b, &button_c, &button_d, &button_e, &button_f, &button_ok, &button_no};
byte i = 0;

bool independent_light = true;
bool buttons_active = false;

void setup()
{
  Serial.begin(9600);
  lights.begin();
  lights.setBrightness(50);
  Keyboard.begin();
  for (i = 0; i < TOTAL_BUTTONS; i++) {
    pinMode(buttons[i]->ID, INPUT);
  }
}

void loop()
{
  delay(5);
  // read buttons
  if (buttons_active) {
    for (i = 0; i < TOTAL_BUTTONS; i++) {
      readInput(buttons[i]);

      if (!independent_light) {
        if (buttons[i]->on) {
          lights.setPixelColor(buttons[i]->light_index, buttons[i]->light);
        } else {
          lights.setPixelColor(buttons[i]->light_index, 0);
        }
      }
    }
  }

  readSerial();
  lights.show();
}

/**
   SERIAL comm

*/
static void readSerial() {
  while (Serial.available() > 0) {
    byte byteBuffer[100];
    Serial.readBytes(byteBuffer, 3);
    byte start = byteBuffer[0];

    // if the starting byte is not 0x7E drain all the incoming data
    if (start != 0x7E) {
      if (Serial.available() > 0) {
        Serial.read();
      }
      return;
    }

    byte command = byteBuffer[1];
    byte lengthToRead = byteBuffer[2];

    Serial.readBytes(byteBuffer, lengthToRead);
    handleMessage(command, byteBuffer, lengthToRead);
    //byte ack[4] = {0x66, command, byteBuffer, lengthToRead};
    //Serial.write(ack, 4);
    //Serial.flush();
  }
}

void handleMessage(byte command, byte payload[], byte payloadSize) {
  ack[1] = command;
  if (command == 0x01) {
    buttons_active = bool(payload[0]);
    sendAck(0x01);
  } else if (command == 0x03) {
    independent_light = bool(payload[0]);
    sendAck(0x01);
  } else if (command == 0x05) {
    lights.setBrightness(payload[0]);
    sendAck(0x01);
  } else if (command == 0x10) {
    setLightsColor(payload);
    sendAck(0x01);
  } else if (command == 0x11) {
    setLightsStatus(payload);
    sendAck(0x01);
  } else if (command == 0x12) {
    setLightsById(payload);
    sendAck(0x01);
  } else if (command == 0x30) {
    setButtonsMode(payload);
    sendAck(0x01);
  } else if (command == 0x31) {
    setButtonsLock(payload);
    sendAck(0x01);
  } else if (command == 0x32) {
    setButtonsStatus(payload);
    sendAck(0x01);
  } else if (command == 0x91) {
    sendAck(ID);
  }
}
void sendAck(byte value){
    ack[2] = 1;
    byte ackPayload[1] = {value};
    Serial.write(ack, 3);
    Serial.write(ackPayload, 1);
    Serial.flush();  
}

void setLightsColor(byte payload[]) {
  for (byte i = 1; i < payload[0] * 4 + 1; i += 4) {
    setColorLight(buttons[payload[i]], lights.Color(payload[i + 1], payload[i + 2], payload[i + 3]));
  }
}

void setLightsStatus(byte payload[]) {
  for (byte i = 1; i < payload[0] * 2 + 1; i += 2) {
    byte index = payload[i];
    if (payload[i + 1]) {
      lights.setPixelColor(buttons[index]->light_index, buttons[index]->light);
    } else {
      lights.setPixelColor(buttons[index]->light_index, 0);
    }
  }
}

void setLightsById(byte payload[]) {
  for (byte i = 1; i < payload[0] * 4 + 1; i += 4) {
    lights.setPixelColor(payload[i], lights.Color(payload[i + 1], payload[i + 2], payload[i + 3]));
  }
}

void setButtonsMode(byte payload[]) {
  for (byte i = 1; i < payload[0] * 2 + 1; i += 2) {
    buttons[payload[i]]->mode = bool(payload[i + 1]);
  }
}

void setButtonsLock(byte payload[]) {
  for (byte i = 1; i < payload[0] * 2 + 1; i += 2) {
    buttons[payload[i]]->locked = bool(payload[i + 1]);
  }
}

void setButtonsStatus(byte payload[]) {
  for (byte i = 1; i < payload[0] * 2 + 1; i += 2) {
    buttons[payload[i]]->switch_active = bool(payload[i + 1]);
    buttons[payload[i]]->pressed = bool(payload[i + 1]);
    buttons[payload[i]]->on = bool(payload[i + 1]);
  }
}
