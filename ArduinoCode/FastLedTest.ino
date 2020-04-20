#define FASTLED_INTERNAL // add this before including FastLED.h
#include <FastLED.h>
#include <LiquidCrystal.h> //using elecfreaks lcd key shield v1.2

// How many leds in your strip?
#define NUM_LEDS 64

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 11
#define CLOCK_PIN 12

// Define the array of leds
CRGB leds[NUM_LEDS];
CHSV primaryColor = CHSV( 160, 255, 255);
CHSV secondaryColor = CHSV( 0, 255, 255);
CHSV rainbowColor = CHSV( 0, 255, 255);
int primaryHue = 160;   //Blue
int secondaryHue = 0;
#define MINIMUM_BRIGHTNESS 200;    //Setting the minimum brightness lower causes instability in the LED strip
int primaryBrightness = 255;
int secondaryBrightness = 255;
int buttonTime = 200;   //How long buttons are not active after pressing
int buttonTimer = 0;    //Time until buttons are active again
int preset = 0;
float bassResponse = 20; //Should be set between 0 and 100;
int linaTime = 10;
int linaTimer = 0;
int strobeDelay = 50;
int strobeTimer = 0;
int loopDelay = 2;
bool strobeState = true;
float hue = 0; //Used for rainbow effect
bool fadeIncreasing = true;
int incomingByte = 0; //For sending data between Arduino and Processing code


//Presets
#define PRESET_FULL_BASS 0
#define PRESET_PIXEL_MOVE_BASS 1
#define PRESET_STROBE 2
#define PRESET_LINA 3
#define PRESET_FADE 4

//LCD menus
#define MAIN_MENU 1
#define PRESET_MENU 10
#define COLOR_MENU 11
#define BASS_MENU 12
#define STROBE_MENU 13
#define PRIMARY_HUE 110
#define SECONDARY_HUE 111
#define PRIMARY_BRIGHTNESS 1100
#define SECONDARY_BRIGHTNESS 1110

//LCD variables
void  Potentiometer();
const int Encoder_A =  3;            // Incremental Encoder singal A is PD3 
const int Encoder_B =  2;            // Incremental Encoder singal B is PD2 

volatile int selected=0;
volatile int updateScreen=0;
volatile int minOption = 0;
volatile int maxOption = 2;
int menu = MAIN_MENU; //The menu in which the user currently is

// define some values used by the panel and buttons
int lcd_key    = 0;
int adc_key_in = 0;
#define btnBACK   4
#define btnNONE   5 
#define btnSELECT 6
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  lcd.begin(16, 2);   // start the library  
  pinMode(10,OUTPUT);
  digitalWrite(10, 1); //Turns on LCD backlight
  lcd.setCursor(0,0);  
  printToLCD();
  pinMode(Encoder_A, INPUT); 
  pinMode(Encoder_B, INPUT); 
  digitalWrite(Encoder_A, 1);
  digitalWrite(Encoder_B, 1);
  attachInterrupt(digitalPinToInterrupt(3), Potentiometer, FALLING);        //interrupts: numbers 0 (on digital pin 2) and 1 (on digital pin 3).
  
  Serial.begin(9600); 
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  for(int i = 0; i < NUM_LEDS; i++)
  {
     leds[i] = primaryColor;
  }
}

void loop() { 
  timers();
    
  if(updateScreen == 1)
  {  
    lcd.clear();
    printToLCD();
    updateScreen=0;
  }
  
  lcd_key = read_LCD_buttons();  // read the buttons  
  reactToInputLCD(); 
  setColors();
  
  if (preset == PRESET_FULL_BASS) 
  {
    fullStripReact(primaryColor, secondaryColor);
  }
  else if (preset == PRESET_PIXEL_MOVE_BASS)
  {
    pixelMoveReact(primaryColor, secondaryColor);
  }
  else if (preset == PRESET_STROBE)
  {
    fullStrobe(primaryColor, secondaryColor);
  }
  else if (preset == PRESET_LINA)
  {
    rainbowColor = slowlyChangeHue(1, 0, 255, rainbowColor);
    lina(rainbowColor);
  }
  else if (preset == PRESET_FADE)
  {
    fade();
  }
  
  delay(loopDelay);
}
void timers()
{
  if(buttonTimer > 0)
  { 
    buttonTimer = buttonTimer - loopDelay;
  }
  if(strobeTimer > 0)
  { 
    strobeTimer = strobeTimer - loopDelay;
  }
  else 
  {
    strobeTimer = strobeDelay;
    if(strobeState)
      strobeState = false;
    else 
      strobeState = true;
  }
}

void setColors()
{
  primaryColor.hue = primaryHue;
  primaryColor.val = primaryBrightness;
  secondaryColor.hue = secondaryHue;
  secondaryColor.val = secondaryBrightness;
}

void fullStrobe(CHSV defaultColor, CHSV changedColor)
{
  if(strobeState)
  {
    for(int i = NUM_LEDS - 1; i >= 0;i--)
    {
      leds[i] = changedColor;
    }
  }
  else 
  {
    for(int i = NUM_LEDS - 1; i >= 0;i--)
    {
      leds[i] = defaultColor;
    }
  }
  
  FastLED.show();
}

void pixelMoveReact(CHSV defaultColor, CHSV changedColor)
{
  if (Serial.available() > 0) 
  {
  leds[0] = defaultColor;
  
    incomingByte = Serial.read();
    float alpha = incomingByte / 100.0f;
    if(alpha > bassResponse / 100)
    {
      leds[0] = changedColor;
    }
    
    for(int i = NUM_LEDS - 1; i > 0;i--){
     leds[i] = leds[i-1];
    }
    
  }
  FastLED.show();
}

void fullStripReact(CHSV defaultColor, CHSV changedColor)
{
  if (Serial.available() > 0) 
  {
    incomingByte = Serial.read();
    float alpha = incomingByte / 100.0f;
    if(alpha > bassResponse / 100)
    {
      for(int i = NUM_LEDS - 1; i >= 0;i--){
        leds[i] = changedColor;
      }
      
    }
    else 
    {
      for(int i = NUM_LEDS - 1; i >= 0;i--){
        leds[i] = defaultColor;
      }
    }
  }
  FastLED.show();
}

void lina(CHSV color)
{
  linaTimer--;
  if(linaTimer < 0)
  {
    linaTimer = linaTime;
    leds[NUM_LEDS / 2 - 1] = color;
    leds[NUM_LEDS / 2] = color;
    for(int i = 0; i < NUM_LEDS / 2;i++)
    {
      leds[i] = leds[i + 1];
      
    }
    for(int i = NUM_LEDS - 1; i > NUM_LEDS / 2 - 1; i--)
    {
      leds[i] = leds[i - 1];
    }
    FastLED.show();
  }
}
void fade()
{
    bool primaryLower;
    if(primaryColor.hue < secondaryColor.hue)
      primaryLower = true;
    else
      primaryLower = false;
      
    if((primaryColor.hue < rainbowColor.hue && secondaryColor.hue < rainbowColor.hue) || 
       (primaryColor.hue > rainbowColor.hue && secondaryColor.hue > rainbowColor.hue)   )
       rainbowColor.hue = primaryColor.hue;
    else 
    {
        if(primaryLower)
        {
          if(fadeIncreasing)
          {
            rainbowColor = slowlyChangeHue(0.1, primaryColor.hue, secondaryColor.hue, rainbowColor); 
            if(abs(secondaryColor.hue - rainbowColor.hue) * 10 < 200)
              fadeIncreasing = false;
          }
          else 
          {
            rainbowColor = slowlyChangeHue(0.1, secondaryColor.hue, primaryColor.hue, rainbowColor);
            if(abs(primaryColor.hue - rainbowColor.hue) * 10 < 200)
              fadeIncreasing = true;
          }
        }
        else
        {
          if(fadeIncreasing)
          {
            rainbowColor = slowlyChangeHue(0.1, secondaryColor.hue, primaryColor.hue, rainbowColor);
            if(abs(primaryColor.hue - rainbowColor.hue)< 2)
            {
              fadeIncreasing = false;
            }
          }
          else 
          {
            rainbowColor = slowlyChangeHue(0.1, primaryColor.hue, secondaryColor.hue, rainbowColor);
            if(abs(secondaryColor.hue - rainbowColor.hue)< 2)
            {
              fadeIncreasing = true;
            }
          }
        }
    }
    for(int i = NUM_LEDS - 1; i >= 0;i--)
    {
      leds[i] = rainbowColor;
    }
    FastLED.show();
}

void printToLCD()
{
    lcd.setCursor(0,0);
    if(menu == MAIN_MENU)
    {
        lcd.print("Main menu");
        lcd.setCursor(0,1);
        if(selected == 0)
        {
          lcd.print("Change preset");
        }
        else if(selected == 1)
        {
          lcd.print("Change colors");
        }
        else if(selected == 2)
        {
          lcd.print("Set bass response");
        }
        else if(selected == 3)
        {
          lcd.print("Set strobe speed");
        }
    }
    else if(menu == PRESET_MENU)
    {
      lcd.print("Change preset");
      lcd.setCursor(0,1);
      if(selected == PRESET_FULL_BASS)
      {
        lcd.print("Full strip bass");
      }
      else if(selected == PRESET_PIXEL_MOVE_BASS)
      {
        lcd.print("Pixel move bass");
      }
      else if(selected == PRESET_STROBE)
      {
        lcd.print("Strobe");
      }
      else if(selected == PRESET_LINA)
      {
        lcd.print("Lina");
      }
      else if(selected == PRESET_FADE)
      {
        lcd.print("Fade");
      }
    }
    else if(menu == COLOR_MENU)
    {
      lcd.print("Change colors");
      lcd.setCursor(0,1);
      if(selected == 0)
      {
        lcd.print("Primary color");
      }
      else if(selected == 1)
      {
        lcd.print("Secondary color");
      }
    }
    else if(menu == BASS_MENU)
    {
      lcd.print("Bass response");
      lcd.setCursor(0,1);
      lcd.print(selected);
    }
    else if(menu == STROBE_MENU)
    {
      lcd.print("Strobe delay");
      lcd.setCursor(0,1);
      lcd.print(selected);
    }
    else if(menu == PRIMARY_HUE)
    {
      lcd.print("Pr. color hue");
      lcd.setCursor(0,1);
      lcd.print(selected);
    }
    else if(menu == SECONDARY_HUE)
    {
      lcd.print("Sec. color hue");
      lcd.setCursor(0,1);
      lcd.print(selected);
    }

    else if(menu == PRIMARY_BRIGHTNESS)
    {
      lcd.print("Pr. color bright");
      lcd.setCursor(0,1);
      lcd.print(selected);
    }
    else if(menu == SECONDARY_BRIGHTNESS)
    {
      lcd.print("Sec. color bright");
      lcd.setCursor(0,1);
      lcd.print(selected);
    }
}

void reactToInputLCD()
{
  if(buttonTimer < 1)
  {
    switch (lcd_key)               // depending on which button was pushed, we perform an action  
    {    
       case btnSELECT:    
       {        
         if(menu == MAIN_MENU || menu == COLOR_MENU) //If in main or color menu
         {
            minOption = 0;
            if(menu == MAIN_MENU)           //If in main menu
            {
              if (selected == 0)    //if preset menu was selected
              {
                menu = PRESET_MENU;
                maxOption = 4;
                selected = 0;
              }
              else if (selected == 1) //if color menu was selected
              {
                menu = COLOR_MENU;
                maxOption = 1;
                selected = 0;
              }
              else if(selected == 2)  //if bass response menu was selected
              {
                menu = BASS_MENU;
                selected = bassResponse;
                maxOption = 100;
              }
              else if(selected == 3)  //if strobe delay menu was selected
              {
                menu = STROBE_MENU;
                selected = strobeDelay;
                minOption = 3;    //Basically, the blinking is not perceivable below this
                maxOption = 100;
              }
              
            }
            else if(menu == COLOR_MENU)     //If in color menu
            {
              maxOption = 255;
              if(selected == 0)
              {
                menu = PRIMARY_HUE;
                selected = primaryHue;
              }
              else if(selected == 1)
              {
                menu = SECONDARY_HUE;
                selected = secondaryHue;
              }
            }
         }
         
         else if(menu == PRESET_MENU)      //If in preset menu
         {
            preset = selected;   
            /*
            minOption = 0;
            maxOption = 3;
            menu = MAIN_MENU;
            selected = 0;
            */
         }

         else if(menu == BASS_MENU)      //If in bass response menu
         {
            bassResponse = selected;    
            /*
            minOption = 0;
            maxOption = 3;
            menu = MAIN_MENU;
            selected = 0;
            */
         }
         else if(menu == STROBE_MENU)      //If in strobe delay menu
         {
            strobeDelay = selected;  
            /*  
            minOption = 0;
            maxOption = 3;
            menu = MAIN_MENU;
            selected = 0;
            */
         }
         
         else if(menu == PRIMARY_HUE || menu == SECONDARY_HUE)  //If in primary hue or secondary hue menu
         {
            if(menu == PRIMARY_HUE)                   //If in primary hue menu
            {
              menu = PRIMARY_BRIGHTNESS;
              minOption = MINIMUM_BRIGHTNESS;
              maxOption = 255;
              primaryHue = selected;
              selected = primaryBrightness;
            }
            else if(menu == SECONDARY_HUE)              //If in secondary hue menu
            {
              menu = SECONDARY_BRIGHTNESS;
              minOption = MINIMUM_BRIGHTNESS;
              maxOption = 255;
              secondaryHue = selected;
              selected = secondaryBrightness;
            }
         }
         
         else if(menu == PRIMARY_BRIGHTNESS || menu == SECONDARY_BRIGHTNESS) //If in primary brightness or secondary brightness menu
         {
            if(menu == PRIMARY_BRIGHTNESS)                   //If in primary brightness menu
            {
              primaryBrightness = selected;
            }
            else if(menu == SECONDARY_BRIGHTNESS)              //If in secondary brightness menu
            {
              secondaryBrightness = selected;
            }
            minOption = 0;
            maxOption = 1;
            selected = 0;
            menu = COLOR_MENU;
         }
         
         lcd.clear();
         printToLCD();
         buttonTimer = buttonTime;
         break; 
         
       } 
       case btnBACK:   
       {     
         minOption = 0; //The minimum possible option is always 0 when going back
         if(menu == PRESET_MENU || menu == COLOR_MENU || menu == BASS_MENU || menu == STROBE_MENU)
         {
            menu = MAIN_MENU;
            selected = 0;
            maxOption = 3;
         }
         else if (menu == PRIMARY_HUE || menu == SECONDARY_HUE)
         {
            menu = COLOR_MENU;
            selected = 0;
            maxOption = 1;
         }
         else if (menu == PRIMARY_BRIGHTNESS)
         {
            selected = primaryHue;
            menu = PRIMARY_HUE;
            maxOption = 255;
         }
         else if (menu == SECONDARY_BRIGHTNESS)
         {
            selected = secondaryHue;
            menu = SECONDARY_HUE;
            maxOption = 255;
         }
         lcd.clear();
         printToLCD();
         buttonTimer = buttonTime;
         break;     
       } 
       
       case btnNONE:   
       {     
         break;     
       } 
    } 
  }
}

void Potentiometer()
{  
    if(digitalRead(Encoder_B))
    {
     selected++;
     if(selected > maxOption)
     {
        selected = minOption;
     }
    }
    else
    {  
      selected--;
      if(selected < minOption)
      {
         selected = maxOption;
      }
    }     
    updateScreen = 1;
}

int read_LCD_buttons()
{
  adc_key_in = analogRead(0);

  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 450)  return btnBACK;
  if (adc_key_in < 850)  return btnSELECT;

  return btnNONE;
}
                                        //160       0
CHSV slowlyChangeHue(float changeSpeed, int startValue, int endValue, CHSV color)
{
  if(endValue < startValue)
  {
    hue = hue - changeSpeed;
    if(hue > startValue || hue < endValue)
      hue = endValue;
  }
  else 
  {
    hue = hue + changeSpeed;
    if(hue < startValue || hue > endValue)
      hue = startValue;
  }
  color.hue = hue;
  return color;
}
