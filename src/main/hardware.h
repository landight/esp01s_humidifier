#ifndef _HARDWARE_H
#define  _HARDWARE_H

#include "Arduino.h"
#include "DHT11.h"

//引脚相关
#define PIN_BTN 3
#define PIN_DHT11 4
#define PIN_LED 2
#define PIN_PWM 0
#define PIN_PWM_PERIPHS PERIPHS_IO_MUX_GPIO0_U
#define PIN_PWM_FUNC FUNC_GPIO0

//按键时长判断相关
#define KEY_TIME_PRESS_TOO_LONG (10*1000)
#define KEY_TIME_PRESS_LONG 500

//pwm频率相关
#define PWM_FREQ 108000
#define PWM_RANGE 255


typedef enum {
  KEY_PRESS,  //按下(持续)
  KEY_PRESS_NONE, //未按下(持续)
  KEY_PRESS_TOO_LONG,//按下超时(持续)
  KEY_PRESS_DOWM,//按下(瞬间)
  KEY_PRESS_UP_SHORT,//短按松开(瞬间)
  KEY_PRESS_UP_LONG,//长按松开(瞬间)
  KEY_PRESS_UP_TOO_LONG,//按下超时松开(瞬间)
} Key_Press_State;


void key_init(void);
Key_Press_State key_scan(void);

void pwm_init(void);
void pwm_on(void);
void pwm_off(void);
bool pwm_state(void);

void led_init(void);
void led_on(void);
void led_off(void);
bool led_state(void);

bool dht11_status(void);
bool dht11_read(void);
String dht11_get_err_string(void);
int dht11_get_humi(void);
int dht11_get_temp(void);

#endif
