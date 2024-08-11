#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>    
#include <avr/wdt.h> 
#include <TinyWireM.h>
#include <Tiny4kOLED.h>

#define LANG_EN 0
#define LANG_PL 1

// select LANG_EN or LANG_PL

#define LANGUAGE LANG_EN
#define WELCOME_LINE2    "<geocache name>"
#define WELCOME_LINE3    "by PanJanek123" 

#if LANGUAGE == LANG_EN

#define WELCOME_LINE1    "Geocache logbook:" 

#define MENU_LOG         "[log]"
#define MENU_BROWSE      "[browse]"
#define MENU_FINISH      "[finish]"

#define BUTTON_CANCEL    "[cancel]"
#define BUTTON_GO_ON     "[ ok! ]"

#define CONFIRM_LINE1    "Confirm last time"
#define CONFIRM_LINE2    "that nick is correct"

#define BUTTON_GO_BACK   "[back/undo]"

#define MSG_SAVED        "SAVED"

#define GOODBYE_LINE1    "Goodbye"
#define GOODBYE_LINE2    "turning off"
#define GOODBYE_LINE3    "To save battery"

#else

#define WELCOME_LINE1    "Logbook geokesza:" 

#define MENU_LOG         "[wpisz sie]"
#define MENU_BROWSE      "[przegladaj]"
#define MENU_FINISH      "[koniec]"

#define BUTTON_CANCEL    "[anuluj]"
#define BUTTON_GO_ON     "[dalej]"

#define CONFIRM_LINE1    "Potwierdz ostatecznie"
#define CONFIRM_LINE2    "poprawnosc nicka:"

#define BUTTON_GO_BACK   "[cofnij/popraw]"

#define MSG_SAVED        "ZAPISANO"

#define GOODBYE_LINE1    "Do widzenia."
#define GOODBYE_LINE2    "Zasypiam..."
#define GOODBYE_LINE3    "by oszczedzac baterie"

#endif

//power = PB1, button1=PB3, button2=PB4
#define POWER_PIN 1
#define BUTTON1_PIN 3
#define BUTTON2_PIN 4
#define SLEEP_AFTER_SECONDS 30
#define LOGBOOK_MAX 3000
#define LOGBOOK_MAX_STR "3000"

// 24C512 address is 10100<a1><a2> = 80 if a1->a2->GND
#define EXTERNAL_EEPROM_I2C_ADDR 0x50

const uint8_t pointer_up[] PROGMEM = { 0,16,8+16,4+8+16, 4+8+16,8+16,16,0 };
const uint8_t pointer_down[] PROGMEM = { 0,4,4+8,4+8+16,4+8+16,4+8,4,0 };
const uint8_t bar[][1] PROGMEM = { {0}, {128}, {128+64}, {128+64+32}, {128+64+32+16}, {128+64+32+16+8}, {128+64+32+16+8+4},{128+64+32+16+8+4+2},{128+64+32+16+8+4+2+1}};

uint8_t b1 = 0;
uint8_t b2 = 0;
uint8_t previous_b1 = 0;
uint8_t previous_b2 = 0;
bool pressed1 = false;
bool pressed2 = false;
uint8_t previous_mode = 0;
uint8_t mode = 0;             //0=main menu, 1=input, 2=confirmation, 3=browse,  255=fatal error
uint8_t previous_position = 0;
uint8_t position = 0;
long lastActivity;
long timer;
int index;
int size;

char input[22];
char buffer[22];

void setup() {
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  TinyWireM.begin();
  initData();
  startScreen();
}

void wait(int ms){
  timer += ms;
  delay(ms);
}

void waitLong(){
  wait(3000);
}

ISR(PCINT0_vect) {
    // This is called when the interrupt occurs, but I don't need to do anything in it
    }

void sleepUntilTrigger()
{
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin
  PCMSK |= _BV(PCINT4);                   // Use PB4 as interrupt pin (3 or 4 will wake up)
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep
  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on
  sei();                                  // Enable interrupts
}

void initData()
{
  timer = 0;
  lastActivity = timer;
  previous_mode = -1;
  mode = 0;
  position = 0;
  memset(input, ' ', 21);
}

void screenOn()
{
  digitalWrite(POWER_PIN, HIGH);
  wait(10);
  oled.begin(128, 32, sizeof(tiny4koled_init_128x32br), tiny4koled_init_128x32br);
  oled.clear();
  oled.on();
  oled.setFont(FONT6X8);
}

void screenOff()
{
  oled.clear();
  oled.off();
  wait(10);
  digitalWrite(POWER_PIN, LOW);
}

void oledText(uint8_t x,  uint8_t y, const char text[], bool invert = false)
{
  timer+=5+2*strlen(text); // estimate time taken for text display
  oled.invertOutput(invert);
  oled.setCursor(x, y);
  oled.print(text);
  oled.invertOutput(false);
}

void oledText(uint8_t y, const char text[], bool invert = false)
{
  oledText(63 - strlen(text)*6 / 2, y, text, invert);
}

void oledTextCenterLarge(const char text[])
{
  oled.setFont(FONT8X16);
  oledText(64 - strlen(text)*8 / 2, 1, text);
  oled.setFont(FONT6X8);
}

void oledTextP(uint8_t x,  uint8_t y, const __FlashStringHelper * progmemText, bool invert = false)
{
  strcpy_P(buffer, (char*)progmemText);
  oledText(x,  y, buffer, invert);
}

void oledTextP(uint8_t y, const __FlashStringHelper * progmemText, bool invert = false)
{
  strcpy_P(buffer, (char*)progmemText);
  oledText(63 - strlen(buffer)*6 / 2, y, buffer, invert);
}

void readButtons() {
  previous_b1 = b1;
  previous_b2 = b2;
  b1 = digitalRead(BUTTON1_PIN);
  b2 = digitalRead(BUTTON2_PIN);

  pressed1 = previous_b1 == HIGH && b1 == LOW;
  pressed2 = previous_b2 == HIGH && b2 == LOW;
  if (pressed1 || pressed2)
    lastActivity = timer;
}

void startScreen()
{
  screenOn();
  oledText(3, 0, WELCOME_LINE1); 
  oledTextCenterLarge(WELCOME_LINE2); 
  oledTextP(40, 3, F(WELCOME_LINE3));
  waitLong();
  oled.clear();
}

void goodbye()
{
  oled.clear();
  if (mode == 255) {
    oledTextCenterLarge("ERROR");
  } else {
    oledTextP(0, F(GOODBYE_LINE1));
    oledTextCenterLarge(GOODBYE_LINE2);
    oledTextP(3, F(GOODBYE_LINE3));
  }
  waitLong();
  oled.clear();

  initData();
  screenOff();
  sleepUntilTrigger();
  initData();
  startScreen();
}

void advance()
{
  char c = input[position];
  bool upper = (position == 0) || (input[position-1] == ' ');
  if (c == ' ') 
    c = upper ? 'A' : 'a';
  else if (c == (upper ? 'Z' : 'z'))
    c = '0';
  else if (c == '?')
    c = upper ? 'a' : 'A';      
  else if ((c >= 'A' && c <'Z') || (c >= 'a' && c <'z') || (c >= '0' && c <'9')) {
    c = c+1;
  }
  else if (c == '9') 
    c = '-';
  else if (c == '-') 
    c = '_';
  else if (c == '_') 
    c = '.';    
  else if (c == '.') 
    c = '!';    
  else if (c == '!') 
    c = '?';    
  else
    c = ' ';

  input[position] = c;
}

void externalEEPROMStartTransmission(unsigned int eeaddress){
  TinyWireM.beginTransmission(EXTERNAL_EEPROM_I2C_ADDR);
  TinyWireM.send((int)(eeaddress >> 8));
  TinyWireM.send((int)(eeaddress & 0xFF));
}

void writeExternalEEPROM(unsigned int eeaddress, byte data)
{
  externalEEPROMStartTransmission(eeaddress);
  TinyWireM.send(data);
  TinyWireM.endTransmission();
  delay(10);
}

byte readExternalEEPROM(unsigned int eeaddress)
{
  externalEEPROMStartTransmission(eeaddress);
  uint8_t err = TinyWireM.endTransmission();
  if (err == 0) {
    err = TinyWireM.requestFrom(EXTERNAL_EEPROM_I2C_ADDR,1);
  }
  
  delay(5);
  if (!TinyWireM.available())
    delay(10);
  if (TinyWireM.available() && err == 0)  
  {
    return TinyWireM.receive();
  }
  else
  {
    mode=255;    //signal error!
    return 0xFF;
  }
}

void logbookSize() {
  size = (((int)readExternalEEPROM(0)) << 8) + ((int)readExternalEEPROM(1));
  if (size < 0)
    size = 0;
  if (size > LOGBOOK_MAX)
    size = LOGBOOK_MAX;
}

void eepromRead(int addr)
{
  for(int a=0; a<21; a++){
    input[a] = readExternalEEPROM(addr+a);
  }
}

void logbookStore()
{
  logbookSize();
  for(int a=0; a<21; a++){
    writeExternalEEPROM(2 + size*21+a, input[a]);
  }
  size = size + 1;
  writeExternalEEPROM(0, size >> 8);
  writeExternalEEPROM(1, size & 0xFF); // %256
}

void showLog()
{
  oled.clear();
  for(int i=index-2; i<=index; i++)
  {
    if ((i>=0) && (i<size)) {
      eepromRead(2+i*21);
      input[21] = 0;
      oledText(0, 1+i-(index-2), input);
    }
  }

  oledText(0,0, "Nr "); 
  itoa(index, buffer, 10);
  uint8_t len = strlen(buffer);
  oledText(18,0,buffer);
  buffer[0]='/';
  itoa(size, buffer+1, 10);
  oledText(18+len*6, 0, buffer);
}

void navigation()
{
  readButtons();
  previous_position = position;
  previous_mode = mode;
  if (mode == 0) { // menu
    if (pressed1) {
      position = (position+1)%3;
    }
    else if (pressed2) {
      if (position == 0) { // -> zaloguj
        oled.clear();
        logbookSize();
        if (size >= LOGBOOK_MAX) {
          oledTextCenterLarge("Logbook full!");
          waitLong();
          oled.clear();
          previous_mode = -1;
        } else {
          mode = 1;
          memset(input, ' ', 21);
          input[21] = 0;
          position = 0;
        previous_position = 0;
        }
      }
      else if (position == 1) { // -> przegladaj
        logbookSize();
        oled.clear();
        if (size == 0) {
          oledTextCenterLarge("Empty!");
          waitLong();
          oled.clear();
          previous_mode = -1;
        } else {
          index = size;
          mode = 3;
          position = 0;
        }
      } else { // -> koniec
         goodbye();
      }
    }
  } else if (mode == 1) { // input
    uint8_t inputLen = 20;
    while (inputLen>= 0 && input[inputLen] == ' ') inputLen--;
    inputLen++;  
    if (pressed1) { //21 = backspace, 22=dalej, 23=anuluj
      if ((position < 20) && (position > 0) && input[position] == ' ' && input[position-1] == ' ')
        position = 21;
      else
        position = (position+1)%24;
      if ((position == 21 && inputLen == 0) || (position == 22 && inputLen < 3))
        position = 23;       
    }
    else if (pressed2) {
      if (position <21)
        advance();
      else if (position == 21) { // backspace
        position = inputLen-1;
        input[position] = ' ';
      } else if (position == 23) {   // -> menu
        mode = 0; 
        position = 0;
        oled.clear();
      }
      else if (position == 22) {   // -> confirm
        mode = 2; 
        position = 0;
        oled.clear();
      }
    }
  }
  else if (mode == 2) { // confirmation
    if (pressed1) {
      position = (position+1) & 1;
    }
    else if (pressed2) {
      if (position == 0) {        // -> input
        oled.clear();
        mode = 1;
      } else  {  // -> store
        oled.clear();
        logbookStore();
        oledTextCenterLarge(MSG_SAVED);
        waitLong();
        oled.clear();
        mode = 0;
        position = 1;
      }
    }
  } else if (mode == 3) {   // browsing
    if (pressed1) {
      position = (position+1)&3;
    }
    else if (pressed2) {
      if (position == 0) {     // scroll up
        index--;
        if (index < 1)
          index = 1;
        showLog();
      } else if (position == 1) {   // scroll down
        index++;
        if (index > size)
          index = size;
        showLog();
      } else if (position == 2) {  // <-menu
        oled.clear();
        mode = 0;
        position = 1;      
      } else {  // top
        index = 1;
        showLog();
      }
    }
  }
}

void display()
{
  if (mode == 0) { // ------------------- main menu ---------------------
    if (mode != previous_mode) {
     oledTextP(0, F("Menu"));
    } 
    oledTextP(1, F(MENU_LOG), position == 0); 
    oledTextP(2, F(MENU_BROWSE), position == 1);
    oledTextP(3, F(MENU_FINISH), position == 2);
  }
  else if (mode == 1) { // ------------------- input ------------------
    if (mode!=previous_mode) {
      logbookSize();
      itoa(size+1, buffer, 10);
      oledText(0, 0, "Nr "); 
      oledText(18, 0, buffer); 
      uint8_t x = 18+strlen(buffer)*6;
      if (size==0)
        oledText(x, 0, " FTF:)"); 
      else if (size==1)
        oledText(x, 0, " STF:)");
      else if (size==2)
        oledText(x, 0, " TTF:)");   
      else     
        oledText(x, 0, "/" LOGBOOK_MAX_STR ":"); 

      oledText(96, 0, "Nick:");              
      oledText(2, "[backspace]", position == 21);
      oledText(0, 3, BUTTON_CANCEL, position == 23);
      oledText(84, 3, BUTTON_GO_ON, position == 22);  
      oledText(0, 1, input);
    }

    if (position < 21)
    { 
      oled.invertOutput(timer & 128);
      oled.setCursor(position*6,1);
      oled.print(input[position]);
      oled.invertOutput(false);
    }

    if (position != previous_position) 
    {
      if (position >= 21) 
      {
        oledText(2, "[backspace]", position == 21);
        oledText(0, 3, BUTTON_CANCEL, position == 23);
        oledText(84, 3, BUTTON_GO_ON, position == 22);  
      }

      if (previous_position < 21)
      {
        oled.invertOutput(false);
        oled.setCursor(previous_position*6,1);
        oled.print(input[previous_position]);
      }

      if (previous_position == 21)
        oledText(2, "[backspace]", false);
      if (previous_position == 23)
        oledText(0, 3, BUTTON_CANCEL, false);
      if (previous_position == 22)
        oledText(84, 3, BUTTON_GO_ON, false);           
    }
  } else if (mode == 2) { // ------------------- confirmation --------------
    if (mode != previous_mode) {
     oledTextP(0, F(CONFIRM_LINE1));
     oledTextP(1, F(CONFIRM_LINE2));
     oledText(2, input);
    } 

    oledTextP(0,3, F(BUTTON_GO_BACK), position == 0); 
    oledText(102, 3, "[OK]", position == 1);       
  } else if (mode == 3) {  // ------------------- browse -------------------
    if (mode!=previous_mode) {
      showLog(); 
    }

    oled.invertOutput(position == 0);
    oled.bitmap(117,0,125,1,pointer_up);          
    oled.invertOutput(position == 1);
    oled.bitmap(117,3,125,4,pointer_down);     
    oledText(74,0, "[<-]", position == 2);     
    oledText(98,0, "[^]", position == 3);         
  }
}

void showTimeLeft()
{
  int y = 32 * (SLEEP_AFTER_SECONDS*1000 - (timer - lastActivity)) / (SLEEP_AFTER_SECONDS*1000);
  if (y < 0)
    y = 0;
  for(uint8_t i=0; i<4; i++) {
    uint8_t idx = (y>=((i+1)<<3)) ? 8 : ((y<((i+1)<<3)-8) ? 0 : (y&7));
    oled.bitmap(127,3-i, 128, 3-i+1, bar[idx]);
  }
}

void loop() 
{
  display();
  navigation();

  int ttl = SLEEP_AFTER_SECONDS - (timer - lastActivity) / 1000;
  if (ttl == -1 || mode==255) {
    goodbye();
  }
 
  showTimeLeft();
  wait(mode == 1 ? 20 : 5);
}
