/**
   MiMo 0.3 Material Module
   by @agar3s
*/
#include <Keyboard.h>
#include <Mouse.h>
#include <Adafruit_NeoPixel.h>
#include <buttons.h>
#define LIGHTS  10
#define N_LEDS 69
#define BUTTON_0 5
#define BUTTON_1 8
#define BUTTON_2 6
#define BUTTON_3 9
#define BUTTON_4 7


int LEFT[3] = {1, 0, 0};
int RIGHT[3] = {2, 0, 0};

Adafruit_NeoPixel lights = Adafruit_NeoPixel(N_LEDS, LIGHTS, NEO_GRB + NEO_KHZ800);

Button button_0 = {false, false, false, false, false, BUTTON_0, lights.Color(255, 0, 0), 0, 'd'};
Button button_1 = {false, false, false, false, false, BUTTON_1, lights.Color(0, 255, 0), 1, 'f'};
Button button_2 = {false, false, false, false, false, BUTTON_2, lights.Color(0, 0, 255), 2, 'g'};
Button button_3 = {false, false, false, false, false, BUTTON_3, lights.Color(0, 0, 255), 3, 'c'};
Button button_4 = {false, false, false, false, false, BUTTON_4, lights.Color(0, 0, 255), 4, 'v'};

byte ack[3] = {0x7E, 0x00, 0x01};
const byte ID = 0x99;

const byte TOTAL_BUTTONS = 5;
Button* buttons[TOTAL_BUTTONS] = {&button_0, &button_1, &button_2, &button_3, &button_4};
byte i = 0;

float xx = 3.0;
float yy = 3.0;
bool independent_light = true;
bool buttons_active = false;
bool tunner_active = false;

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

  // read variable resistors
  if (tunner_active) {
    float xAxis = read_analog(LEFT);
    float yAxis = read_analog(RIGHT);
    Mouse.move(xAxis, yAxis, 0);
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
    //Serial.write(ack, 4);
    //Serial.flush();
  }
}

void handleMessage(byte command, byte payload[], byte payloadSize) {
  ack[1] = command;
  if (command == 0x01) {
    buttons_active = bool(payload[0]);
    sendAck(0x01);
  } else if (command == 0x02) {
    sendAck(0x01);
    tunner_active = bool(payload[0]);
  } else if (command == 0x03) {
    independent_light = bool(payload[0]);
    sendAck(0x01);
  } else if (command == 0x04) {
    clearMatrix();
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
    setLightsInMatrix(payload);
    sendAck(0x01);
  } else if (command == 0x13) {
    drawInMatrix(payload, payloadSize);
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
  } else if(command == 0x41) {
    ack[3] = 0;
    ack[2] = 4;
    
    read_analog(LEFT);
    read_analog(RIGHT);
    Serial.write(ack, 3);
    byte payload_2[4]= {LEFT[1]>>8, LEFT[1]&0xFF,RIGHT[1]>>8, RIGHT[1]&0xFF};
    Serial.write(payload_2, 4);
    Serial.flush();
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

void setLightsInMatrix(byte payload[]) {
  for (byte i = 1; i < payload[0] * 4 + 1; i += 4) {
    lights.setPixelColor(payload[i], lights.Color(payload[i + 1], payload[i + 2], payload[i + 3]));
  }
}

static void clearMatrix() {
  for (int i = 0; i < 64; i++) {
    lights.setPixelColor(i + TOTAL_BUTTONS, 0);
  }
}

static void drawInMatrix(byte payload[], int totalBuffer) {
  clearMatrix();

  bool set_color = true;
  bool read_once = false;
  uint32_t color_code = 0;
  uint32_t color = 0;

  for (int i = 0; i <= totalBuffer; i++) {
    byte value = payload[i];
    if (set_color) {
      color = lights.Color(payload[i], payload[i + 1], payload[i + 2]);
      i += 2;
      set_color = false;
      read_once = false;
    } else if (value == 0 && read_once) {
      set_color = true;
    } else {
      lights.setPixelColor(value + TOTAL_BUTTONS, color);
      read_once = true;
    }
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
/**
   Mouse read analog data
*/
float read_analog(int analog[])
{
  analog[1] = analogRead(analog[0]);
  float distance = analog[1] - analog[2];

  if (distance < 4.0 && distance > -4.0) {
    distance = 0.0;
  }
  analog[2] = analog[1];
  return -distance / 2;
}
