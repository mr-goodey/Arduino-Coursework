#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

char validChannels[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
char channels[26];
String descriptions[26];
int minimum[26];
int maximum[26];
int values[26];
int channelCounter = 0;
char sortChannels[26];
char sortedChannels[26];
long timer = millis();
int displayNum = 0;
byte downArrow[8] = {0b00000,0b00100,0b00100,0b10101,0b01110,0b00100,0b00000};
byte upArrow[8] = {0b00000,0b00100,0b01110,0b10101,0b00100,0b00100,0b00000};
int recent[64];
int count = 0;
int total;
int average = 0;
float c;

enum state_e { SYNCHRONISATION, WAITING_PRESS, WAITING_SELECT };

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.clear();
  lcd.setBacklight(5); // purple
}

bool checkValidity(String input) {
  // returns 0 if not valid and 1 if valid
  bool valid = false;
  char channel = input[1];
  for(int i = 0; i < sizeof(validChannels); i++) {
    if (channel == validChannels[i]) {
      valid = true;
      break;
    }
  }
  if (valid == true) {
    return true;
  } else {
    return false;
  }
}

void addChannel1(String input) {
  // adds channel to array if it is not already in the array
  // also sets minimum to 0 and maximum to 256
  char newChannel = input[1];
  bool allGood = true;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == newChannel) {
      allGood = false;
      break;
    }
  }
  if (allGood == true) {
    channels[channelCounter] = newChannel;
    minimum[channelCounter] = 0;
    maximum[channelCounter] = 255;
    channelCounter += 1;
  }
}

void addChannel2(String input) {
  // adds channel to its alphabetical position
  char newChannel = input[1];
  bool allGood = true;
  for (int i = 0; i < sizeof(sortChannels); i++) {
    if (sortChannels[i] == newChannel) {
      allGood = false;
      break;
    }
  }
  if (allGood == true) {
    int placement;
    for (int i = 0; i < sizeof(sortChannels); i++) {
      if (validChannels[i] == newChannel) {
        placement = i;
        break;
      }
    }
    sortChannels[placement] = newChannel;
  }
}

void addChannel3(String input) {
  // adds channel to new ordered array without white spaces between
  char newChannel = input[1];
  bool allGood = true;
  for (int b = 0; b < sizeof(sortedChannels); b++) {
    if (sortedChannels[b] == newChannel) {
      allGood = false;
      break;
    }
  }
  if (allGood = true) {
    int sort = 0;
    for (int a = 0; a < sizeof(sortedChannels); a++) {
      if (validChannels[a] == sortChannels[a]) {
        sortedChannels[sort] = validChannels[a];
        sort += 1;
      }
    }
  }
}

void updateDescription(char channel, String description) {
  // changes description in array
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  descriptions[placement] = description;
}

String getDescription(char channel) {
  // gets the current description associated with the channel
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  return descriptions[placement];
}

bool checkExist(char channel) {
  // checks whether a channel is currently
  bool allGood = false;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      allGood = true;
      break;
    }
  }
  return allGood;
}

void setMax(char channel, int newMax) {
  // sets the maximum of a channel
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  maximum[placement] = newMax;
}

int getMax(char channel) {
  // returns the maximum of a channel
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  return maximum[placement];
}

void setMin(char channel, int newMin) {
  // sets the minimum of a channel
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  minimum[placement] = newMin;
}

int getMin(char channel) {
  // returns the minimum of a channel
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  return minimum[placement];
}

void setVal(char channel, int newVal) {
  // sets the value of a channel
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  values[placement] = newVal;
}

int getVal(char channel) {
  // returns the value of a channel
  int placement;
  for (int i = 0; i < sizeof(channels); i++) {
    if (channels[i] == channel) {
      placement = i;
      break;
    }
  }
  return values[placement];
}

void readInput() {
  // combines previous functions to correctly read user input
  String input = Serial.readString();
  char function = input[0];
  char channel = input[1];
  String message;
  if (checkValidity(input) == 1) {
    if (function == 'C') {
      addChannel1(input);
      addChannel2(input);
      addChannel3(input);
      String description = input.substring(2);
      updateDescription(channel,description);
    } else if (function == 'X') {
      if (checkExist(channel) == 1) {
        String newMaximum = input.substring(2);
        if (newMaximum.length() > 1) {
          bool allGood = true;
          for (int i = 0; i < (newMaximum.length() - 1); i++) {
            if (isDigit(newMaximum[i])) {
              allGood = true;
            } else {
              allGood = false;
              break;
            }
          }
          if (allGood == 1) {
            int newMax = newMaximum.toInt();
            setMax(channel,newMax);
          }
        }
      }
    } else if (function == 'N') {
      if (checkExist(channel) == 1) {
        String newMinimum = input.substring(2);
        if (newMinimum.length() > 1) {
          bool allGood = true;
          for (int i = 0; i < (newMinimum.length() - 1); i++) {
            if (isDigit(newMinimum[i])) {
              allGood = true;
            } else {
              allGood = false;
              break;
            }
          }
          if (allGood == 1) {
            int newMin = newMinimum.toInt();
            setMin(channel,newMin);
          }
        }
      }
    } else if (function == 'V') {
      if (checkExist(channel) == 1) {
        String newValue = input.substring(2);
        if (newValue.length() > 1) {
          bool allGood = true;
          for (int i = 0; i < (newValue.length() - 1); i++) {
            if (isDigit(newValue[i])) {
              allGood = true;
            } else {
              allGood = false;
              break;
            }
          }
          if (allGood == 1) {
            int newVal = newValue.toInt();
            setVal(channel,newVal);
            updateAverage(newVal);
          }
        }
      }
    }
  }
}

bool tooBig() {
  // checks whether any channel has a value greater than the max
  bool tooBig = false;
  for (int i = 0; i < channelCounter; i++) {
    if (getMin(channels[i]) <= getMax(channels[i])) {
      if (getVal(channels[i]) > getMax(channels[i])) {
        tooBig = true;
        break;
      }
    }
  }
  return tooBig;
}

bool tooSmall() {
  // checks whether any channel has a value smaller than the min
  bool tooSmall = false;
  for (int i = 0; i < channelCounter; i++) {
    if (getMin(channels[i]) <= getMax(channels[i])) {
      if (getVal(channels[i]) < getMin(channels[i])) {
        tooSmall = true;
        break;
      }
    }
  }
  return tooSmall;
}

void changeBacklight() {
  // changes backlight colour depending on input values
  if (tooBig() == 1 && tooSmall() == 1) {
    lcd.setBacklight(3); // yellow
  } else if (tooBig() == 1) {
    lcd.setBacklight(1); // red
  } else if (tooSmall() == 1) {
    lcd.setBacklight(2); // green
  } else {
    lcd.setBacklight(7); // white
  }
}

void correctDisplayNum() {
  // ensures the display number is always valid
  if (channelCounter <= 2) {
    displayNum = 0;
  } else {
    if (displayNum < 0) {
      displayNum = 0;
    } else if (displayNum > (channelCounter - 2)) {
      displayNum = (channelCounter - 2);
    }
  }
}

void updateAverage(int value) {
  // appropriately calculates average after a new value is entered
  if (count <= 63) {
    total = 0;
    recent[count] = value;
    count += 1;
    for (int i = 0; i < count; i++) {
      total += recent[i];
    }
    c = (float)total / (float)count;
    average = c + 0.5;
  } else {
    total = 0;
    for (int i = 0; i < 63; i++) {
      recent[i] = recent[(i + 1)];
    }
    recent[63] = value;
    for (int i = 0; i < 63; i++) {
      total += recent[i];
    }
    c = (float)total / (float)64;
    average = c + 0.5;
  }
}

void displayChannels() {
  // displays channels on screen correctly
  lcd.createChar(1,downArrow);
  lcd.createChar(2,upArrow);
  lcd.clear();
  if (channelCounter == 0) {
    lcd.clear();
  } else if (channelCounter == 1) {
    lcd.setCursor(0,0);
    lcd.print(" ");
    lcd.print(sortedChannels[0]);
    if (getVal(sortedChannels[0]) < 10) {
      lcd.print("  ");
      lcd.print(getVal(sortedChannels[0]));
    } else if (getVal(sortedChannels[0]) < 100) {
      lcd.print(" ");
      lcd.print(getVal(sortedChannels[0]));
    } else {
      lcd.print(getVal(sortedChannels[0]));
    }
    lcd.print(",");
    if (average < 10) {
      lcd.print("  ");
      lcd.print(average);
    } else if (average < 100) {
      lcd.print(" ");
      lcd.print(average);
    } else {
      lcd.print(average);
    }
    lcd.print(" ");
    lcd.print(getDescription(sortedChannels[0]));
  } else if (channelCounter == 2) {
    lcd.setCursor(0,0);
    lcd.print(" ");
    lcd.print(sortedChannels[0]);
    if (getVal(sortedChannels[0]) < 10) {
      lcd.print("  ");
      lcd.print(getVal(sortedChannels[0]));
    } else if (getVal(sortedChannels[0]) < 100) {
      lcd.print(" ");
      lcd.print(getVal(sortedChannels[0]));
    } else {
      lcd.print(getVal(sortedChannels[0]));
    }
    lcd.print(",");
    if (average < 10) {
      lcd.print("  ");
      lcd.print(average);
    } else if (average < 100) {
      lcd.print(" ");
      lcd.print(average);
    } else {
      lcd.print(average);
    }
    lcd.print(" ");
    lcd.print(getDescription(sortedChannels[0]));
    lcd.setCursor(0,1);
    lcd.print(" ");
    lcd.print(sortedChannels[1]);
    if (getVal(sortedChannels[1]) < 10) {
      lcd.print("  ");
      lcd.print(getVal(sortedChannels[1]));
    } else if (getVal(sortedChannels[1]) < 100) {
      lcd.print(" ");
      lcd.print(getVal(sortedChannels[1]));
    } else {
      lcd.print(getVal(sortedChannels[1]));
    }
    lcd.print(",");
    if (average < 10) {
      lcd.print("  ");
      lcd.print(average);
    } else if (average < 100) {
      lcd.print(" ");
      lcd.print(average);
    } else {
      lcd.print(average);
    }
    lcd.print(" ");
    lcd.print(getDescription(sortedChannels[1]));
  } else {
    if (displayNum == 0) {
      lcd.setCursor(0,0);
      lcd.print(" ");
      lcd.print(sortedChannels[displayNum]);
      if (getVal(sortedChannels[displayNum]) < 10) {
        lcd.print("  ");
        lcd.print(getVal(sortedChannels[displayNum])); 
      } else if (getVal(sortedChannels[displayNum]) < 100) {
        lcd.print(" ");
        lcd.print(getVal(sortedChannels[displayNum]));
      } else {
        lcd.print(getVal(sortedChannels[displayNum]));
      }
      lcd.print(",");
      if (average < 10) {
        lcd.print("  ");
        lcd.print(average);
      } else if (average < 100) {
        lcd.print(" ");
        lcd.print(average);
      } else {
        lcd.print(average);
      }
      lcd.print(" ");
      lcd.print(getDescription(sortedChannels[displayNum]));
      lcd.setCursor(0,1);
      lcd.write((byte)1);
      lcd.print(sortedChannels[(displayNum + 1)]);
      if (getVal(sortedChannels[(displayNum + 1)]) < 10) {
        lcd.print("  ");
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      } else if (getVal(sortedChannels[(displayNum + 1)]) < 100) {
        lcd.print(" ");
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      } else {
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      }
      lcd.print(",");
      if (average < 10) {
        lcd.print("  ");
        lcd.print(average);
      } else if (average < 100) {
        lcd.print(" ");
        lcd.print(average);
      } else {
        lcd.print(average);
      }
      lcd.print(" ");
      lcd.print(getDescription(sortedChannels[(displayNum + 1)]));
    } else if (displayNum == (channelCounter - 2)) {
      lcd.setCursor(0,0);
      lcd.write((byte)2);
      lcd.print(sortedChannels[displayNum]);
      if (getVal(sortedChannels[displayNum]) < 10) {
        lcd.print("  ");
        lcd.print(getVal(sortedChannels[displayNum]));
      } else if (getVal(sortedChannels[displayNum]) < 100) {
        lcd.print(" ");
        lcd.print(getVal(sortedChannels[displayNum]));
      } else {
        lcd.print(getVal(sortedChannels[displayNum]));
      }
      lcd.print(",");
      if (average < 10) {
        lcd.print("  ");
        lcd.print(average);
      } else if (average < 100) {
        lcd.print(" ");
        lcd.print(average);
      } else {
        lcd.print(average);
      }
      lcd.print(" ");
      lcd.print(getDescription(sortedChannels[displayNum]));
      lcd.setCursor(0,1);
      lcd.print(" ");
      lcd.print(sortedChannels[(displayNum + 1)]);
      if (getVal(sortedChannels[(displayNum + 1)]) < 10) {
        lcd.print("  ");
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      } else if (getVal(sortedChannels[(displayNum + 1)]) < 100) {
        lcd.print(" ");
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      } else {
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      }
      lcd.print(",");
      if (average < 10) {
        lcd.print("  ");
        lcd.print(average);
      } else if (average < 100) {
        lcd.print(" ");
        lcd.print(average);
      } else {
        lcd.print(average);
      }
      lcd.print(" ");
      lcd.print(getDescription(sortedChannels[(displayNum + 1)]));
    } else if ((displayNum > 0) && (displayNum < (channelCounter - 2))) {
      lcd.setCursor(0,0);
      lcd.write((byte)2);
      lcd.print(sortedChannels[displayNum]);
      if (getVal(sortedChannels[displayNum]) < 10) {
        lcd.print("  ");
        lcd.print(getVal(sortedChannels[displayNum]));
      } else if (getVal(sortedChannels[displayNum]) < 100) {
        lcd.print(" ");
        lcd.print(getVal(sortedChannels[displayNum]));
      } else {
        lcd.print(getVal(sortedChannels[displayNum]));
      }
      lcd.print(",");
      if (average < 10) {
        lcd.print("  ");
        lcd.print(average);
      } else if (average < 100) {
        lcd.print(" ");
        lcd.print(average);
      } else {
        lcd.print(average);
      }
      lcd.print(" ");
      lcd.print(getDescription(sortedChannels[displayNum]));
      lcd.setCursor(0,1);
      lcd.write((byte)1);
      lcd.print(sortedChannels[(displayNum + 1)]);
      if (getVal(sortedChannels[(displayNum + 1)]) < 10) {
        lcd.print("  ");
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      } else if (getVal(sortedChannels[(displayNum + 1)]) < 100) {
        lcd.print(" ");
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      } else {
        lcd.print(getVal(sortedChannels[(displayNum + 1)]));
      }
      lcd.print(",");
      if (average < 10) {
        lcd.print("  ");
        lcd.print(average);
      } else if (average < 100) {
        lcd.print(" ");
        lcd.print(average);
      } else {
        lcd.print(average);
      }
      lcd.print(" ");
      lcd.print(getDescription(sortedChannels[(displayNum + 1)]));
    }
  }
}

#ifdef __arm__
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  // returns space between heap and stack as an integer
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

void loop() {
  static enum state_e state = SYNCHRONISATION;
  static int uds;
  static long press_time;
  static int last_b = 0;
  int b;
  int pressed;
  int released;
  switch (state) {
    case SYNCHRONISATION:
      if (Serial.available() > 0) {
        String input = Serial.readString();
        if (input.length() == 2) {
          if (input[0] == 'X') {
            Serial.println();
            Serial.println("UDCHARS,FREERAM,RECENT,NAMES");
            lcd.setBacklight(7);
            state = WAITING_PRESS;
          }
        }
      } else {
        if (millis() - timer >= 1000) {
          Serial.print("Q");
          timer = millis();
        }
      }
      break;
    case WAITING_PRESS:
      if (Serial.available() > 0) {
        readInput();
        changeBacklight();
        displayChannels();
      }
      b = lcd.readButtons();
      pressed = b & ~last_b;
      last_b = b;
      if (pressed & (BUTTON_UP)) {
        displayNum -= 1;
        correctDisplayNum();
        displayChannels();
      } else if (pressed & (BUTTON_DOWN)) {
        displayNum += 1;
        correctDisplayNum();
        displayChannels();
      } else if (pressed & (BUTTON_SELECT)) {
        uds = pressed;
        press_time = millis();
        state = WAITING_SELECT;
      }
      break;
    case WAITING_SELECT:
      if (Serial.available() > 0) {
        readInput();
      }
      if (millis() - press_time >= 1000) {
        press_time = millis();
        if (uds == BUTTON_SELECT) {
          lcd.clear();
          lcd.setBacklight(5);
          lcd.setCursor(0,0);
          lcd.print("F126576");
          lcd.setCursor(0,1);
          lcd.print(freeMemory());
        }
      } else {
        b = lcd.readButtons();
        released = ~b & last_b;
        last_b = b;
        if (released & uds) {
          changeBacklight();
          displayChannels();
          state = WAITING_PRESS;
        }
      }
      break;
  }
}
