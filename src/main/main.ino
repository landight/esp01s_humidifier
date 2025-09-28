#include "hardware.h"
#include <ESP8266WebServer.h>
#include <Arduino.h>

const static char index_html_text[] = R"==(
<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>加湿器</title><style>*{margin:0;padding:0;user-select:none}.top>div{width:50vw;text-align:center}.top>div :nth-child(1){font-size:small;color:#666;margin-top:10px}.top>div :nth-child(2){font-size:larger;color:#333}.img :nth-child(1){margin-top:10vw;width:5vw;height:2vw;background-color:#b2e8ff;margin-left:60vw}.img :nth-child(2){margin-top:2px;width:50vw;height:18vw;background-color:#d2e8ff;border-radius:20% 20% 0 0;margin-left:25vw}.img :nth-child(3){margin-top:5px;width:50vw;height:30vw;background-color:#d2e8ff;border-radius:0 0 5% 5%;margin-left:25vw}.state td:nth-child(1){font-size:small;font-weight:700}.state tr:nth-child(2n){background-color:#f9f9f9}.state td{font-size:small;width:calc(50vw - 10px);padding-left:10px;height:25px}button{width:100px;height:30px;background-color:#fff;border-width:1px;box-shadow:2px 2px 1px #ddd,1px 1px 1px inset #eee}</style></head><body><div class="top" style="display: flex;box-shadow: 0px 1px 5px #eee;"><div><p>湿度</p><p id="humi_">-</p></div><div><p>温度</p><p id="temp_">-</p></div></div><div class="img" style="width: 100vw; height: 60vw;"><div></div><div></div><div></div></div><div class="state" style="display: flex; justify-content: center; margin: 30px 10px 10px 10px; border: 1px solid #666;"><table><tr><td>工作状态</td><td id="state_">-</td></tr><tr><td>工作模式</td><td id="mode_">-</td></tr><tr><td>加湿状态</td><td id="pwm_">-</td></tr><tr><td>传感器状态</td><td id="dht11_">-</td></tr><tr id="dht11ErrTr"><td>传感器错误信息</td><td id="dht11ErrStr_">-</td></tr><tr><td>LED状态</td><td id="led_">-</td></tr><tr><td>工作时长</td><td id="workTime_">-</td></tr><tr><td>当前模式时长</td><td id="modeTime_">-</td></tr></table></div><div class="control"><div class="mode" style="display: flex;justify-content: space-between; margin: 0 10px 0px 10px;"><select name="mode" id="modeSelect" style="width: 120px; height: 30px;"><option value="free">空闲模式</option><option value="time_30_min">定时半小时</option><option value="time_60_min">定时一小时</option><option value="auto">自动</option></select><button id="modeBtn" onclick="modeUpdate()">修改模式</button><button id="ledBtn" onclick="ledUpdate()">开启LED</button></div></div></body><script>var device={workTime:0,modeTime:0};const state2Str={'work':'正常工作','no_water':'缺水','err':'出错',};const mode2Str={'free':"空闲状态",'time_30_min':"定时-30分钟",'time_60_min':"定时-60分钟",'auto':"自动",};let time=0;setInterval(()=>{device.workTime++;device.modeTime++;dataDisplay();if(time % 3==0){getState()}time++},1000);function time2Str(s){let min=Math.floor(s/60);if(min==0) return s+'s';let h=Math.floor(min/60);if(h==0) return min+'min '+s%60+'s';return h+'h '+min%60+'min '+s%60+'s'}function dataDisplay(){const{humi,temp,led,dht11,pwm,dht11ErrStr,state,mode,modeTime,workTime}=device;humi_.innerHTML=humi+'%';temp_.innerHTML=temp+'℃';led_.innerHTML=led==1?"开":"关";dht11_.innerHTML=dht11=='1'?"正常":"出错";pwm_.innerHTML=pwm=='1'?"加湿中":"暂停加湿";dht11ErrTr.style.display=(dht11=='1')?"none":"table-row";dht11ErrStr_.innerHTML=(dht11=='0')?dht11ErrStr:"正常工作";state_.innerHTML=state2Str[state];mode_.innerHTML=mode2Str[mode];modeTime_.innerHTML=time2Str(modeTime);workTime_.innerHTML=time2Str(workTime);ledBtn.innerHTML=led==1?"关闭LED":"开启LED"}
function getState(){fetch('/state') .then(async(res)=>{let text=await res.text();let arr=text.split(" \n ");console.log(arr);arr.forEach((str)=>{let [name,value]=str.split(':');device[name]=value});dataDisplay()})}function modeUpdate(){if(modeSelect.value==device.mode){alert("已经在这个模式工作了");return};const formData=new URLSearchParams();formData.append('mode',modeSelect.value);fetch('/mode/update',{method:'POST',body:formData,}).then(async(res)=>{let text=await res.text();console.log(text);if(text=='true'){device.mode=modeSelect.value;dataDisplay()}else{alert("切换模式失败")}})}function ledUpdate(){console.log('/led/'+(device.led=='1'?"off":"on"));fetch('/led/'+(device.led=='1'?"off":"on"),{method:'POST',}).then(async(res)=>{let text=await res.text();if(text=='true'){device.led=Number.parseInt(device.led)*(-1)+1;dataDisplay()}else{alert(device.led?"关闭":"开启"+"LED失败")}})}</script></html>

)==";


// 设备工作模式 和 工作状态
enum Device_Mode {
  DEVICE_MODE_FREE,         // 空闲
  DEVICE_MODE_TIME_30_MIN,  // 定时30min
  DEVICE_MODE_TIME_60_MIN,  // 定时60min
  DEVICE_MODE_AUTO,         // 自动 20-40湿度
  DEVICE_MODE_ENUM_COUNT,
};
enum Device_State {
  DEVICE_STATE_WORK,      // 工作
  DEVICE_STATE_NO_WATER,  // 缺水
  DEVICE_STATE_ERR,       // 出错
};
const String state2str[] = { "work", "no_water","err" };
const String mode2str[] = { "free", "time_30_min", "time_60_min", "auto" };


uint8_t device_mode = DEVICE_MODE_FREE;
uint8_t device_mode_last = DEVICE_MODE_FREE;
uint8_t device_state = DEVICE_STATE_WORK;
uint8_t device_state_last = DEVICE_STATE_WORK;


//计时
unsigned long start_ms = 0;



//网络
#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#define SERVER_PORT 80
ESP8266WebServer server(SERVER_PORT);
#endif

void setup() {
  key_init();
  pwm_init();
  led_init();
  pwm_off();
  web_init();
}

void loop() {
  handler_key();
  handler_state();
  server.handleClient();
  handler_dht11();
}


void handler_key() {
  switch (key_scan()) {
    case KEY_PRESS_UP_SHORT:
      {
        // 短按 切换模式
        device_mode_last = device_mode;
        device_mode = (device_mode + 1) % DEVICE_MODE_ENUM_COUNT;
        break;
      }
    case KEY_PRESS_UP_LONG:
      {
        // 长按 开关灯
        led_state() ? led_off() : led_on();
        break;
      }
    case KEY_PRESS_UP_TOO_LONG:
      {
        // 检测到水位恢复
        device_state_last = device_state;
        device_state = DEVICE_STATE_WORK;  // 恢复正常工作状态
        break;
      }
    case KEY_PRESS_TOO_LONG:
      {
        // 检测到缺水
        device_state_last = device_state;
        device_state = DEVICE_STATE_NO_WATER;
      }
  }
}
void handler_state() {
  if (device_state == DEVICE_STATE_WORK) {
    switch (device_mode) {
      case DEVICE_MODE_AUTO: mode_auto(); break;
      case DEVICE_MODE_TIME_30_MIN: mode_time_30_min(); break;
      case DEVICE_MODE_TIME_60_MIN: mode_time_60_min(); break;
      case DEVICE_MODE_FREE: mode_free(); break;
    }
  }else if(device_state == DEVICE_STATE_NO_WATER){
    unsigned long ms = millis();
    if(ms%2000==0){
      led_on();
    }else if(ms%1000==0){
      led_off();
    }
  }else if(device_state == DEVICE_STATE_ERR){
    unsigned long ms = millis();
    if(ms%200==0){
      led_on();
    }else if(ms%100==0){
      led_off();
    }
  }
}
void handler_dht11(){
  static unsigned long ms_read = 0,ms_now = 0;
  ms_now = millis();
  if(ms_now % (10*1000) ==0 && ms_now!=ms_read && device_state !=DEVICE_STATE_ERR){
    ms_read = ms_now;
    if( dht11_read() == false ){
        device_state_last = device_state;
        device_state = DEVICE_STATE_ERR;
    }
  }
}


/*
* 模式
*/
void mode_free() {
  if (device_mode_last != device_mode) {
    //切换时
    device_mode_last = device_mode;
    pwm_off();

    led_off();
    delay(500);
    for (int i = 0; i < 1; i++) {
      led_on();
      delay(50);
      led_off();
      delay(50);
    }
    led_on();
  }

  //持续执行
  //什么都不做
}
void mode_time_30_min() {
  if (device_mode_last != device_mode) {
    //切换时
    device_mode_last = device_mode;
    pwm_off();

    led_off();
    delay(500);
    for (int i = 0; i < 3; i++) {  //闪灯3次
      led_on();
      delay(50);
      led_off();
      delay(50);
    }
    led_on();

    start_ms = millis();
    pwm_on();
  }

  //持续执行
  if (millis() - start_ms >= 30 * 60 * 1000) {
    pwm_off();
    device_mode_last = device_mode;
    device_mode = DEVICE_MODE_FREE;  //切回空闲模式
  }
}
void mode_time_60_min() {
  if (device_mode_last != device_mode) {
    //切换时
    device_mode_last = device_mode;
    pwm_off();

    led_off();
    delay(500);
    for (int i = 0; i < 5; i++) {  
      led_on();
      delay(50);
      led_off();
      delay(50);
    }
    led_on();

    start_ms = millis();
    pwm_on();
  }

  //持续执行
  if (millis() - start_ms >= 60 * 60 * 1000) {
    pwm_off();
    device_mode_last = device_mode;
    device_mode = DEVICE_MODE_FREE;  //切回空闲模式
  }
}
void mode_auto() {
  if (device_mode_last != device_mode) {
    //切换时
    device_mode_last = device_mode;
    pwm_off();

    led_off();
    delay(500);
    for (int i = 0; i < 7; i++) {  
      led_on();
      delay(50);
      led_off();
      delay(50);
    }
    led_on();
    start_ms = millis();
  }

  //持续执行
  //根据湿度来决定是否加湿
  if (dht11_status()) {
    int humi = dht11_get_humi();
    if (humi < 20 && pwm_state() == false) {
      pwm_on();
    } else if (humi > 40 && pwm_state() == true) {
      pwm_off();
    }
  } else {
    pwm_off();
  }
}

/*
* 网络处理
*/
void web_init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  server.on("/", HTTP_GET, handler_web_index);
  server.on("/state", HTTP_GET, handler_web_state);
  server.on("/mode/update", HTTP_POST, handler_web_mode_update);
  server.on("/led/on", HTTP_POST, handler_web_led_on);
  server.on("/led/off", HTTP_POST, handler_web_led_off);
  server.begin();

}
void handler_web_index() {
  server.send(200, "text/html; charset=UTF-8", String(index_html_text));
}
void handler_web_state() {
  String result = "";
  char str[200] = { 0 };
  sprintf(str, "state:%s \n mode:%s \n workTime:%ld \n modeTime:%ld \n led:%s \n pwm:%s \n dht11:%s \n dht11ErrStr:%s \n humi:%d \n temp:%d",
          state2str[device_state].c_str(),
          mode2str[device_mode].c_str(),
          millis()/ 1000,
          (millis() - start_ms) / 1000,
          String(led_state()),
          String(pwm_state()),
          String(dht11_status()).c_str(),
          dht11_get_err_string().c_str(),
          dht11_get_humi(),
          dht11_get_temp());

  result = String(str);
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/plain", result);
}
void handler_web_led_on() {
  led_on();
  server.send(200, "text/plain", "true");
}
void handler_web_led_off() {
  led_off();
  server.send(200, "text/plain", "true");
}
void handler_web_mode_update() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "mode") {
      String state = server.arg(i);
      if (state == "free") {
        device_mode_last = device_mode;
        device_mode = DEVICE_MODE_FREE;

      } else if (state == "time_30_min") {
        device_mode_last = device_mode;
        device_mode = DEVICE_MODE_TIME_30_MIN;

      } else if (state == "time_60_min") {
        device_mode_last = device_mode;
        device_mode = DEVICE_MODE_TIME_60_MIN;

      } else if (state == "auto") {
        device_mode_last = device_mode;
        device_mode = DEVICE_MODE_AUTO;

      } else {
        server.send(200, "text/plain", "false");
        return;
      }
      server.send(200, "text/plain", "true");
      return;
    }
  }
  server.send(200, "text/plain", "err");
}