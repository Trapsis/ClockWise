// ClockWise code for Brandy Caterino's Invention Convention project
// ClockWise is a time managment tool that helps you organize your study time
//   The software drives three LCD screens representing the three upcoming study subjects you are working on
//   NeoPixel rings represent what percentage of time you have left
//   Three libraries are used to manage time, drive the LCDs, and the neoPixel rings
#include <timer.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

// Buzzer Pin
int buzPin = 4;                                      //<-----------------------PIN
#define NOTE_C  131


// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// Each display has had a different address set by soldering a different jumper.
LiquidCrystal_I2C lcd0(0x27, lcdColumns, lcdRows);  
LiquidCrystal_I2C lcd1(0x26, lcdColumns, lcdRows);  
LiquidCrystal_I2C lcd2(0x25, lcdColumns, lcdRows); 

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            2  //Neopixel Pin              <-----------------------PIN
#define NUMPIXELS      48

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

auto timer = timer_create_default(); // create a timer with default settings

String timerSubject[][6]={{"Math","0","15","0","0","40"},
          {"Science","0","10","40","40","0"},
          {"BREAK","0","20","40","0","0"},
          {"Spanish","0","25","00","40","40"}};
const int numSubjects = sizeof(timerSubject)/sizeof(timerSubject[0]);
int currentSubject;
long myTimerStart;
//long mySetTimer;

class subject{
  public:
  String subjectName;
  long millisTime;
  int red;
  int green;
  int blue;
  subject(int arrayIn){
    subjectName = timerSubject[arrayIn][0];
    millisTime = (timerSubject[arrayIn][1].toInt()*60000)+(timerSubject[arrayIn][2].toInt()*1000);
    red = timerSubject[arrayIn][3].toInt();
    green = timerSubject[arrayIn][4].toInt();
    blue = timerSubject[arrayIn][5].toInt();
  }
};



  
bool doEverySecond(void *) {
  if (currentSubject != numSubjects){
  
    subject mySubject(currentSubject); //get the currect subject from the array
    long timeLeft = (myTimerStart+mySubject.millisTime)-millis();  //how much time is left?
  
    ShowDisplay(0,mySubject.subjectName,getDisplayTime(timeLeft));
    
    //light up the %time left on the NeoPixel ring
    float percentLeft = 1.00-(((float)millis()-(float)myTimerStart)/(float)mySubject.millisTime);
    ShowPercent(0 ,mySubject.red, mySubject.green, mySubject.blue, percentLeft);
    
    //return
    return true; // repeat? true    
  }
 else{
    return false;
 }
  
}

bool doTimerEnd(void *) {
  Serial.print("Timer DONE: ");
  Serial.println(millis());
  
  //ring the buzzer
  timer.in(1, soundBuzzer);
  timer.in(300, silenceBuzzer);
  timer.in(350, soundBuzzer);
  timer.in(650, silenceBuzzer);
  timer.in(700, soundBuzzer);
  timer.in(1000, silenceBuzzer);
  
  currentSubject += 1; //Advance the subject counter  
  
  //Check to see if there are any more subjects in the array
  if (currentSubject != numSubjects){
    subject mySubject(currentSubject);  //get the next item in the timerSubject array and put it in subject object
    myTimerStart = millis();
    timer.in(mySubject.millisTime, doTimerEnd); //Set a timer for the next subject    
  }

  lcd0.clear();  //Clear screen 0
  ShowPercent(0 ,0, 0, 0, 0);  //Clear neoPixel ring 0 

  showOtherScreens(currentSubject);

  return true; // repeat? true
}

void setup() {
  
  Serial.begin(115200);

  pixels.begin(); // This initializes the NeoPixel library.
  // initialize LCD
  lcd0.init();
  lcd1.init();
  lcd2.init();
  // turn on LCD backlight                      
  lcd0.backlight();
  lcd1.backlight();
  lcd2.backlight();
  
  //ledcSetup(0,1E5,12); //setup the buzzer
  //ledcAttachPin(04,0);
  //ledcAttachPin(buzPin,0);

  currentSubject = 0;
  subject mySubject(currentSubject);  //get the first item in the timerSubject array and put it in subject object
  
  myTimerStart = millis();
  timer.in(mySubject.millisTime, doTimerEnd); //Set a timer for the first subject 
  
  timer.every(1000, doEverySecond);
  
  showOtherScreens(currentSubject);
}

void loop() {
  timer.tick(); // tick the timer
}


bool soundBuzzer(void *) {
  //ledcWriteNote(0,NOTE_C,6);
  tone(buzPin,NOTE_C);
  return true;      
}

bool silenceBuzzer(void *) {
  //ledcWriteTone(0,0); 
  noTone(buzPin);
  return true;    
}

void ShowDisplay(int displaynum, String line1, String line2){
  switch (displaynum) {
    case 0:
      lcd0.clear();
      lcd0.setCursor(0, 0);
      lcd0.print(line1);
      lcd0.setCursor(0, 1);
      lcd0.print(line2);    
      break;  
    case 1:
      lcd1.clear();
      lcd1.setCursor(0, 0);
      lcd1.print(line1);
      lcd1.setCursor(0, 1);
      lcd1.print(line2);   
      break;
    case 2:
      lcd2.clear();
      lcd2.setCursor(0, 0);
      lcd2.print(line1);
      lcd2.setCursor(0, 1);
      lcd2.print(line2);      
      break;
  }
}

void ClearDisplay(int displaynum){
  switch (displaynum) {
    case 0:
      lcd0.clear();   
      break;  
    case 1:
      lcd1.clear();
      break;
    case 2:
      lcd2.clear();     
      break;
  }  
}

void ShowPercent(int ring ,int red, int green, int blue, float percent){
  int startAddress = ring * 16;
  int endAddress = startAddress + 15;
  int lightpix = (round(16*percent)-1) + startAddress;  

  uint32_t blankColor = pixels.Color(0,0,0);
  uint32_t color = pixels.Color(red, green, blue);

  for(int i=startAddress; i<endAddress+1; i++){

    if (i<=lightpix){
      pixels.setPixelColor(i, color);
    }
    else{
      pixels.setPixelColor(i, blankColor);
    }  
  }
  pixels.show();
}

String getDisplayTime(long inTime){
    //Get hours, munutes, seconds left
    unsigned long allSeconds=inTime/1000;
    int runHours= allSeconds/3600;
    int secsRemaining=allSeconds%3600;
    int runMinutes=secsRemaining/60;
    int runSeconds=secsRemaining%60;
    
    char buf[21];
    sprintf(buf,"%02d:%02d:%02d",runHours,runMinutes,runSeconds);

    return buf;    
}

void showOtherScreens(int currentSub){
  int currentDisplay = 1;

  for (int i = currentSub+1; i < currentSub+3; i++){
    
    //Check to see if there is another subject after the current one
    if (i < numSubjects){ //There is another subject so display it on currentDisplay
      subject mySubject(i);
      //Display the next subject and the total time on the currentDisplay screen
      ShowDisplay(currentDisplay, mySubject.subjectName, getDisplayTime(mySubject.millisTime));
      
      //Display 100% of the neoPixels in the correct color on the currentDisplay neoPixel ring   
      ShowPercent(currentDisplay ,mySubject.red, mySubject.green, mySubject.blue, 1);   
    }
    else{ //blank currentDisplay
      //Display the next subject and the total time on the currentDisplay screen
      ClearDisplay(currentDisplay);
      
      //Display 100% of the neoPixels in the correct color on the currentDisplay neoPixel ring   
      ShowPercent(currentDisplay ,0 ,0 ,0 , 1);        
    }
    currentDisplay++;
  }
}
