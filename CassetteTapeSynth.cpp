#include "CassetteTapeSynth.h"

int notesFromF2ToDacValues [] = {
  255, // f2
  269, // f2#
  283, // g2
  297, // g#2
  313, // a2
  329, // a#2
  344, // b2

  361, // c3
  379, // c#3
  400, // d3
  423, // d#3
  446, // e3
  468, // f3
  491, // f#3
  517, // g3
  546, // g#3
  577, // a3
  607, // a#3
  639, // b3

  676, // c4
  713, // c#4
  750, // d4
  793, // d#4
  839, // e4
  883, // f4
  934, // f#4
  985, // g4
  1038, // g#4
  1096, // a4   <--- original pitch
  1153, // a#4
  1223, // b4

  1290, // c5
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
