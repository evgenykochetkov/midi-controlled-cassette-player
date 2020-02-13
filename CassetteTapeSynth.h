#ifndef CassetteTapeSynth_h
#define CassetteTapeSynth_h

#include <Adafruit_MCP4725.h>

class CassetteTapeSynth {
  private:
    Adafruit_MCP4725* _dac;

    float _currentDacValue = 0;
    uint16_t _targetDacValue = 0;

    bool _isPressed = false;
    byte _currentNote = 0;
    float _bend = 0;
    float _glide = 0;

  public:
    CassetteTapeSynth(Adafruit_MCP4725* dac);
    void noteOn(byte note); // midi note number
    void noteOff(byte note); // midi note number
    void setBend(float amount); // from -1 to 1
    void setGlide(float amount); // from 0 to 1
    void updateTapeSpeed(); // must be called repeatedly
};

#endif
