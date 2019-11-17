#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <GyverEncoder.h>

// Пункты меню:
// Дата и время (вывод)                                 id = 1
// Дата и время (установка)                             id = 2
// Время вклчения света и скорость полного включения    id = 3
// Время выключения света и скорость полного выключения id = 4

// lcd.setCursor(col, row);

typedef unsigned char   U8;
typedef unsigned int    U16;
typedef unsigned short  UShort;

#define RELAY_LED       0
#define RELAY_UV_LAMP   0
#define PWM_LED         0

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

RTC_DS3231 rtc;
char daysOfTheWeek[7][12]   = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
U8 daysOnMonth[12]          = {31, 28, 31, 30, 31, 30, 31, 31, 30 , 31, 30, 31};

DateTime now;

Encoder enc(2, 3, 4);

enum menuItem
{
  showTimeDate = 0,
  editSunrise,
  editSundown,
  systemMenu,
};

enum typeValue
{
  timeValue = 0,
  dayValue,
  monthValue,
  yearValue, 
  spacerValue,
  okCancelValue,
  offOnValue
};

enum confirm
{
    ok = 0,
    cancel
};

enum state
{
    on = 0,
    off
};

U8 menuId               = showTimeDate;     // Standart start menu
U8 flagClear            = 1;
U8 stringPosition       = 0;                // Position cursor editing fiedl
U8 inMenu               = 0;                // In menu for editing
U8 inTimeEditMenu       = 0;
U8 isBlink              = 1;                // Blink editing value
U8 isEdit               = 0;
U8 cursorPosition       = 1;
U8 confirmValue         = cancel;
U8 sunriseStateValue    = off;
U8 sundownStateValue    = off;
U8 editTimeDate         = off;
U8 stepSundown;

unsigned long timer_1 = millis();
unsigned long timer_2 = millis();
unsigned long sunriseTimer;

// Set time and day of week
static U8 setHours;
static U8 setMinutes;
static U8 setDayOfWeek;
static U8 setDay;
static U8 setMonth;
static U16 setYear;

// Set time and spees of sunrise
U8 sunRiseHours      = 0;
U8 sunRiseMinutes    = 0;
U8 sunRiseSpeed      = 0;
U8 stepSunrise       = 0;
U8 sunriseStartFlag  = 0;

// Set time and spees of sundown
U8 sunDownHours;
U8 sunDownMinutes;
U8 sunDownSpeed;

U16 i;   // Counter of sunrise;

void lcdPrintTempAndDay();
void lcdPrintDate();
void lcdBlink(U16 data, U8 typeValue, U8 sizeCursor);
void sunrise();
void sundown();
void showNowTime();
void showEditTime();
void editNowTime();
void editSundown();
void editSunrise();
void systemOutput();

void setup()
{
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    enc.setType(TYPE1);

    pinMode(5, OUTPUT);
    analogWrite(5, 0);
    
    setHours    = rtc.now().hour(); 
    setMinutes  = rtc.now().minute();
    setDay      = rtc.now().day();
    setMonth    = rtc.now().month();
    setYear     = rtc.now().year();

}
void loop()
{
    now = rtc.now();
    enc.tick();

  if (enc.isHolded())
  {
    lcd.clear();
    if (inMenu == 0 && menuId == showTimeDate && editTimeDate == off)
    {
        editTimeDate = on;
    }

    if(menuId == showTimeDate && inMenu == 1 && editTimeDate = on)
    {

    }

    if(inMenu == 1 && menuId == editTimeDate && confirmValue == ok)
    {
        rtc.adjust(DateTime(setYear, setMonth, setDay, setHours, setMinutes));
        menuId = showTimeDate;
    }

    if(inMenu == 1 && menuId == editSunrise && confirmValue == ok)
    {
        if (sunriseStateValue == on)
        {
            stepSunrise = (sunRiseSpeed * 60) / 255;  // Каждые stepSunrise секунд увеличиваем значение analogWrite на 1;
        }
        menuId = showTimeDate;
    }

     if(inMenu == 1 && menuId == editSunrise && confirmValue == on)
    {
        if (sunriseStateValue == off)
        {
            i = 0;
            sunriseStartFlag = 0;
            analogWrite(5, 0);
        }
        menuId = showTimeDate;
    }
    
    cursorPosition = 1;
    inMenu = !inMenu; 
  }

  if (enc.isClick())
  {
    if (cursorPosition < 6)
    { 
        //lcd.clear();
        ++cursorPosition;
    }
    else
    {
      //lcd.clear();
      cursorPosition = 1;
    }
  }

  if (enc.isRight())
  {
    if (!inMenu)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      if (menuId < systemMenu)
      {
        ++menuId;
      }
      else
      {
        menuId = showTimeDate;
      }
    }
    else
    {   lcd.clear();
    
        if(cursorPosition == 1 && menuId == editTimeDate)
        {
          if (setHours < 23)
            setHours ++;
          else
            setHours = 0;
        }

        else if(cursorPosition == 1 && menuId == editSunrise)
        {
          if (sunRiseHours < 23)
            sunRiseHours ++;
          else
            sunRiseHours = 0;
        }
    
        else if(cursorPosition == 2 && menuId == editTimeDate)
        {
          if (setMinutes < 59)
            setMinutes ++;
          else
            setMinutes = 0;
        }

        else if(cursorPosition == 2 && menuId == editSunrise)
        {
          if (sunRiseMinutes < 59)
            sunRiseMinutes ++;
          else
            sunRiseMinutes = 0;
        }
        
        else if(cursorPosition == 3 && menuId == editTimeDate)
        {
            if(setDay < daysOnMonth[setMonth])
            {
                ++setDay;
            }
            else
            {
                setDay = 1;
            }
        }

        else if(cursorPosition == 3 && menuId == editSunrise)
        {
            sunRiseSpeed ++;
        }

        else if(cursorPosition == 4 && menuId == editTimeDate)
        {
            if(setMonth < 12)
            {
                ++setMonth;    
            }
            else
            {
                setMonth = 1;
            }
        }

        else if(cursorPosition == 4 && menuId == editSunrise)
        {
            sunriseStateValue = !sunriseStateValue;
        }

        else if(cursorPosition == 5 && menuId == editTimeDate)
        {
                ++setYear;
        }

        else if(cursorPosition == 5 && menuId == editSunrise)
        {
            confirmValue = !confirmValue;
        }
        
        else if(cursorPosition == 6 && menuId == editTimeDate)
        {
                confirmValue = !confirmValue;
        }
 
        isBlink = 0;
        timer_2 = millis() - 1000;
    }
  }

  if (enc.isLeft())
  {
    if (!inMenu)
    {
      if (menuId > showTimeDate)
      {
        --menuId;
      }
      else
      {
        menuId = systemMenu;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
    }
    
    else
    {
        if (cursorPosition == 1 && menuId == editTimeDate)
        {
          if (setHours > 0 )
          {
            --setHours;
          }
          else
          {
            setHours = 23;
          }
        }

        else  if (cursorPosition == 1 && menuId == editSunrise)
        {
          if (sunRiseHours > 0 )
          {
            --sunRiseHours;
          }
          else
          {
            sunRiseHours = 23;
          }
        }

        else if (cursorPosition == 2 && menuId == editTimeDate)
        {
          if (setMinutes > 0)
          {
            --setMinutes;
          }
          else
          {
            setMinutes = 59;
          }
        }

        else if (cursorPosition == 2 && menuId == editSunrise)
        {
          if (sunRiseMinutes > 0)
          {
            --sunRiseMinutes;
          }
          else
          {
            sunRiseMinutes = 59;
          }
        }

        else if (cursorPosition == 3 && menuId == editTimeDate)    
        {
            if(setYear < 1970)
            {
                setYear = 2100;    
            }
            else
            {
                --setYear;
            }
        }

         else if (cursorPosition == 3 && menuId == editSunrise)    
        {
           if(sunRiseSpeed > 0)
           {
                --sunRiseSpeed;
           }

        }

        if (cursorPosition == 4 && menuId == editTimeDate)    
        {
            if(setMonth < 1)
            {
                setMonth = 12;    
            }
            else
            {
                --setMonth;
            }
        }

         if (cursorPosition == 4 && menuId == editTimeDate)    
        {
            if(setMonth < 1)
            {
                setMonth = 12;    
            }
            else
            {
                --setMonth;
            }
        }
        
         if (cursorPosition == 5 && menuId == editTimeDate)               // Добавить проверку на високосный год
        {
            if(setDay < 1)
            {
                setDay = daysOnMonth[setMonth - 1];
            }
            else
            {
                --setDay;
            }
        }
        if(cursorPosition == 6 && menuId == editTimeDate)
        {
            confirmValue = !confirmValue;
        }
    }
  }

    if(sunriseStateValue == on && i <= 255 && sunriseStartFlag)
    {
        if((millis() - sunriseTimer) > (stepSunrise * 1000))
        {
            ++i;
            analogWrite(5, i);
            sunriseTimer = millis();
        }
    }
    if(i >= 255)
    {
        analogWrite(5, 255);
    }

    if( (now.hour() == sunRiseHours) && (now.minute() == sunRiseMinutes) && (!sunriseStartFlag) && (sunriseStateValue == on))
    {
        sunriseStartFlag = 1;
        i = 0;
        sunriseTimer = millis();
    }
    
  switch (menuId)
  {
    case showTimeDate: // Show date and time
    {
        if(!inMenu)
        {
          showNowTime();
        }
        else if(editTimeDate == on && inTimeEditMenu == 0)
        {
          showEditTime();
        }
        else if(editTimeDate == on && inTimeEditMenu == 1)
        {
          editNowTime();       
        }
        break;
    }
    case editTimeDate: // Set date and time
    {
        if (!inMenu) // Show current time and day of week
        {
          lcd.setCursor(0, 0);
          if (now.hour() < 10)
          {
            lcd.print("0");
          }
          lcd.print(now.hour(), DEC);

          lcdBlink(0, spacerValue, 1);
          
          lcd.setCursor(3, 0);
          if (now.minute() < 10)
          {
            lcd.print("0");
          }
          lcd.print(now.minute(), DEC);

          lcdPrintDate();         
        }
        else
        {
          
        }
        break;
    }
    case editSunrise:
    {
        editSunrise();
        break;
    }
    case editSundown:
    {
        editSundown();
        break;
    }
    case systemMenu:
    {
        systemOutput();
        break;
    }
    default:
    {
        lcd.home();
        lcd.print("in default");
        break;
    }
  }
}

void lcdPrintTempAndDay()
{
  lcd.setCursor(0, 1);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" T: ");
  lcd.print((int)rtc.getTemperature());
  lcd.print(" C");
}

void lcdPrintDate()
{
    lcd.setCursor(0, 1);
    if(setDay < 10)
    {
        lcd.print("0");
    }
    lcd.print(setDay);

    lcd.print(".");
    
    if(setMonth < 10)
    {
        lcd.print("0");
    }
    lcd.print(setMonth);

    lcd.print(".");

    lcd.print(setYear);
}

void lcdBlink(U16 data, U8 typeValue, U8 sizeCursor)
{
    if ((millis() - timer_2) > 500)
    {  
        if (isBlink)        // Если данные видны на дисплее, то скрываем их
        {
            for (U8 i = 0; i < sizeCursor; ++i)
            {
                lcd.print(" ");
            }
            isBlink = 0;
        }
    else 
    {
        if (typeValue == timeValue && data < 10)
        {
            lcd.print("0");
            lcd.print(data);
        }
        else if (typeValue == timeValue && data >= 10)
        {
            lcd.print(data);
        }
        else if(typeValue == spacerValue)
        {
            lcd.print(":");
        }

        else if(typeValue == okCancelValue)
        {
            lcd.setCursor(10, 0);
            if(data == ok)
            {
                lcd.print("    Ok");
            }
            else
            {
                lcd.print("Cancel");
            }
        }  
                 
        else if(typeValue == offOnValue)
        {
            lcd.setCursor(7, 1);
            if(data == on)
            {
                lcd.print(" ON");   
            }
            else
            {
                lcd.print("OFF");
            }
        }
        else if(typeValue == dayValue)
        {
            if(data < 10)
            {
                lcd.print("0");
            }
            lcd.print(data);
        }
        else if(typeValue == monthValue)
        {
            if(data < 10)
            {
                lcd.print("0");
            }
            lcd.print(data);
        }
        else if(typeValue == yearValue)
        {
            lcd.print(data);
        
        }
        isBlink = 1;
    }
    timer_2 = millis();
  }
}

void showNowTime()
{
    lcd.setCursor(0, 0);
    if (now.hour() < 10)
    {
        lcd.print("0");
    }
    lcd.print(now.hour(), DEC);
    
    lcd.print(":");
    
    if (now.minute() < 10)
    {
        lcd.print("0");
    }
    lcd.print(now.minute(), DEC);
    
    lcd.print(":");
    
    if (now.second() < 10)
    {
        lcd.print("0");
    }
    else
    {
        lcd.setCursor(6, 0);
    }
    
    lcd.print(now.second(), DEC);
    
    lcdPrintTempAndDay();
    timer_1 = millis();
    flagClear = 0;
}

void editSunrise()
{
    if(!inMenu)
    {
        lcd.home();
        lcd.print("Sunrise");
    
        lcd.setCursor(0, 1);
        if(sunRiseHours < 10)
        {
            lcd.print("0");
        }
        lcd.print(sunRiseHours);
        lcd.print(":");
        if(sunRiseMinutes < 10)
        {
            lcd.print("0");
        }
        lcd.print(sunRiseMinutes);
        
        lcd.setCursor(7, 1);
        if(sunriseStateValue == off)
        {
            lcd.print("OFF");
        }
        else
        {
            lcd.print("ON");
        }
        
        lcd.setCursor(13, 1);
        if(sunRiseSpeed < 10)
        {
            lcd.print("0");
        }
        lcd.print(sunRiseSpeed);
    }
    else
    {
        if (cursorPosition == 1)    // Set start hours
        {
            lcd.home();
            lcd.print("Sunrise");
            lcd.setCursor(0, 1);
            lcdBlink(sunRiseHours, timeValue, 2);
            
            lcd.setCursor(2, 1);
            lcd.print(":");
            
            if (setMinutes < 10)
            {
                lcd.print("0");
            }
            lcd.print(setMinutes);

            lcd.setCursor(7, 1);
            if(sunriseStateValue == off)
            {
                lcd.print("OFF");
            }
            else
            {
                lcd.print(" ON");
            }
            lcd.setCursor(10, 0);
            if(confirmValue ==  ok)
            {
                lcd.print("    Ok");
            }
            else
            {
                lcd.print("Cancel");
            }
            
            lcd.setCursor(13, 1);
            if(sunRiseSpeed < 10)
            {
                lcd.print("0");
            }
            lcd.print(sunRiseSpeed);
            } 

        if (cursorPosition == 2)    // Set start minutes
        {
            lcd.home();
            lcd.print("Sunrise");
            
            lcd.setCursor(0, 1);
            if(sunRiseHours < 10)
            {
            lcd.print("0");
            }
            lcd.print(sunRiseHours);

            lcd.setCursor(2, 1);
            lcd.print(":");
            
            lcdBlink(sunRiseMinutes, timeValue, 2);

            lcd.setCursor(7, 1);
            if(sunriseStateValue == off)
            {
                lcd.print("OFF");
            }
            else
            {
                lcd.print(" ON");
            }               
            
            if(confirmValue ==  ok)
            {
                lcd.setCursor(10, 0);
                lcd.print("    Ok");
            }
            else
            {
                lcd.setCursor(10, 0);
                lcd.print("Cancel");
            }
            lcd.setCursor(13, 1);
            if(sunRiseSpeed < 10)
            {
                lcd.print("0");
            }
            lcd.print(sunRiseSpeed);
            }

        if (cursorPosition == 3)    // Set speed sunrise
        {
            lcd.home();
            lcd.print("Sunrise");
            
            lcd.setCursor(0, 1);
            if(sunRiseHours < 10)
            {
            lcd.print("0");
            }
            lcd.print(sunRiseHours);

            lcd.setCursor(2, 1);
            lcd.print(":");
            
            if(sunRiseMinutes < 10)
            {
            lcd.print("0");
            }
            lcd.print(sunRiseMinutes);

            lcd.setCursor(7, 1);
            if(sunriseStateValue == off)
            {
                lcd.print("OFF");
            }
            else
            {
                lcd.print(" ON");
            }               

            lcd.setCursor(13, 1);
            lcdBlink(sunRiseSpeed, timeValue, 3);

            lcd.setCursor(10, 0);
            if(confirmValue ==  ok)
            {
                lcd.print("    Ok");
            }
            else
            {
                lcd.print("Cancel");
            }
        }

        if (cursorPosition == 4)    // Set OFF-ON
        {
            lcd.home();
            lcd.print("Sunrise");
            
            lcd.setCursor(0, 1);
            if(sunRiseHours < 10)
            {
            lcd.print("0");
            }
            lcd.print(sunRiseHours);

            lcd.setCursor(2, 1);
            lcd.print(":");
            
            if(sunRiseMinutes < 10)
            {
            lcd.print("0");
            }
            lcd.print(sunRiseMinutes);            

            lcd.setCursor(13, 1);
            if(sunRiseSpeed < 10)
            {
                lcd.print("0");
            }
            lcd.print(sunRiseSpeed);
            
            lcd.setCursor(10, 0);
            if(confirmValue ==  ok)
            {
                lcd.print("    Ok");
            }
            else
            {
                lcd.print("Cancel");
            }

            if(sunRiseSpeed < 10)
            {
                lcd.print("0");
            }
            lcd.print(sunRiseSpeed);

            lcd.setCursor(7, 1);
            lcdBlink(sunriseStateValue, offOnValue, 3);
        }
    
        if (cursorPosition == 5)    // Set confirm
        {
            lcd.home();
            lcd.print("Sunrise");
            
            lcd.setCursor(0, 1);
            if(sunRiseHours < 10)
            {
                lcd.print("0");
            }
            lcd.print(sunRiseHours);

            lcd.setCursor(2, 1);
            lcd.print(":");
            
            if(sunRiseMinutes < 10)
            {
                lcd.print("0");
            }
            lcd.print(sunRiseMinutes);

            lcd.setCursor(7, 1);
            if(sunriseStateValue == off)
            {
                lcd.print("OFF");
            }
            else
            {
                lcd.print(" ON");
            }               

            lcd.setCursor(13, 1);
            if(sunRiseSpeed < 10)
            {
                lcd.print("0");
            }
            lcd.print(sunRiseSpeed);


            lcd.setCursor(10, 0);
            lcdBlink(confirmValue, okCancelValue, 6);
        }
        
        if (cursorPosition == 6)
        {
            cursorPosition = 1;
        }
    }
}
void editSundown()
{
    lcd.home();
    lcd.print("Sundown menu");
}

void systemOutput()
{
    lcd.setCursor(0, 0);
    lcd.print("System output ");
    
    lcd.setCursor(0, 1);
    lcd.print("U_LONG : ");
    lcd.print(sizeof(unsigned long));
}

void editNowTime()
{
  lcd.setCursor(11, 1);
          lcd.print("#edit");
          
          if(cursorPosition == 1)   // Set hours
          {
            lcd.setCursor(0, 0);
            lcdBlink(setHours, timeValue, 2);
            lcd.setCursor(2, 0);    
            lcd.print(":");
            if (setMinutes < 10)
            {
              lcd.print("0");
            }
            lcd.print(setMinutes);
            
            if(confirmValue ==  ok)
            {
                lcd.setCursor(10, 0);
                lcd.print("    Ok");
            }
            else
            {
                lcd.setCursor(10, 0);
                lcd.print("Cancel");
            }
            lcd.setCursor(0, 1);
            
            lcdPrintDate();
          }
          
          if(cursorPosition == 2)   // Set minutes
          {
            lcd.setCursor(0, 0);
            if (setHours < 10)
            {
              lcd.print("0");
            }
            lcd.print(setHours);

            lcd.print(":");

            lcdBlink(setMinutes, timeValue,  2);

            if(confirmValue ==  ok)
            {
                lcd.setCursor(10, 0);
                lcd.print("    Ok");
            }
            else
            {
                lcd.setCursor(10, 0);
                lcd.print("Cancel");
            }
            
            lcd.setCursor(0, 1);
            lcdPrintDate();
          }
          
          if(cursorPosition == 3)   // Set day
          {
            lcd.setCursor(0, 0);
            if (setHours < 10)
            {
              lcd.print("0");
            }
            lcd.print(setHours);

            lcd.setCursor(2, 0);
            lcd.print(":");
            lcd.setCursor(3, 0);
            if (setMinutes < 10)
            {
              lcd.print("0");
            }
            lcd.print(setMinutes);

            lcd.setCursor(10, 0);
             if(confirmValue ==  ok)
            {
                lcd.print("    Ok");
            }
            else
            {
                lcd.print("Cancel");
            }

            lcd.setCursor(0, 1);
            lcdBlink(setDay, dayValue, 2);
            lcd.setCursor(2, 1);
            lcd.print(".");
            if(setMonth < 10)
            {
                lcd.print("0");
            }
            lcd.print(setMonth);
            lcd.print(".");
            lcd.print(setYear);
          }
          
          if(cursorPosition == 4)   // Set month.
          {
            lcd.setCursor(0, 0);
            if (setHours < 10)
            {
              lcd.print("0");
            }
            lcd.print(setHours);

            lcd.setCursor(2, 0);
            lcd.print(":");
            lcd.setCursor(3, 0);
            if (setMinutes < 10)
            {
              lcd.print("0");
            }
            lcd.print(setMinutes);
            
            lcd.setCursor(10, 0);
            if(confirmValue ==  ok)
            {
                lcd.print("    Ok");
            }
            else
            {
                lcd.print("Cancel");
            }

            lcd.setCursor(0, 1);
            if(setDay < 10)
            {
                lcd.print("0");
            }
            lcd.print(setDay);
            lcd.print(".");
            lcdBlink(setMonth, monthValue, 2);
            lcd.setCursor(5, 1);
            lcd.print(".");
            lcd.print(setYear);
          }
        
          if(cursorPosition == 5)   // Set year.
          {
            lcd.setCursor(0, 0);
            if (setHours < 10)
            {
              lcd.print("0");
            }
            lcd.print(setHours);

            lcd.setCursor(2, 0);
            lcd.print(":");
            lcd.setCursor(3, 0);
            if (setMinutes < 10)
            {
              lcd.print("0");
            }
            lcd.print(setMinutes);
            
            lcd.setCursor(10, 0);
            if(confirmValue ==  ok)
            {
                lcd.print("    Ok");
            }
            else
            {
                lcd.print("Cancel");
            }

            lcd.setCursor(0, 1);
            if(setDay < 10)
            {
                lcd.print("0");
            }
            lcd.print(setDay);
            lcd.print(".");
            if(setMonth < 10)
            {
                lcd.print("0");
            }
            lcd.print(setMonth);
            lcd.print(".");
            lcd.setCursor(6, 1);
            lcdBlink(setYear, yearValue, 4);
          }

          if(cursorPosition == 6)   // Confirm changes
          {
            lcd.setCursor(0, 0);
          if (setHours < 10)
          {
            lcd.print("0");
          }
          lcd.print(setHours, DEC);

          lcdBlink(0, spacerValue, 1);
          
          lcd.setCursor(3, 0);
          if (setMinutes < 10)
          {
            lcd.print("0");
          }
          lcd.print(setMinutes, DEC);

          lcdPrintDate();  

          lcd.setCursor(10, 0);
          lcdBlink(confirmValue, okCancelValue, 6);
          }
}

void showEditTime()
{
  lcd.setCursor(0, 0);
  if (setHours < 10)
  {
    lcd.print("0");
  }
  lcd.print(setHours, DEC);

  lcdBlink(0, spacerValue, 1);

  lcd.setCursor(3, 0);
  if (setMinutes < 10)
  {
    lcd.print("0");
  }
  lcd.print(setMinutes, DEC);

  lcdPrintDate();
}