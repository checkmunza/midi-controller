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

int getIntType(MidiType val) {
  switch (val) {
    case ProgramChange:
      return 0xC0;
    case ControlChange:
      return 0xB0;
    default:
      return 0;
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

void saveBankJson(int bank_index) {
  // TODO: Save midibank JSON
  DynamicJsonBuffer json_buffer(2000);
  JsonObject &bank_json = json_buffer.createObject();
  bank_json["name"] = midicontroller.midibank[bank_index].getName();
  JsonArray &midipreset_arr_json = bank_json.createNestedArray("midipreset");
  for (int preset_index = 0; preset_index < midicontroller.midibank[bank_index].getMidiPresetAmount(); preset_index++) {
    JsonObject &midipreset_json = midipreset_arr_json.createNestedObject();
    JsonArray &midimessage_arr_json = midipreset_json.createNestedArray("midimessage");
    for (int message_index = 0; message_index < midicontroller.midibank[bank_index].midipreset[preset_index].getMidiMessageAmount(); message_index++) {
      const MidiController::MidiMessage *midimessage = midicontroller.midibank[bank_index].midipreset[preset_index].getMidiMessage(message_index);
      JsonObject &midimessage_json = midimessage_arr_json.createNestedObject();
      midimessage_json["type"] = getIntType(midimessage->inType);
      midimessage_json["data1"] = midimessage->inData1;
      midimessage_json["data2"] = midimessage->inData2;
      midimessage_json["channel"] = midimessage->inChannel;
    }
  }
  bank_json.prettyPrintTo(Serial);
}

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
  for (int bank_index = 0; ; bank_index++) {
    // Build file Name
    char file_name[30];
    sprintf(file_name, "/midibank/%02d.json\0", bank_index);    
    File saveJsonFile = SPIFFS.open(file_name, "r");
    // break the loop if it not exists
    if (!saveJsonFile) {
      Serial.print(file_name);
      Serial.println(" not found");
      midicontroller.bankUp();
      bank_current = midicontroller.getBankCurrent();
      break;
    }
    
    DynamicJsonBuffer json_buffer(1500);
    JsonObject &bank_json = json_buffer.parseObject(saveJsonFile);
    saveJsonFile.close();

    // MIDI Bank Setup
    if (!midicontroller.insertBank()) {
      loopReport("MidiController_bank is full.");
    }
    midicontroller.bankUp();
    bank_current = midicontroller.getBankCurrent();
    JsonArray &midipreset_arr_json = bank_json["midipreset"];
    midicontroller.midibank[bank_current].init(midipreset_arr_json.size()); // TODO: Checking midipreset_amount
    midicontroller.midibank[bank_current].setName(bank_json["name"]);
    for (int preset_index = 0; preset_index < midipreset_arr_json.size(); preset_index++) {
      JsonArray &midimessage_arr_json = midipreset_arr_json[preset_index]["midimessage"];
      midicontroller.midibank[bank_current].midipreset[preset_index].init(midimessage_arr_json.size()); // TODO: Checking midimessage_amount
      for (int message_index = 0; message_index < midimessage_arr_json.size(); message_index++) {
        JsonObject &midimessage_json = midimessage_arr_json[message_index];
        MidiType type = getMidiType(midimessage_json["type"].as<int>());
        DataByte data1 = midimessage_json["data1"].as<byte>();
        DataByte data2 = midimessage_json["data2"].as<byte>();
        Channel channel = midimessage_json["channel"].as<byte>();        
        midicontroller.midibank[bank_current].midipreset[preset_index].load(message_index, type, data1, data2, channel);
      }
    }
  }

  Serial.println("End of Setup Function");
  Serial.print("Current Bank: ");
  Serial.println(midicontroller.midibank[bank_current].getName());
}

void loop() {
  button_update();
  if (button_status == BUTTON_NOT_PRESSING) {
    if (button_pressed != 0) {
      Serial.print("Button_pressed: ");
      Serial.println(button_pressed);
    }
    switch (button_pressed) {
      case 0:
        break;
      case 1:
        midicontroller.sendMessage(0);
        break;
      case 2:
        Serial.println("##### saveBankJson #####");
        saveBankJson(bank_current);
        Serial.println();
        Serial.println("##########");
        break;
      case 8:
        midicontroller.sendMessage(3);
        break;
      case 9:
        midicontroller.bankUp();
        bank_current = midicontroller.getBankCurrent();
        Serial.print("Current Bank: ");
        Serial.println(midicontroller.midibank[bank_current].getName());
        break;
    }
    button_pressed = 0;
  }
}
