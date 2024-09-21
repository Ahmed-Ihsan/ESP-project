#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

// Configuration
const char* WIFI_SSID = "Redmi_E642";
const char* WIFI_PASSWORD = "123456789";
const char* AUTH_USERNAME = "admin";
const char* AUTH_PASSWORD = "password";
const char* MDNS_NAME = "espcar";

// Motor pins
const int MOTOR_A_PIN1 = D2;
const int MOTOR_A_PIN2 = D3;
const int MOTOR_B_PIN1 = D5;
const int MOTOR_B_PIN2 = D6;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// HTML, CSS, and JavaScript content
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP8266 Car Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, Helvetica, sans-serif;
            text-align: center;
            margin: 0px auto;
            padding-top: 30px;
            max-width: 400px;
        }
        .button {
            padding: 15px 25px;
            font-size: 24px;
            text-align: center;
            outline: none;
            color: #fff;
            background-color: #2196F3;
            border: none;
            border-radius: 5px;
            box-shadow: 0 6px #999;
            cursor: pointer;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            -khtml-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
            -webkit-tap-highlight-color: rgba(0,0,0,0);
        }
        .button:hover {background-color: #0b7dda}
        .button:active {
            background-color: #0b7dda;
            box-shadow: 0 2px #666;
            transform: translateY(4px);
        }
        .control-pad {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            margin: 20px auto;
            max-width: 300px;
        }
        #status { margin-top: 20px; }
    </style>
</head>
<body>
    <h1>ESP8266 Car Control</h1>
    <div class="control-pad">
        <div></div>
        <button class="button" id="forward">↑</button>
        <div></div>
        <button class="button" id="left">←</button>
        <button class="button" id="stop">■</button>
        <button class="button" id="right">→</button>
        <div></div>
        <button class="button" id="backward">↓</button>
        <div></div>
    </div>
    <div id="status"></div>
    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        window.addEventListener('load', onLoad);

        function initWebSocket() {
            console.log('Trying to open a WebSocket connection...');
            websocket = new WebSocket(gateway);
            websocket.onopen    = onOpen;
            websocket.onclose   = onClose;
            websocket.onmessage = onMessage;
        }

        function onOpen(event) {
            console.log('Connection opened');
            document.getElementById("status").innerHTML = "Connected";
        }

        function onClose(event) {
            console.log('Connection closed');
            document.getElementById("status").innerHTML = "Disconnected";
            setTimeout(initWebSocket, 2000);
        }

        function onMessage(event) {
            console.log('Received: ' + event.data);
        }

        function onLoad(event) {
            initWebSocket();
            initButtons();
        }

        function initButtons() {
            document.getElementById('forward').addEventListener('mousedown', function(){ sendCommand('forward'); });
            document.getElementById('backward').addEventListener('mousedown', function(){ sendCommand('backward'); });
            document.getElementById('left').addEventListener('mousedown', function(){ sendCommand('left'); });
            document.getElementById('right').addEventListener('mousedown', function(){ sendCommand('right'); });
            document.getElementById('stop').addEventListener('mousedown', function(){ sendCommand('stop'); });

            ['forward', 'backward', 'left', 'right'].forEach(function(dir) {
                document.getElementById(dir).addEventListener('mouseup', function(){ sendCommand('stop'); });
                document.getElementById(dir).addEventListener('mouseleave', function(){ sendCommand('stop'); });
            });
        }

        function sendCommand(command) {
            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(JSON.stringify({action: command}));
            } else {
                console.log('WebSocket not connected');
            }
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  
  pinMode(MOTOR_A_PIN1, OUTPUT);
  pinMode(MOTOR_A_PIN2, OUTPUT);
  pinMode(MOTOR_B_PIN1, OUTPUT);
  pinMode(MOTOR_B_PIN2, OUTPUT);

  stopMotors();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(MDNS_NAME)) {
    Serial.println("MDNS responder started");
  }

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(AUTH_USERNAME, AUTH_PASSWORD))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
  });

  server.begin();

  ArduinoOTA.setHostname(MDNS_NAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  MDNS.update();
  ws.cleanupClients();
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.println("WebSocket client connected");
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("WebSocket client disconnected");
  } else if(type == WS_EVT_DATA){
    handleWebSocketMessage(arg, data, len);
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    String action = doc["action"];
    
    if(action == "forward") moveForward();
    else if(action == "backward") moveBackward();
    else if(action == "left") turnLeft();
    else if(action == "right") turnRight();
    else if(action == "stop") stopMotors();
  }
}

void moveForward() {
  Serial.println("MDNS responder started");
  digitalWrite(MOTOR_A_PIN1, HIGH);
  digitalWrite(MOTOR_A_PIN2, LOW);
  digitalWrite(MOTOR_B_PIN1, HIGH);
  digitalWrite(MOTOR_B_PIN2, LOW);
}

void moveBackward() {
  Serial.println("MDNS responder started");
  digitalWrite(MOTOR_A_PIN1, LOW);
  digitalWrite(MOTOR_A_PIN2, HIGH);
  digitalWrite(MOTOR_B_PIN1, LOW);
  digitalWrite(MOTOR_B_PIN2, HIGH);
}

void turnLeft() {
  Serial.println("MDNS responder started");
  digitalWrite(MOTOR_A_PIN1, LOW);
  digitalWrite(MOTOR_A_PIN2, LOW);
  digitalWrite(MOTOR_B_PIN1, HIGH);
  digitalWrite(MOTOR_B_PIN2, LOW);
}

void turnRight() {
  Serial.println("MDNS responder started");
  digitalWrite(MOTOR_A_PIN1, HIGH);
  digitalWrite(MOTOR_A_PIN2, LOW);
  digitalWrite(MOTOR_B_PIN1, LOW);
  digitalWrite(MOTOR_B_PIN2, LOW);
}

void stopMotors() {
  Serial.println("MDNS responder started");
  digitalWrite(MOTOR_A_PIN1, LOW);
  digitalWrite(MOTOR_A_PIN2, LOW);
  digitalWrite(MOTOR_B_PIN1, LOW);
  digitalWrite(MOTOR_B_PIN2, LOW);
}
