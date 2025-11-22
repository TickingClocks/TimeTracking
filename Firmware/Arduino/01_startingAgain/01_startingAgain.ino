#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "ui.h"
#include <Adafruit_NeoPixel.h>

#define PIN        8
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

//button variables
//switch connections
int switch1 = 1; //correct
int switch2 = 9; //correct
int switch3 = 19; //correct
int switch4 = 23; //correct
int switch5 = 20; //correct
int switch6 = 2; //check //squareline7
int switch7 = 0; //check //squareline 8
int switch8 = 18; //check //squareline 9

bool switch1_state = 0;
bool switch2_state = 0;
bool switch3_state = 0;
bool switch4_state = 0;
bool switch5_state = 0;
bool switch6_state = 0;
bool switch7_state = 0;
bool switch8_state = 0;

//keeping track of each second
long currentSecondCount = 0;
long lastSecondCount = 0;

long task1SecondCount = 0; //timer for task 1
long task2SecondCount = 0; //timer for task 2
long task3SecondCount = 0; //timer for task 3
long task4SecondCount = 0;
long task5SecondCount = 0;
long task6SecondCount = 0;

//converted time variables
int task1Seconds = 0;
int task1Minutes = 0;
int task1Hours = 0;

int task2Seconds = 0;
int task2Minutes = 0;
int task2Hours = 0;

int task3Seconds = 0;
int task3Minutes = 0;
int task3Hours = 0;

int task4Seconds = 0;
int task4Minutes = 0;
int task4Hours = 0;

int task5Seconds = 0;
int task5Minutes = 0;
int task5Hours = 0;

int task6Seconds = 0;
int task6Minutes = 0;
int task6Hours = 0;

//LED variables
long currentLEDBreatheCount = 0;
long lastLEDBreatheCount = 0;
long LED_Breathe_Interval = 10; //ms
long LED_Breathe_R = 0;
long LED_Breathe_G = 0;
long LED_Breathe_B = 0;
bool LED_Breathe_Direction = 1;
uint8_t LED_Breathe_Increment = 1;
bool time1Counting = 0; //switch 1 state
bool timer2Counting = 0; //switch 2 state
bool timer3Counting = 0; //switch 3 state
bool timer4Counting = 0;
bool timer5Counting = 0;
bool timer6Counting = 0;

uint8_t randomCount = 0;

bool splashAnimation = 0; //has the splashscreen run?

uint8_t uartValue = 75; //value we are going to display - keep below 100

long valueChangeTime = 0;
long valueChangeLastTime = 0;
long valueChangeInterval = 100;
int valueChangeUp = 1;


void setup()
{       
  LCD_Init();
  Lvgl_Init(); 
  ui_init();

  pinMode(switch1, INPUT_PULLUP);
  pinMode(switch2, INPUT_PULLUP);
  pinMode(switch3, INPUT_PULLUP);
  pinMode(switch4, INPUT_PULLUP);
  pinMode(switch5, INPUT_PULLUP);
  pinMode(switch6, INPUT_PULLUP);
  pinMode(switch7, INPUT_PULLUP);
  pinMode(switch8, INPUT_PULLUP);

  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0, 150, 0));
  pixels.show();

  //splash screen
  lv_obj_add_flag(ui_Spinner3, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Spinner4, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Spinner5, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_Image1, LV_OBJ_FLAG_HIDDEN); // show
  Timer_Loop(); //run lvgl
  //wait to display picture
  //delay(3000);
  switch5_state = digitalRead(switch5);
  while(switch5_state == 1){
    switch5_state = digitalRead(switch5);
  }
  //lv_obj_add_flag(ui_Image1, LV_OBJ_FLAG_HIDDEN);   // hide
  Timer_Loop(); //run lvgl

  Serial.begin(9600);

}

void loop()
{

  if(splashAnimation < 1){
    splashAnimation = 1;
    slide_image_off_screen(ui_Image1);
  }

  readSwitches(); //reads status of the physical switches
  matchScreenToStates(); //updates system based on switch states
  checkSecondsCounter(); //timer keeping functions

  if(Serial.available())
  {
    uartValue = Serial.read();
  }

  lv_label_set_text_fmt(ui_value, "%d", uartValue); //set screen value
  lv_arc_set_value(ui_Arc1, uartValue); //set arc value
  //update the arc on screen 2
  //lv_arc_set_value(ui_Arc1, uartValue);

  int switchCheck = switch1_state + switch2_state + switch3_state;

  if(randomCount > 1){
    randomCount = 0;
    if(switchCheck > 0){
      if(switchCheck > 1){
        LED_Breathe(4);
      }else{
        if(switch1_state > 0){
          LED_Breathe(1);
        }
        if(switch2_state > 0){
          LED_Breathe(2);
        }
        if(switch3_state > 0){
          LED_Breathe(3);
        }
      }
    }else{
      //all switches are off - go to default state
      pixels.setPixelColor(0, pixels.Color(0, 0, 20));
      pixels.show();
    }
    
  }
  
  Timer_Loop(); //run lvgl
  delay(5);

  randomCount++;

}

void slide_image_off_screen(lv_obj_t * img)
{
    int startX = lv_obj_get_x(img);
    int endX   = 320;   // adjust if your screen width is different

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_values(&a, startX, endX);
    lv_anim_set_time(&a, 1500);  // duration in ms //originally 600
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);  // nice smooth ease
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);

    // optionally hide after sliding off screen
    lv_anim_set_ready_cb(&a, [](lv_anim_t * a) {
        lv_obj_add_flag((lv_obj_t*)a->var, LV_OBJ_FLAG_HIDDEN);
    });

    lv_anim_start(&a);
}

void checkSecondsCounter(void){
  currentSecondCount = millis();
  if(currentSecondCount > lastSecondCount + 1000){
    //one second has passed
    lastSecondCount = currentSecondCount;
    incrementTimers();
  }
}

void setTimerScreenText(void){

  lv_label_set_text_fmt(ui_timer1, "1 || h: %d m: %d s: %d", task1Hours, task1Minutes, task1Seconds);
  lv_label_set_text_fmt(ui_timer2, "2 || h: %d m: %d s: %d", task2Hours, task2Minutes, task2Seconds);
  lv_label_set_text_fmt(ui_timer3, "3 || h: %d m: %d s: %d", task3Hours, task3Minutes, task3Seconds);
}

void incrementTimers(void){
  int changeOccurred = 0;

  if(switch1_state){
    //increment switch 1 timer
    task1SecondCount++;
    changeOccurred++;
  }

  if(switch2_state){
    //increment switch 2 timer
    task2SecondCount++;
    changeOccurred++;
  }

  if(switch3_state){
    //increment switcch 3 timer
    task3SecondCount++;
    changeOccurred++;
  }

  if(changeOccurred > 0){
    convertTimers();
    setTimerScreenText();
  }
}

void convertTimers(void){
  //convert timer 1
  if(task1SecondCount > 3600){ //greater than 1 hour
    task1Hours = task1SecondCount / 3600;
    task1Minutes = (task1SecondCount - (task1Hours * 3600)) / 60;
    task1Seconds = (task1SecondCount - ((task1Hours * 3600) + (task1Minutes * 60)));
  }else{
    if(task1SecondCount > 60){
      task1Hours = 0;
      task1Minutes = (task1SecondCount - (task1Hours * 3600)) / 60;
      task1Seconds = (task1SecondCount - ((task1Hours * 3600) + (task1Minutes * 60)));
    }else{
      task1Hours = 0;
      task1Minutes = 0;
      task1Seconds = (task1SecondCount - ((task1Hours * 3600) + (task1Minutes * 60)));
    }

  }

  //convert timer 2
  if(task2SecondCount > 3600){
    task2Hours = task2SecondCount / 3600;
    task2Minutes = (task2SecondCount - (task2Hours * 3600)) / 60;
    task2Seconds = (task2SecondCount - ((task2Hours * 3600) + (task2Minutes * 60)));
  }else{
    if(task2SecondCount > 60){
      task2Hours = 0;
      task2Minutes = (task2SecondCount - (task2Hours * 3600)) / 60;
      task2Seconds = (task2SecondCount - ((task2Hours * 3600) + (task2Minutes * 60)));
    }else{
      task2Hours = 0;
      task2Minutes = 0;
      task2Seconds = (task2SecondCount - ((task2Hours * 3600) + (task2Minutes * 60)));
    }
  }

  //convert timer 3
  if(task3SecondCount > 3600){
    task3Hours = task3SecondCount / 3600;
    task3Minutes = (task3SecondCount - (task3Hours * 3600)) / 60;
    task3Seconds = (task3SecondCount - ((task3Hours * 3600) + (task3Minutes * 60)));
  }else{
    if(task3SecondCount > 60){
      task3Hours = 0;
      task3Minutes = (task3SecondCount - (task3Hours * 3600)) / 60;
      task3Seconds = (task3SecondCount - ((task3Hours * 3600) + (task3Minutes * 60)));
    }else{
      task3Hours = 0;
      task3Minutes = 0;
      task3Seconds = (task3SecondCount - ((task3Hours * 3600) + (task3Minutes * 60)));
    }
  }
}

void readSwitches(void){
  switch1_state = !digitalRead(switch1);
  switch2_state = !digitalRead(switch2);
  switch3_state = !digitalRead(switch3);
  switch4_state = !digitalRead(switch4);
  switch5_state = !digitalRead(switch5);
}

void matchScreenToStates(void){
  if(switch1_state > 0){
    lv_obj_add_state(ui_Switch1, LV_STATE_CHECKED);
    lv_obj_clear_flag(ui_Spinner3, LV_OBJ_FLAG_HIDDEN); // Show    
  }else{
    lv_obj_clear_state(ui_Switch1, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_Spinner3, LV_OBJ_FLAG_HIDDEN); // hidden
  }

  if(switch2_state > 0){
    lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);
    lv_obj_clear_flag(ui_Spinner4, LV_OBJ_FLAG_HIDDEN); // Show
  }else{
    lv_obj_clear_state(ui_Switch2, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_Spinner4, LV_OBJ_FLAG_HIDDEN); // hidden
  }

  if(switch3_state > 0){
    lv_obj_add_state(ui_Switch3, LV_STATE_CHECKED);
    lv_obj_clear_flag(ui_Spinner5, LV_OBJ_FLAG_HIDDEN); // Show
  }else{
    lv_obj_clear_state(ui_Switch3, LV_STATE_CHECKED);
    lv_obj_add_flag(ui_Spinner5, LV_OBJ_FLAG_HIDDEN); // hidden
  }

  if(switch4_state > 0){
    lv_obj_add_state(ui_Switch4, LV_STATE_CHECKED);
    lv_obj_add_state(ui_Switch6, LV_STATE_CHECKED);
    lv_scr_load(ui_Screen2);
  }else{
    lv_obj_clear_state(ui_Switch4, LV_STATE_CHECKED);
    lv_obj_clear_state(ui_Switch6, LV_STATE_CHECKED);
    lv_scr_load(ui_Screen1);
  }

  if(switch5_state > 0){
    lv_obj_add_state(ui_Switch5, LV_STATE_CHECKED);
  }else{
    lv_obj_clear_state(ui_Switch5, LV_STATE_CHECKED);
  }
}

void LED_setBreathe(void){
  
  int colorOption = 0;
  colorOption = switch1_state + switch2_state + switch3_state;

  if(colorOption > 0){ //is a timer counting
    if(colorOption > 1){ //are two or more timers counting
      LED_Breathe(4); 
    }else{
      if(switch1_state > 0){
        LED_Breathe(1);
      }
      if(switch2_state > 0){
        LED_Breathe(2);
      }
      if(switch3_state > 0){
        LED_Breathe(3);
      }
    }
  }else{
    //no switches are flipped - default non error color
      pixels.setPixelColor(0, pixels.Color(0, 100, 0));
      pixels.show();
  }
}

bool checkBreatheTime(void){
  int colorOption = 0;
  currentLEDBreatheCount = millis();
  if(currentLEDBreatheCount > lastLEDBreatheCount + LED_Breathe_Interval){
    lastLEDBreatheCount = currentLEDBreatheCount;
    //enough time has elapsed to change the LED color
    LED_setBreathe();
  }
}

void LED_Breathe(uint8_t colorOption){

int stepSize = 2;
int topEnd = 250;
int lowEnd = 25;

  switch (colorOption){
    case 0:
      //no switches flipped, no breathe animation
    break;

    case 1:
    //time 1 counting = green pulse
    if(LED_Breathe_Direction > 0){
      //count up
      if(LED_Breathe_G < topEnd){
        LED_Breathe_G+=stepSize;
      }else{
        LED_Breathe_Direction = 0;
      }
    }else{
      //count down
      if(LED_Breathe_G > lowEnd){
        LED_Breathe_G-=stepSize;
      }else{
        LED_Breathe_Direction = 1;
      }
    }
    LED_Breathe_R = 0;
    LED_Breathe_B = 0;
    break;

    case 2:
    //time 2 counting = blue pulse
    if(LED_Breathe_Direction > 0){
      //count up
      if(LED_Breathe_B < topEnd){
        LED_Breathe_B+=stepSize;
      }else{
        LED_Breathe_Direction = 0;
      }
    }else{
      //count down
      if(LED_Breathe_B > lowEnd){
        LED_Breathe_B-=stepSize;
      }else{
        LED_Breathe_Direction = 1;
      }
    }
    LED_Breathe_R = 0;
    LED_Breathe_G = LED_Breathe_B;
    break;

    case 3:
    //time 3 counting = purple pulse
    if(LED_Breathe_Direction > 0){
      //count up
      if(LED_Breathe_B < topEnd){
        LED_Breathe_B+=stepSize;
        //LED_Breathe_B+=10;
      }else{
        LED_Breathe_Direction = 0;
      }
    }else{
      //count down
      if(LED_Breathe_B > lowEnd){
        LED_Breathe_B-=stepSize;
      }else{
        LED_Breathe_Direction = 1;
      }
    }
    LED_Breathe_R = LED_Breathe_B;
    LED_Breathe_G = 0;
    break;

    case 4:
    //multiple switches flipped - red
    if(LED_Breathe_Direction){
      //count up
      if(LED_Breathe_R < topEnd){
        LED_Breathe_R+=stepSize;
      }else{
        LED_Breathe_Direction = 0;
      }
    }else{
      //count down
      if(LED_Breathe_R > lowEnd){
        LED_Breathe_R-=stepSize;
      }else{
        LED_Breathe_Direction = 1;
      }
    }
    LED_Breathe_B = 0;
    LED_Breathe_G = 0;
    break;
  }

  pixels.setPixelColor(0, pixels.Color(LED_Breathe_R, LED_Breathe_G, LED_Breathe_B));
  pixels.show();

}

