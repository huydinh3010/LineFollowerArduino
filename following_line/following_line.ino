#define IR_1 14
#define IR_2 12
#define IR_3 13
#define IR_4 5
#define IR_5 4
#define ML1 15
#define ML2 16
#define MR1 2
#define MR2 0
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Arduino.h"

// Self-control
bool selfControl = false;

// cac thong so se hieu chinh
int Kp = 200; 
int Ki = 10;
int Kd = 80;
int lbase_speed = 800;
int rbase_speed = 850;
//

// Ket noi wifi dien thoai
const char* ssid = "vuson";
const char* password = "29121998";
//

//HTML
const char index_html[] PROGMEM = ""
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"    <title>Line Follower Robot</title>"
"</head>"
"<body>"
"    <h2>Line Follower Robot</h2>"
"    <div>Self-control   <input type=\"checkbox\" id=\"led\" name=\"led\"></div>"
"    <form>"
"        Kp:<br>"
"        <input type=\"text\" id=\"kp\" value=\"200\" disabled=\"true\">"
"        <br>"
"        <br>"
""
"        Ki:<br>"
"        <input type=\"text\" id=\"ki\" value=\"10\" disabled=\"true\">"
"        <br>"
"        <br>"
""
"        Kd:<br>"
"        <input type=\"text\" id=\"kd\" value=\"80\" disabled=\"true\">"
"        <br>"
"        <br>"
""
"        left base speed:<br>"
"        <input type=\"text\" id=\"lbs\" value=\"800\" disabled=\"true\">"
"        <br>"
"        <br>"
""
"        right base speed:<br>"
"        <input type=\"text\" id=\"rbs\" value=\"850\" disabled=\"true\">"
"        <br>"
"        <br>"
""
"        <input type=\"button\" id=\"submit\" value=\"Submit\">"
"        <br>"
"    </form> "
""
"    <p>If you click the \"Submit\" button, the data will be sent to the robot.</p>"
""
"    <script type=\"text/javascript\">"
"        var led = document.getElementById('led');"
"        var kp = document.getElementById('kp');"
"        var ki = document.getElementById('ki');"
"        var kd = document.getElementById('kd');"
"        var lbs = document.getElementById('lbs');"
"        var rbs = document.getElementById('rbs');"
"        var submit = document.getElementById('submit');"
"        var url = window.location.host;"
"        var ws = new WebSocket('ws://' + url + '/ws');"
""
"        ws.onopen = function()"
"        {"
"            console.log(\"socket connected\")"
"        };"
""
"        ws.onmessage = function(evt)"
"        {"
"        };"
"        ws.onclose = function() {"
"            console.log(\"socket disconnected\")"
"        };"
"        submit.onclick = function() {"
"            var value ;"
"            value = \"\" + kp.value + \"|\" + ki.value + \"|\" + kd.value + \"|\" + lbs.value + \"|\" + rbs.value ;"
"            ws.send(value); "
"            console.log(value);"            
"        };"            
"        led.onchange = function() {"
"            var led_status = 'LED_OFF';"
"            if (led.checked) {"
"                led_status = 'LED_ON';"
"                kp.disabled = false; "
"                kd.disabled = false; "
"                ki.disabled = false; "
"                lbs.disabled = false; "
"                rbs.disabled = false; "
"            }"
"            else{"
"                kp.disabled = true; "
"                kd.disabled = true; "
"                ki.disabled = true; "
"                lbs.disabled = true; "
"                rbs.disabled = true; "
"            }"
"            console.log(led_status);"
"            ws.send(led_status);"
"        }"
"    </script>"
"</body>"
"</html>";
//

//Khai bao web server
AsyncWebServer server(8000);
AsyncWebSocket ws("/ws");
//

// Hàm xử lí sự kiện trên Server khi client là browser phát sự kiện
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA && len > 0) { // type: loại sự kiện mà server nhận được. Nếu sự kiện nhận được là từ websocket thì bắt đầu xử lí
    data[len] = 0;
    String data_str = String((char*)data); // ép kiểu, đổi từ kiểu char sang String
    if (data_str == "LED_ON") {
      //digitalWrite(LED, 0); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
      selfControl = true;
      //Stop robot to config
      lbase_speed = 0;
      rbase_speed = 0;
      Serial.println("SELF CONTROL MODE");
      Serial.println("--------------");
    } else if (data_str == "LED_OFF") {
      //digitalWrite(LED, 1); // Khi client phát sự kiện "LED_OFF" thì server sẽ tắt LED
      selfControl = false;
      Serial.println("LINE FOLLOWING MODE");
      Serial.println("--------------");
    } else {
        //Convert data_str --> kp ki kd
        char *buf = (char*)data;
        char *p = buf;
        p[len] = 0;
        char *str;
        int result_int[5];
        int i = 0;
        int j;
        while ((str = strtok_r(p, "|", &p)) != NULL) // delimiter is the semicolon
        {  
          String toString = String(str);
          int result = (int)toString.toInt();
          result_int[i] = result;
          i = i + 1;
        }
        Kp = result_int[0];
        Serial.print("Kp: ");
        Serial.println(Kp);
        Ki = result_int[1];
        Serial.print("Ki: ");
        Serial.println(Ki);
        Kd = result_int[2];
        Serial.print("Kd: ");
        Serial.println(Kd);
        lbase_speed = result_int[3];
        Serial.print("Left base speed: ");
        Serial.println(lbase_speed);
        rbase_speed = result_int[4];
        Serial.print("Right base speed: ");
        Serial.println(rbase_speed);
        Serial.println("--------------");
    }
  }
}

int error = 0;
int lastError = 0;
int out = 0;
int P;
int I = 0;
int D;

int computePID(){                                                    
		// P
		P = error;
    out = P*Kp;
    I = I + error;
    D = error - lastError;
    out = Kp*P + Kd*D + Ki*I;                      
    if(error == 0) I = 0;                                  
    return out;                                        
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(IR_1, INPUT);
  pinMode(IR_2, INPUT);
  pinMode(IR_3, INPUT);
  pinMode(IR_4, INPUT);
  pinMode(IR_5, INPUT);
  pinMode(ML1, OUTPUT);
  pinMode(ML2, OUTPUT);
  pinMode(MR1, OUTPUT);
  pinMode(MR2, OUTPUT);
  analogWrite(ML2, 0);
  analogWrite(MR2, 0);
  analogWrite(ML1, lbase_speed);
  analogWrite(MR1, rbase_speed);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }
  ws.onEvent(onWsEvent); // gọi hàm onWsEvent
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {

    request->send_P(200, "text/html", index_html); // trả về file index.html trên giao diện browser khi browser truy cập vào IP của server
  });
    server.begin(); // khởi động server
  Serial.print("IP Address is :");
  Serial.println(WiFi.localIP());    
}

void readSensor(){
  int sensor[5];
  sensor[0] = digitalRead(IR_1);
  sensor[1] = digitalRead(IR_2);
  sensor[2] = digitalRead(IR_3);
  sensor[3] = digitalRead(IR_4);
  sensor[4] = digitalRead(IR_5);
  if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==0))
  error=4;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==0)&&(sensor[4]==0))
  error=3;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==0)&&(sensor[4]==1))
  error=1;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==0)&&(sensor[3]==0)&&(sensor[4]==1))
  error=1;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==0)&&(sensor[3]==1)&&(sensor[4]==1))
  error=0;
  else if((sensor[0]==1)&&(sensor[1]==0)&&(sensor[2]==0)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-1;
  else if((sensor[0]==1)&&(sensor[1]==0)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-1;
  else if((sensor[0]==0)&&(sensor[1]==0)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-3;
  else if((sensor[0]==0)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-4;
}

void controlMotor(int inp){
  int right_speed = rbase_speed - inp;
  int left_speed = lbase_speed + inp;
  right_speed = constrain(right_speed, 0, 1023);
  left_speed = constrain(left_speed, 0, 1023);
  analogWrite(ML1, left_speed);
  analogWrite(MR1, right_speed);
}

void loop() {
  if (selfControl == false){
    readSensor();
    if(lastError != error) controlMotor(computePID());
    lastError = error;
  }
  else{
    // More functions can be developed later ...
  }         
}
