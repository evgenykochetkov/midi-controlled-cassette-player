#include "CassetteTapeSynth.h"

int notesFromF2ToDacValues [] = {
  247, // f2
  258, // f2#
  273, // g2
  289, // g#2
  306, // a2
  323, // a#2
  337, // b2

  354, // c3
  370, // c#3
  390, // d3
  413, // d#3
  440, // e3
  462, // f3
  484, // f#3
  507, // g3
  537, // g#3
  570, // a3
  599, // a#3
  627, // b3

  664, // c4
  706, // c#4
  741, // d4
  781, // d#4
  831, // e4
  872, // f4
  922, // f#4
  976, // g4
  1023, // g#4
  1088, // a4   <--- original pitch
  1142, // a#4
  1215, // b4

  1277, // c5
};

#define F2_MIDI_NOTE 41
constexpr int notesListLength = sizeof(notesFromF2ToDacValues) / sizeof((notesFromF2ToDacValues)[0]);

int midiNoteToDacValue(byte note) {
  return notesFromF2ToDacValues[constrain(note - F2_MIDI_NOTE, 0, notesListLength - 1)];
}

uint16_t calculateTargetDacValue(bool isPressed, byte note, float bend) {
  if (!isPressed) {
    return 0;
  }

  int16_t dacValue = midiNoteToDacValue(note);
  if (bend != 0) {
    int8_t bendDirection = bend > 0 ? 1 : -1;
    int16_t bendTargetDacValue = midiNoteToDacValue(note + (bendDirection * 2));
    int16_t delta = (bendTargetDacValue - dacValue) * bendDirection * bend;
    dacValue += delta;
  }

  return dacValue;
}

CassetteTapeSynth::CassetteTapeSynth(Adafruit_MCP4725* dac) {
  _dac = dac;
  _currentDacValue = 0;
  _targetDacValue = 0;

  _isPressed = false;
  _currentNote = 0;
  _bend = 0;
  _glide = 0;
}

void CassetteTapeSynth::noteOn(byte note) {
  _currentNote = note;
  _isPressed = true;

  _targetDacValue = calculateTargetDacValue(_isPressed, _currentNote, _bend);
}

void CassetteTapeSynth::noteOff(byte note) {
  if (note == _currentNote) {
    _isPressed = false;

    _targetDacValue = calculateTargetDacValue(_isPressed, _currentNote, _bend);
  }
}

void CassetteTapeSynth::setBend(float amount) {
  _bend = amount;

  _targetDacValue = calculateTargetDacValue(_isPressed, _currentNote, _bend);
}

void CassetteTapeSynth::setGlide(float amount) {
  if (amount == 0) {
    _glide = 0;
    return;
  }

  _glide = (1.1 - amount) * 0.7;
}

// should be called repeatedly
void CassetteTapeSynth::updateTapeSpeed() {
  if (_currentDacValue == _targetDacValue) return;

  if (_glide == 0) {
    _currentDacValue = _targetDacValue;
  } else {
    // TODO: calculate increments based on delta time
    if (_currentDacValue < _targetDacValue) { // going up
      _currentDacValue = min(_currentDacValue + _glide, _targetDacValue);
    } else if (_currentDacValue > _targetDacValue) { // going down
      _currentDacValue = max(_currentDacValue - _glide, _targetDacValue);
    }
  }

  _dac->setVoltage(_currentDacValue, false);
}
