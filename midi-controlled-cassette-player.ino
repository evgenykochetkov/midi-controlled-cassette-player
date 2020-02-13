#include <Wire.h>
#include <MIDI.h>
#include "CassetteTapeSynth.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1,  MIDI);

Adafruit_MCP4725 dac;

CassetteTapeSynth synth = CassetteTapeSynth(&dac);

void setup() {
  dac.begin(0x62);
  dac.setVoltage(0, false);
  //  Serial.begin(115200);
  MIDI.begin();
}

void loop() {
  if (MIDI.read()) {
    switch (MIDI.getType()) {
      case midi::NoteOn: {
          synth.noteOn(MIDI.getData1());
          break;
        }

      case midi::NoteOff: {
          synth.noteOff(MIDI.getData1());
          break;
        }

      case midi::PitchBend: {
          float bendAmount = ((float)((MIDI.getData1() & 0x7f) | ((MIDI.getData2() & 0x7f) << 7)) + MIDI_PITCHBEND_MIN) / MIDI_PITCHBEND_MAX;
          synth.setBend(bendAmount);
          break;
        }

      case midi::ControlChange: {
          byte ccNumber = MIDI.getData1();
          if (ccNumber == midi::ModulationWheel) {
            float glideAmount = (float)(MIDI.getData2()) / 127.0;
            synth.setGlide(glideAmount);
          }
          break;
        }

      default:
        break;
    }
  }

  synth.updateTapeSpeed();
}
