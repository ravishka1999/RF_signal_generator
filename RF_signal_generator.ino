
#include <LiquidCrystal.h>
#include <si5351.h>

const long band80mstart = 3500000;
const long bald80mEnd = 3900000;
const long band40mstart = 7000000;
const long bald40mEnd = 7155000;
volatile long freq = 3700000;
volatile long radix = 1000;
volatile long oldfreq = 0;
volatile long old80mfreq = 3700000;
volatile long old40mfreq = 7060000;
volatile long currentfreq = 0;
volatile int updatedisplay = 0;
volatile int band = 0;
volatile int oldband = 0;

static const int rotApin = 2;
static const int rotBpin = 3;
static const int pushSWpin = 4;

static const int BandSWpin = 5;

volatile int rotstate = 0;
volatile int rotAval = 1;
volatile int rotBval = 1;

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Si5351 si5351;



void setup() {
  pinMode(rotApin, INPUT);
  pinMode(rotBpin, INPUT);
  pinMode(pushSWpin, INPUT);
  pinMode(BandSWpin, INPUT);

  digitalWrite(rotApin, HIGH);
  digitalWrite(rotBpin, HIGH);
  digitalWrite(pushSWpin, HIGH);
  digitalWrite(BandSWpin, HIGH);

  attachInterrupt(digitalPinToInterrupt(rotApin), ISRrotAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotBpin), ISRrotBChange, CHANGE);

  lcd.begin(16, 2);
  lcd.cursor();

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(191, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

  UPdateDisplay();
  SendFrequency();

}

void loop() {
  currentfreq = getfreq();

  if (currentfreq != oldfreq) {
    UPdateDisplay();
    SendFrequency();
    oldfreq = currentfreq;

  }
  if (digitalRead(pushSWpin) == LOW) {
    delay(10);
    while (digitalRead(pushSWpin) == LOW) {
      if (updatedisplay == 1) {
        UPdateDisplay();
        updatedisplay = 0;
      }
    }
    delay(50);
  }

  if (digitalRead(BandSWpin) == LOW)
    band = 1;
  else
    band = 0;
  if (band != oldband) {
    if (band == 0) {
      old40mfreq = freq;
      freq = old80mfreq;
    }

    if (band == 1) {
      old80mfreq = freq;
      freq = old40mfreq;
    }
    UPdateDisplay();
    oldband = band;
  }
}
long getfreq() {
  long temp_freq;
  cli();
  temp_freq = freq;
  sei();
  return temp_freq;
}

void ISRrotAChange() {
  if (digitalRead(rotApin)) {
    rotAval = 1;
    UpdateRot();
  } else {
    rotAval = 0;
    UpdateRot();
  }
}

void ISRrotBChange() {
  if (digitalRead(rotBpin)) {
    rotBval = 1;
    UpdateRot();
  } else {
    rotBval = 0;
    UpdateRot();
  }
}

void UpdateRot() {
  switch (rotstate)
  {
    case 0:
      if (!rotBval)
        rotstate = 1;
      if (!rotAval)
        rotstate = 11;
      break;


    case 1:
      if (!rotBval)
      {
        if (!rotAval)
        {
          if (digitalRead(pushSWpin) == LOW)
          {
            updatedisplay = 1;
            if (radix == 1000000)
              radix = 100000;
            else if (radix == 100000)
              radix = 10000;

            else if (radix == 10000)
              radix = 1000;

            else if (radix == 1000)
              radix = 100;

            else if (radix == 100)
              radix = 10;

            else if (radix == 10)
              radix = 1;

            else
              radix = 1000000;

          }
          else
          {
            freq = (freq + radix);
            if (band == 0)
              if (freq > bald80mEnd)
                freq = bald80mEnd;
            if (band == 1)
              if (freq > bald40mEnd)
                freq = bald40mEnd;

          }

          rotstate = 2;


        }
      }

      else if (rotAval)
        rotstate = 0;
      break;

    case 2:

      if (rotBval)
        rotstate = 3;
      break;


    case 3:

      if (rotAval)
        rotstate = 0;
      break;

    case 11:
      if (!rotAval)
      {
        if (!rotBval)
        {
          if (digitalRead(pushSWpin) == LOW)
          {
            updatedisplay = 1;
            if (radix == 1)
              radix = 10;


            else if (radix == 10)
              radix = 100;

            else if (radix == 100)
              radix = 1000;

            else if (radix == 1000)
              radix = 10000;

            else if (radix == 10000)
              radix = 100000;

            else if (radix == 100000)
              radix = 1000000;

            else
              radix = 1;

          }
          else
          {
            freq = (freq - radix);
            if (band == 0)
              if (freq < band80mstart)
                freq = band80mstart;
            if (band == 1)
              if (freq < band40mstart)
                freq = band40mstart;
          }
          rotstate = 12;
        }
      }
      else if (rotBval)
        rotstate = 0;
      break;

    case 12 :
      if (rotAval)
        rotstate = 13;
      break;

    case 13 :
      if (rotBval)
        rotstate = 0;
      break;
  }
}

void UPdateDisplay() {
  lcd. setCursor ( 0, 0);
  lcd.print ("         ");
  lcd.setCursor ( 0, 0);
  lcd.print(freq);
  lcd.setCursor (10, 0);
  lcd. print (" VFO");

  lcd.setCursor(0, 1);
  lcd.print("   ");
  lcd.setCursor(0, 1);

  if (radix == 1)
    lcd.setCursor ( 6, 0);
  if (radix == 10)
    lcd.setCursor ( 5, 0);
  if (radix == 100)
    lcd.setCursor ( 4, 0);
  if (radix == 1000)
    lcd.setCursor ( 3, 0);
  if (radix == 10000)
    lcd.setCursor ( 2, 0);
  if (radix == 100000)
    lcd.setCursor ( 1, 0);
  if (radix == 1000000)
    lcd.setCursor ( 0, 0);

}

void SendFrequency() {
  si5351.set_freq(((freq-0) * 100ULL), SI5351_CLK0);
}
