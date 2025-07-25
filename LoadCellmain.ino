/**********************************************************************************
**                             HPi Source File                                   **
**    Copyright (C) 2020-2024 HPiStudio. Allrights Reserved.                     **
** ********************************************************************************
** this code is part of HPiLabScale                                              **
** Description:                                                                  **
**                                                                               **
**                                                                               **
** Created in fri 1403/07/07 12:40 AM By Hosein Pirani                           **
**                                                                               **
** Modified In tue 1403/07/10 16:25  To  21:30 by me.                            **
** :                         Major Fixes   .                                     **
** TODO: Test All Methods.                                                       **
** TODO: , implement Power Off                                                   **
** ..                                                                            **
** ...                                                                           **
** ....TODO: CODE                                                                **
** .....TODO: More Code                                                          **
** ........TODO: Code                                                            **
** ...........TODO:  #_#                                                         **
** ...............                                                               **
*********************************************************************************/

//#define DEBUG
#include <Arduino.h>
#include "HX711.h"
#include <HT1621.h>

//#include <avr/wdt.h>
//HT161
constexpr auto LCD_CS_PIN = 3;
constexpr auto LCD_WR_PIN = 1;
constexpr auto LCD_DATA_PIN = 0;

HT1621 lcd;

int addr=1;

// HX711 circuit wiring
constexpr auto LOADCELL_DOUT_PIN = 16;
constexpr auto LOADCELL_SCK_PIN = 15;
HX711 scale;

constexpr auto DefaultCalibrationFactor = 1679.25f; // default scale factor. for 128 gain use :3358.5f

constexpr auto MaxAllowedWheightONCES = 18.0f;
constexpr auto MaxAllowedWheightGr    = 510.2914f;

    // scale readings and filters

    double reading = 0.0f;
double prevreading = 0.0f;
const double oncedivide = 0.035274f; // for division use 28.34949254;

//Timing
unsigned long currentMillis = 0;
unsigned long prevMillis = 0;
unsigned long interval = 5;

//PowerOff Timer
unsigned long TurnOffTimer = 0;
unsigned long TurnOffTimeOut = 300000; // 5 minutes.

//power off button (hold) timer.

unsigned long buttonPressTime = 0;
unsigned long OffTimeOut = 1000;
bool buttonHeld = false;

//Buttons
constexpr auto UnitPin = 10;
constexpr auto Power_and_TarePin = 11;

//Buzzer
constexpr auto BuzzerPin = 9;

//flags
bool OZ = true;
bool minusDrawed = false; // flag for clear screen

bool poweron = false;
bool Firstinit = false;

bool PowerFromButton = false;

int _buffer[10] =
    {
        // 191,//0.
        190, // 0
             // 161,//1.
        160, // 1
             // 125,//2.
        124, // 2
             // 95,//3.
        94,  // 3
             // 199,//4.
        198, // 4
             // 219,//5.
        218, // 5
             // 251,//6.
        250, // 6
             // 143,//7.
        142, // 7
             // 255,//8.
        254, // 8
             // 223,//9.
        222, // 9

};

int _Decimalbuffer[10] = // numbers include point.
    {
        191, // 0.
        // 190, // 0
        161, // 1.
        // 160, // 1
        125, // 2.
        // 124, // 2
        95, // 3.
        // 94, // 3
        199, // 4.
        // 198, // 4
        219, // 5.
        // 218, // 5
        251, // 6.
        // 250, // 6
        143, // 7.
        // 142, // 7
        255, // 8.
        //  254, // 8
        223, // 9.
        // 222, // 9

};

int _Signbuffer[3] =
    {
        1, // gr (Right column)
        8, // OZ(Right column) --1oz = 28.349g
        2, // - for column 1 (left)
};

int _digitbuffer[19] =
    {
        248, // E[0]
        238, // A[1]
        236, // P
        230, // H
        114, // o -5[4]
        96,  // r
        112, // c
        118, // d[7]
        136, // Upper r
        174, // N
        96,  // n
        176, // L[11]
        182, // U[12]
        184, // C
        186, // G[14]
        214, // y
        226, // h
        232, // F
        242, // b
};

void setup() {

//Pins
pinMode(UnitPin,INPUT);
pinMode(Power_and_TarePin,INPUT);

pinMode(BuzzerPin,OUTPUT);

//ISRs
attachInterrupt(digitalPinToInterrupt(Power_and_TarePin), checkPowerKey, RISING);
attachInterrupt(digitalPinToInterrupt(UnitPin), checkUnit, RISING);

  //Serial.begin(9600);
 
 delay(10);

 scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
 delay(10); 
 scale.set_gain(64);
 delay(10);
 scale.set_scale(DefaultCalibrationFactor); // this value is obtained by calibrating the scale with known weight.
 delay(10);
 Tare();

 lcd.noDisplay();
 //scale.power_down();
 SetScaleUnit();

 // #endif


}

//-----------------------------------------------------------//

//-----------------------------------------------------------//
void loop() {
  currentMillis = millis();
  //TurnOffTimer  = millis();

if (poweron)
{
  if (PowerFromButton == true && Firstinit == true)
  {
    PowerFromButton = false;
    digitalWrite(BuzzerPin, HIGH);
    delay(200);
    digitalWrite(BuzzerPin, LOW);
    DrawSplash();
    Tare();
  }

 if (Firstinit == false)
     {
     Firstinit = true;
     lcd.begin(LCD_CS_PIN,LCD_WR_PIN,LCD_DATA_PIN);

    lcd.display();
    lcd.clear();

      digitalWrite(BuzzerPin, HIGH);
     delay(200);
      digitalWrite(BuzzerPin, LOW);

      scale.power_up();

      DrawSplash();

      lcd.clear();
      Tare();
      // reset the scale to 0
      delay(100);
    }
  
  // wdt_reset();//reset watchdog
 
  if(currentMillis - prevMillis >= interval)
  {
    prevMillis = currentMillis;
    

   if (scale.wait_ready_timeout(500)) 
   {
   // lcd.clear();
      //reading = round(scale.get_units(),);
     reading = scale.get_units(12);

          if (reading != prevreading)
     {
       //lcd.clear();
       prevreading = reading;

       //Serial.print("Weight: ");
       // Serial.println(reading);
       // if (reading != lastReading)
       // {//
       if (OZ == true)
       {
         Draw(reading * oncedivide);
       }else{
         Draw(reading);
            }
       TurnOffTimer = currentMillis;
     }
       //Reset Timer
       
     //}
      //lastReading = reading;
   }else {
     lcd.clear();
     lcd.wrone(1,_digitbuffer[11]);//Draw L character.
     lcd.wrone(3, _digitbuffer[4]); // Draw o character.
     lcd.wrone(5, _digitbuffer[1]); // Draw A character.
     lcd.wrone(7, _digitbuffer[7]); // Draw d character.
    }

   if(currentMillis - TurnOffTimer >= TurnOffTimeOut )
    {
     //lcd.clear();
     DoSleep();

    }

        
  }

  //button handling

 if (digitalRead(UnitPin) == HIGH)
 {
   if (buttonHeld == false)
   {
    // Serial.println("not held");
     buttonPressTime = millis();
     buttonHeld = true;
   }
   if (millis() - buttonPressTime >= OffTimeOut && buttonHeld == true)
   {
     buttonPressTime = millis();
     //Serial.println("HOLD");
     DoSleep();
     buttonHeld = false;
     // buttonPressTime = millis();
   }
 }
 else
 {
   buttonHeld = false;
   buttonPressTime = millis();
 }

  if (OZ == true)
    {//onces
    lcd.wrone(10,_Signbuffer[1]);

    }else{//grams
    lcd.wrone(10,_Signbuffer[0]);
    }
//#endif
//
}
checkPowerKey();

}//Loop

//-----------------------------------------------------------//

//-----------------------------------------------------------//

void checkPowerKey()
{
  if(digitalRead(Power_and_TarePin) == HIGH)
  {
    digitalWrite(BuzzerPin, HIGH);
    delay(300);
    digitalWrite(BuzzerPin, LOW);
    if (poweron == true)
    {

      //noSleep();

      lcd.display();
      lcd.clear();

      scale.power_up();

      

      delay(3);
      // SetScaleUnit();
      }else
          {
            //delay()
            poweron = true;
            //noSleep();

            lcd.display();
            lcd.clear();

            scale.power_up();
            // scale.tare();
          }
          Tare();
  }
}

//-----------------------------------------------------------//

//-----------------------------------------------------------//

void checkUnit()
{
  if (digitalRead(UnitPin) == HIGH)
  {
    if(poweron)
    {
     SetScaleUnit();
    }
  }
}

//-----------------------------------------------------------//

//-----------------------------------------------------------//

void Draw(double number)
{

  //clear lcd
  for (long i = 1; i <= 5; i++)
  {
    // Serial.println(i);
    // delay(20);

    lcd.wrone(addr, 0); // 0 = 0x00 --> 0b00000000 all segments will be cleared.
    addr += 2;
    if (addr > 9)
    {
      addr = 1;
      break;
    }
    
  }

  int intArray[3] = {0};
  int divisionArray[3] = {0};
  int index = 0;
  int indexdiv = 0;
  // char str[7] = {0};
  bool IsNegativeNumber = (number < 0);
  bool flag = false;
  int stringlengh = 0;
  int beforedivlengh = 0;
  int AfterDivlengh = 0;
  String stlen = "";

  if (number != 0)
  {
    minusDrawed = IsNegativeNumber;

    if ((OZ != true && number > MaxAllowedWheightGr) || (OZ == true && number > MaxAllowedWheightONCES)) // Max 300 grams ~ 10 oNCES.
    {
      //Serial.println("Error!");
      DrawError();
      return;
    };

    stlen = String(number, 2);
    if (IsNegativeNumber) // remove minus from string.
    {
      stlen.remove(0, 1);
      //Serial.print("after removing minus : ");
      ///Serial.println(stlen);
    }
    stringlengh = stlen.length() - 1;
    //lcd.clear();

    // dtostrf(number, 1, 3, str);

    // stringlengh = strlen(str);
    //Serial.print("stringlengh : ");
    //Serial.println(stringlengh);
    for (int i = 0; i <= stringlengh; i++)
    {
      // if (str[i] != '.' && str[i] != '-') // BeforeDivision.
      if (stlen.charAt(i) != '.') // BeforeDivision. && stlen.charAt(i) != '-'
      {
        if (flag == false)
        {
          //Serial.print("Before substr (stlen[i]) : ");
         // delay(20);
          //Serial.println(stlen.substring(i, i + 1));
          //delay(20);
          //Serial.print("Before index : ");
          //delay(20);
          //Serial.print(index);
          //delay(20);

          // intArray[index] = str[i] - '0';
          intArray[index] = stlen.substring(i, i + 1).toInt();
          //Serial.print("  result : ");
          //Serial.println(intArray[index]);
          index++;
          beforedivlengh++;
        }
        else
        {
          //Serial.print("after : ");
       //   delay(20);
          //Serial.println(stlen.substring(i, i + 1));

          //delay(20);
          //Serial.print("after index : ");
          //delay(20);
          //Serial.print(indexdiv);
          //delay(20);

          // divisionArray[indexdiv] = str[i] - '0';
          divisionArray[indexdiv] = stlen.substring(i, i + 1).toInt();
          //Serial.print("  result : ");
          //Serial.println(divisionArray[indexdiv]);
          indexdiv++;
          AfterDivlengh++;
        }
      }
      else
      { // AfterDivision.

        flag = true;
        //Serial.println("flag");
        //delay(50);
      }
    }

    ////--------------------------------------------------------------------///

    // print first 3 digits include "." .
    for (int i = 0; i < beforedivlengh; i++)
    {
      if (IsNegativeNumber)
      {
        lcd.wrone(-1, 2); // negative sign 2
      }
      else
      {
        minusDrawed = false;
        lcd.wrone(-1, 0);
      } // negative sign

      //Serial.print("Before Division intArray : ");
      //delay(20);
      //Serial.print(intArray[i]);
      //Serial.print("   index(i) : ");
      //Serial.println(i);
      //delay(20);
     // Serial.print("  before printing number  : ");
     // Serial.print(i);
    //  Serial.print("  lcd char  : ");
      //Serial.println(_buffer[intArray[i]]);

      if (beforedivlengh - i == 1) // Last number before decimal point
      {
        //Serial.print("  Decimal is : ");
        //Serial.println(_Decimalbuffer[intArray[i]]);
        // Last number before decimal point
        lcd.wrone(addr, _Decimalbuffer[intArray[i]]);
      }
      else
      { // print normal numbers
    //    Serial.print("  not last . ");
    //    Serial.println(_buffer[intArray[i]]);
        lcd.wrone(addr, _buffer[intArray[i]]);
      }

      addr += 2;
      if (addr > 9) // reset lcd cursor
        addr = 1;
    }
    ////--------------------------------------------------------------------///
    for (int i = 0; i < AfterDivlengh; i++)
    {
      if (IsNegativeNumber)
      {
        lcd.wrone(-1, 2); // negative sign2
      }
      else
      {
        minusDrawed = false;
      }
      //Serial.print("After Division : ");
      //delay(20);
    //  Serial.print(divisionArray[i]);
    //  Serial.print("   index : ");
     // Serial.println(i);
      //delay(20);
     // Serial.print("  before printing number  : ");
     // Serial.print(i);
     // Serial.print("  lcd char  : ");
     // Serial.println(_buffer[divisionArray[i]]);

      lcd.wrone(addr, _buffer[divisionArray[i]]);

      if (number >= 100.0f &&  addr == 7)
      {
        if (addr > 8) // reset cursor.
          addr = 1;
      }else
      {
      addr += 2;
      
      if (addr > 9) // reset cursor.
        addr = 1;
        }
    }

    //  delay(200);
  }
  else
  { // number is zero
    lcd.clear();
    //Serial.println("Zero...");
    if (minusDrawed)
    {
      lcd.clear();
      minusDrawed = false;
    }

    for (long i = 1; i <= 3 ; i++)
    {
      // Serial.println(i);
      //delay(20);

      lcd.wrone(addr, _buffer[0]);
      addr += 2;
      if (addr > 5)
      {
        addr = 1;
        break;
      }
    }
  }
  addr = 1; // reset cursor
}

//-----------------------------------------------------------//

//-----------------------------------------------------------//

void DrawError()
{
  lcd.clear();
  lcd.wrone(1, _digitbuffer[0]); // E
  lcd.wrone(3, _digitbuffer[5]); // r
  lcd.wrone(5, _digitbuffer[5]); // r
  lcd.wrone(7, _digitbuffer[4]); // o
  lcd.wrone(9, _digitbuffer[5]); // r
}

//-----------------------------------------------------------//
//09336037823 service tell no.
//-----------------------------------------------------------//

void DrawSplash()
{
  

  lcd.wrone(1, _buffer[0]);      // 0
  lcd.wrone(3, _buffer[9]);      //
  lcd.wrone(5, _buffer[3]);      //
  lcd.wrone(7, _buffer[3]);      //

  delay(1000);
  lcd.clear();

  lcd.wrone(1, _buffer[6]);      //
  lcd.wrone(3, _buffer[0]);      //
  lcd.wrone(5, _buffer[3]);      //

  delay(1000);
  lcd.clear();

  lcd.wrone(1, _buffer[7]);      //
  lcd.wrone(3, _buffer[8]);      //
  lcd.wrone(5, _buffer[2]);      //
  lcd.wrone(7, _buffer[3]);      //
  delay(700);
  lcd.clear();
}

//-----------------------------------------------------------//

//-----------------------------------------------------------//
void SetScaleUnit()
{
  
if (OZ == true)
    {//onces
      digitalWrite(BuzzerPin, HIGH);
      OZ = false;
     // lcd.clear();
      delay(30);
     // lcd.wrone(10, _Signbuffer[0]);
      digitalWrite(BuzzerPin, LOW);
    }else{//grams
      digitalWrite(BuzzerPin, HIGH);
      OZ = true;
     // lcd.clear();
      delay(30);
     // lcd.wrone(10, _Signbuffer[1]);
      digitalWrite(BuzzerPin, LOW);
    }
    Tare();
    digitalWrite(BuzzerPin, LOW);
    return;
}

//-----------------------------------------------------------//

//-----------------------------------------------------------//
/*
void calibrate()
{
  if (scale.is_ready())
  {
    scale.set_scale();
    //Serial.println("Tare... remove any weights from the scale.");

    //lcd.clear();
   //` lcd.wrone(1, _digitbuffer[0]); // E
    lcd.wrone(1, _digitbuffer[9]); // N
    lcd.wrone(3, _digitbuffer[4]); // o
    lcd.wrone(5, _digitbuffer[11]); // L
    lcd.wrone(7, _digitbuffer[4]); // o
    lcd.wrone(9, _digitbuffer[1]); // A

    delay(5000);
    scale.tare();

   // Serial.println("Tare done...");
   // Serial.print("Place a known weight on the scale...");

    lcd.wrone(3, _digitbuffer[14]);  // G
    lcd.wrone(5, _digitbuffer[4]);  // o

    delay(5000);

    double reading = scale.get_units(10);
    CalibrationFactor = reading / CalibrationWeight;

    //Store data in EEProm.
    EEPROM.put(eep_Addr_CalibrationFact, CalibrationFactor);

    scale.set_scale(CalibrationFactor);
    
    lcd.wrone(1, _digitbuffer[7]); // d
    lcd.wrone(3, _digitbuffer[4]);  // o
    lcd.wrone(5, _digitbuffer[9]); // n
    lcd.wrone(7, _digitbuffer[0]);  // E

    delay(300);
    digitalWrite(BuzzerPin,HIGH);
    delay(50);
    digitalWrite(BuzzerPin,LOW);
    //lcd.clear();
    
   //Serial.print("Result: ");
    //Serial.println(reading);
  }
  else
  {
    DrawError();
  }
  //delay(1000);
}
*/

void DoSleep()

{
  digitalWrite(BuzzerPin,HIGH);
  delay(100);
  digitalWrite(BuzzerPin,LOW);

  lcd.noDisplay();

  //scale.power_down();

  poweron = false;
  PowerFromButton = true;

  //sleepMode(SLEEP_IDLE);
  //sleep(); // Go to sleep
  //noSleep();
  //lcd.display();
  //scale.power_up();
}

 //-----------------------------------------------------------//

 //-----------------------------------------------------------//
 void Tare()
 {

  scale.tare();

 }