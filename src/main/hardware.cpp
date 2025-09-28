#include "hardware.h"


bool pwm_flag = false;
DHT11 dht11(PIN_DHT11);

bool dht11_flag = true;
String dht11_err = "";
int dht11_humi = 0;
int dht11_temp = 0;



//key
void key_init(){
  pinMode(PIN_BTN, INPUT_PULLUP);
}
Key_Press_State key_scan() {
  static unsigned long press_time_start = 0;
  static int btn_state_last = HIGH;
  static int btn_state_now = HIGH;

  btn_state_last = btn_state_now;
  btn_state_now = digitalRead(PIN_BTN);

  if (btn_state_last == HIGH && btn_state_now == HIGH) {
    //高电平
    return KEY_PRESS_NONE;
  } else if (btn_state_last == HIGH && btn_state_now == LOW) {
    //下降沿
    press_time_start = millis();  //按下记录时间
    return KEY_PRESS_DOWM;
  } else if (btn_state_last == LOW && btn_state_now == HIGH) {
    //上升沿
    unsigned long ms = millis(); 
    if (ms - press_time_start >= KEY_TIME_PRESS_TOO_LONG ) {
      return KEY_PRESS_UP_TOO_LONG;
    } else if(ms - press_time_start >= KEY_TIME_PRESS_LONG){
      return KEY_PRESS_UP_LONG;
    }else{
      return KEY_PRESS_UP_SHORT;
    }
  } else {
    //低电平
    if (millis() - press_time_start < KEY_TIME_PRESS_TOO_LONG) {
      return KEY_PRESS;
    } else {
      return KEY_PRESS_TOO_LONG;
    }
  }
}



//PWM
void pwm_init(){
  pinMode(PIN_PWM, OUTPUT);
  analogWriteRange(PWM_RANGE);
  analogWriteFreq(PWM_FREQ);
}

void pwm_on(){
  analogWrite(PIN_PWM, floor(PWM_RANGE/2));
  pwm_flag = true;
}

void pwm_off(){
  analogWrite(PIN_PWM, 0);
  pwm_flag = false;
}


bool pwm_state(){
  return pwm_flag;
}

//led
void led_init(){
  pinMode(PIN_LED, OUTPUT);
}

void led_on(){
  digitalWrite(PIN_LED,LOW);
}

void led_off(){
  digitalWrite(PIN_LED,HIGH);
}

bool led_state(){
  return digitalRead(PIN_LED)==LOW;
}


//dht11
bool dht11_status(){
  return dht11_flag;
}

//debug
bool dht11_read(){
  int result = dht11.readTemperatureHumidity(dht11_temp, dht11_humi);

  // int result = 0; dht11_humi = random()%5+40;dht11_temp = random()%5+25;
  if(result == 0){
    dht11_flag = true;
  }else{
    dht11_flag = false;
    dht11_err= DHT11::getErrorString(result);
  }
  return dht11_flag;
}

String dht11_get_err_string(){
  return dht11_err;
}

int dht11_get_humi(){
  return dht11_humi;
}

int dht11_get_temp(){
  return dht11_temp;
}



