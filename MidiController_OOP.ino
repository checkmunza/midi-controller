// -----------------------------------------------------------------------------
// MIDI Controller 
#include "MidiController.h"

MidiController::MidiController midicontroller;
int bank_current;

inline MidiType getMidiType(int val) {
  switch (val) {
    case 0xC0:
      return ProgramChange;
    case 0xB0:
      return ControlChange;
    default:
      return InvalidType;
  }
}

// -----------------------------------------------------------------------------
// Button
#include <Bounce2.h>

#define BUTTON_ITEM 4

const int BUTTON_PIN[BUTTON_ITEM] = {D3, D5, D6, D7};
const int BUTTON_NOT_PRESSED = 0;
const int BUTTON_NOT_PRESSING = 15;

Bounce button[BUTTON_ITEM];
int button_status = BUTTON_NOT_PRESSING;  // Current buttons status
int button_pressed = BUTTON_NOT_PRESSED; // Use when release all buttons

inline void button_setup() {
  for (int index = 0; index < BUTTON_ITEM; index++) {
    pinMode(BUTTON_PIN[index], INPUT_PULLUP);
    button[index] = Bounce(BUTTON_PIN[index], 50);
  }
}

/*! Update button by using bitmask
 *  BUTTON_ITEM MUST NOT EXCEED 32(int)
 */
void button_update() {
  for (int index = 0; index < BUTTON_ITEM; index++) {
    button[index].update();
    if (button[index].read() == 1) {
      button_status |= 1 << index;
    } else {
      button_status &= ~(1 << index);
    }
    button_pressed |= (int)button[index].rose() << index;
  }
}

int button_read() {
  int button_pressed_tmp = button_pressed;
  button_pressed = 0;
  return button_pressed_tmp;
}

// -----------------------------------------------------------------------------
// SPIFFS
#include "FS.h"

// -----------------------------------------------------------------------------
// ArduinoJson
#include <ArduinoJson.h>

// -----------------------------------------------------------------------------

void loopReport(char *message) {
  while (true) {
    Serial.println(message);
    delay(1000);
  }
}

void setup() {
  /* Serial for debug Setup */
  Serial.begin(115200);
  Serial.println("Begin setup");

  /* Button Setup */
  button_setup();

  // ---------------------------------------------------------------------------
  // File Setup
  
  // Loading save.json SPIFFS
  midicontroller.init();
  SPIFFS.begin();
  File saveJsonFile = SPIFFS.open("/save.json", "r");
  // Check if file not exists 
  if (!saveJsonFile) {
    loopReport("File not exists");
  }
  
  /* Loop for loading each MidiBank */
  if (!saveJsonFile.find("\"midibank\": [")) {
    loopReport("Invalid json file");
  }
  while (true) {
    DynamicJsonBuffer json_buffer(1500);
    JsonObject &bank_json = json_buffer.parseObject(saveJsonFile);
    midicontroller.insertBank();
    midicontroller.bankUp();
    bank_current = midicontroller.getBankCurrent();
    midicontroller.midibank[bank_current].init(bank_json["midipreset_amount"]);
    midicontroller.midibank[bank_current].setName(bank_json["name"]);
    JsonArray &midipreset_json = bank_json["midipreset"];
    for (int preset_index = 0; preset_index < bank_json["midipreset_amount"]; preset_index++) {
      int midimessage_amount = midipreset_json[preset_index]["midimessage_amount"];
      midicontroller.midibank[bank_current].midipreset[preset_index].init(midimessage_amount);
      for (int message_index = 0; message_index < midimessage_amount; message_index++) {
        JsonObject &midimessage_json = midipreset_json[preset_index]["midimessage"][message_index];
        MidiType type = getMidiType(midimessage_json["type"].as<int>());
        DataByte data1 = midimessage_json["data1"].as<byte>();
        DataByte data2 = midimessage_json["data2"].as<byte>();
        Channel channel = midimessage_json["channel"].as<byte>();        
        midicontroller.midibank[bank_current].midipreset[preset_index].load(message_index, type, data1, data2, channel);
      }
    }

    // Exits the loop if loaded every banks;
    if (!saveJsonFile.findUntil(",", "]")) {
      saveJsonFile.close();
      midicontroller.bankUp();
      bank_current = midicontroller.getBankCurrent();
      break;
    }
  }

  /*/// MIDI Controller Setup
  midicontroller.setBankAmount(4);
  midicontroller.midibank[0].init(4);
  midicontroller.midibank[0].midipreset[0].init(1);
  midicontroller.midibank[0].midipreset[0].load(0, ProgramChange, 8, 0, 1);
  midicontroller.midibank[0].midipreset[3].init(1);
  midicontroller.midibank[0].midipreset[3].load(0, ProgramChange, 23, 0, 1);
  //*/
  Serial.println(midicontroller.midibank[bank_current].getName());
  Serial.println("End of Setup Function");
}

void loop() {
  button_update();
  if (button_status == BUTTON_NOT_PRESSING) {
    switch (button_pressed) {
      case 0:
        break;
      case 1:
        midicontroller.sendMessage(0);
        break;
      case 8:
        midicontroller.sendMessage(3);
        break;
      case 9:
        midicontroller.bankUp();
        bank_current = midicontroller.getBankCurrent();
        Serial.println(midicontroller.midibank[bank_current].getName());
        break;
    }
    if (button_pressed != 0) {
      Serial.println();
    }
    button_pressed = 0;
  }
}
