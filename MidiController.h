#ifndef MidiController_h
#define MidiController_h

#include <MIDI.h>
USING_NAMESPACE_MIDI

namespace MidiController {

  /* DEFINITION */
  
  // MidiPreset settings
  #define MIDIMESSAGE_MAX 5
  #define MIDIPRESET_NAME_MAX 16
  
  // MidiBank settings
  #define MIDIBANK_NAME_MAX 16
  #define MIDIPRESET_MAX 4
  
  // MidiController settings
  #define MIDIBANK_MAX 8

// -----------------------------------------------------------------------------
  
  struct MidiMessage {
    MidiType inType;
    DataByte inData1;
    DataByte inData2;
    Channel inChannel;
  };
  
  class MidiPreset {
   public:
    MidiPreset();
    bool init(int midimessage_amount);
    bool load(int index, MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel);
    inline void sendMessage();
    
   private:
    char _name[MIDIPRESET_NAME_MAX + 1];
    int _midimessage_amount;
    MidiMessage _midimessage[MIDIMESSAGE_MAX];
  };
  
  class MidiBank {
   public:
    MidiPreset midipreset[MIDIPRESET_MAX];

    MidiBank();
    bool init(int midipreset_amount);
    int getMidiPresetAmount();
    void setName(const char *name);
    const char *getName();
    
    
   private:
    char _name[MIDIBANK_NAME_MAX + 1];
    int _midipreset_amount;
  };
  
  class MidiController {
   public:
    MidiBank midibank[MIDIBANK_MAX];

    MidiController();
    void init();
    bool setBankAmount(int midibank_amount);
    bool insertBank();
    void bankUp();
    void bankDown();
    void sendMessage(int preset_index);
    void test();
    int getBankCurrent();
   
   private:
    int _midibank_amount;
    int _bank_current;
    
  };
}
#endif
