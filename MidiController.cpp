#include "MidiController.h"

namespace MidiController {
  
  /* TODO: MidiController_config.h */
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// -----------------------------------------------------------------------------
  
  bool MidiPreset::load(int index, MidiType inType, DataByte inData1,DataByte inData2, Channel inChannel) {
    if (index >= 0 && index < _midimessage_amount) {
      _midimessage[index].inType = inType;
      _midimessage[index].inData1 = inData1;
      _midimessage[index].inData2 = inData2;
      _midimessage[index].inChannel = inChannel;
      
      return true;
    }
    return false;
  }
  
  bool MidiPreset::init(int midimessage_amount) {
    if (midimessage_amount > 0 && midimessage_amount <= MIDIMESSAGE_MAX) {
      _midimessage_amount = midimessage_amount;

      /* Set NULL MIDI Channel to every messages */
      for (int index = 0; index < midimessage_amount; index++) {
        _midimessage[index].inChannel = MIDI_CHANNEL_OFF;
      }
      
      return true;
    }
    return false;
  }

  void MidiPreset::sendMessage() {
    for (int index = 0; index < _midimessage_amount; index++) {
      Serial.print("Message: ");
      Serial.println(index);
      Serial.println(_midimessage[index].inType);
      Serial.println(_midimessage[index].inData1);
      Serial.println(_midimessage[index].inData2);
      Serial.println(_midimessage[index].inChannel);
      MIDI.send(_midimessage[index].inType,
                _midimessage[index].inData1,
                _midimessage[index].inData2,
                _midimessage[index].inChannel);
    }
  }

  int MidiPreset::getMidiMessageAmount() {
    return _midimessage_amount;
  }

  const MidiMessage* MidiPreset::getMidiMessage(int index) {
    return &_midimessage[index];
  }
  
  MidiPreset::MidiPreset() {
    _name[0] = '\0'; // Set NULL to the head of string
    _midimessage_amount = 0;
  }

// -----------------------------------------------------------------------------

  MidiBank::MidiBank() {
    _midipreset_amount = 0;
    _name[0] = '\0';
  }

  bool MidiBank::init(int midipreset_amount) {
    if (midipreset_amount > 0 && midipreset_amount <= MIDIPRESET_MAX) {
      _midipreset_amount = midipreset_amount;
      return true;
    }

    return false;
  }

  void MidiBank::setName(const char *name) {
    strncpy(_name, name, MIDIBANK_NAME_MAX);
  }

  const char* MidiBank::getName() {
    return _name;
  }

  int MidiBank::getMidiPresetAmount() {
    return _midipreset_amount;
  }

// -----------------------------------------------------------------------------

  MidiController::MidiController() {
    int _midibank_amount = 0;
    _bank_current = -1;
  }

  void MidiController::init() {
    MIDI.begin(MIDI_CHANNEL_OFF);
  }

  bool MidiController::setBankAmount(int midibank_amount) {
    if (midibank_amount > 0 && midibank_amount <= MIDIBANK_MAX) {
      _midibank_amount = midibank_amount;
      _bank_current = 0;
      return true;
    }
    return false;
  }

  bool MidiController::insertBank() {
    if (_midibank_amount < MIDIBANK_MAX) {
      _midibank_amount += 1;
      if (_midibank_amount == 1) {
        _bank_current = 0;
      }
      return true;
    }

    return false;
  }

  void MidiController::bankUp() {
    // Check if it not initialized
    if (_bank_current == -1) {
      return;
    }

    _bank_current += 1;
    if (_bank_current >= _midibank_amount) {
      _bank_current = 0;
    }

  }

  void MidiController::bankDown() {
    // Check if it not initialized
    if (_bank_current == -1) {
      return;
    }

    _bank_current -= 1;
    if (_bank_current < 0) {
      _bank_current = _midibank_amount - 1;
    }

  }

  void MidiController::sendMessage(int preset_index) {
    // Check if it not initialized
    if (_bank_current == -1) {
      return;
    }

    if (preset_index >= 0 && preset_index < midibank[_bank_current].getMidiPresetAmount()) {
      midibank[_bank_current].midipreset[preset_index].sendMessage();
    }
  }

  int MidiController::getBankCurrent() {
    return _bank_current;
  }

  void MidiController::test() {
    MIDI.send(ProgramChange, 15, 0, 1);
  }

}
