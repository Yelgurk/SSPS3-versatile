/* Library to interface with a matrixed keypad.
   There are already numerous other libraries available. I created this on as
   a simpler alternative, both from a usage standpoint and for ease of
   understanding the code.

   https://github.com/Andy4495/matrix-keypad-library
*/
/* Version History
   1.0.0    03/05/2019  A.T.   Original
   1.0.1    04/05/2019  A.T.   Added support for "Keep Active" mode
   1.1.0    12/06/2019  A.T.   Added debouncing methods.

*/
#ifndef KEYPAD4495_LIBRARY
#define KEYPAD4495_LIBRARY

#include "Arduino.h"

class Keypad4495 {
public:
  enum {NO_KEY=0, NO_PIN=255};
  Keypad4495(char *userKeymap, uint8_t *rowPins, uint8_t *colPins, uint8_t numRows, uint8_t numCols, bool keepActive=false);
  char waitForKey();          // Blocking call -- waits for a keypress before returning
  char waitForKeyWithDebounce();  // Blocking call - waits for a keypress and debounces before returning
  char getKey();              // Non-blocking. Returns first key found in scan. Returns NO_KEY if no keys are pressed
  char getKeyWithDebounce();  // Returns first key found in scan and debounces. Returns NO_KEY if no keys are pressed
                              // Non-blocking if no keys are pressed, but blocks while debouncing if a key is pressed
  void getMatrixStatus(uint8_t* matrix_array); // Scans full matrix and sets corresponding element to 1 if that key is pressed
  void setDebounce(uint8_t debounce_time); // Sets the debounce time (defaults to 20ms in constructor)
  uint8_t getDebounce();      // Returns current _debounce_ms setting

private:
  char *_keymap;
  uint8_t *_rowPins;
  uint8_t *_colPins;
  uint8_t _numRows;
  uint8_t _numCols;
  bool _keepActive;
  uint8_t _debounce_ms;
  void disableCols();
  void enableCols();

  long _debounce_start_millis = 0;
  char _debounced_btn         = Keypad4495::NO_KEY;
  bool _is_debouncing         = false;
  bool _is_no_key_delay       = false;

};

#endif