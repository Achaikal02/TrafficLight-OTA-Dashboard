#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

const char* host = "esp32";
const char* ssid = "JURAGAN SOEDARTO LT1C";
const char* password = "JURAGAN73";

// Traffic Light 1
#define TL1_RED     25
#define TL1_YELLOW  26
#define TL1_GREEN   27

// Traffic Light 2
#define TL2_RED     33
#define TL2_YELLOW  32
#define TL2_GREEN   13

// Traffic Light 3
#define TL3_RED     23
#define TL3_YELLOW  22
#define TL3_GREEN   12

// Traffic Light 4
#define TL4_RED     18
#define TL4_YELLOW  19
#define TL4_GREEN   21

// Static IP
IPAddress local_IP(10, 122, 151, 50);
IPAddress gateway(10, 122, 151, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

WebServer server(80);

// Login page
const char* loginIndex =
 "<form name='loginForm'>"
 "<table width='20%' bgcolor='A09F9F' align='center'>"
 "<tr><td colspan=2><center><h2>ESP32 Login</h2></center></td></tr>"
 "<tr><td>Username:</td><td><input type='text' name='userid'></td></tr>"
 "<tr><td>Password:</td><td><input type='password' name='pwd'></td></tr>"
 "<tr><td colspan=2><input type='submit' onclick='check(this.form)' value='Login'></td></tr>"
 "</table>"
 "</form>"

 "<script>"
 "function check(form){"
 "if(form.userid.value=='admin' && form.pwd.value=='admin'){"
 "window.open('/serverIndex')"
 "} else {"
 "alert('Username atau Password salah')"
 "}"
 "}"
 "</script>";

// Upload page
const char* serverIndex =
 "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
 "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
 "<input type='file' name='update'>"
 "<input type='submit' value='Update'>"
 "</form>"
 "<div id='prg'>Progress: 0%</div>"

 "<script>"
 "$('form').submit(function(e){"
 "e.preventDefault();"
 "var form = $('#upload_form')[0];"
 "var data = new FormData(form);"

 "$.ajax({"
 "url: '/update',"
 "type: 'POST',"
 "data: data,"
 "contentType: false,"
 "processData:false,"
 "xhr: function() {"
 "var xhr = new window.XMLHttpRequest();"
 "xhr.upload.addEventListener('progress', function(evt) {"
 "if (evt.lengthComputable) {"
 "var per = evt.loaded / evt.total;"
 "$('#prg').html('Progress: ' + Math.round(per * 100) + '%');"
 "}"
 "}, false);"
 "return xhr;"
 "},"
 "success:function(d, s) {"
 "alert('Update berhasil. ESP32 akan restart');"
 "},"
 "error: function(a, b, c) {"
 "alert('Update gagal');"
 "}"
 "});"
 "});"
 "</script>";

void setup() {
  Serial.begin(9600);

  pinMode(TL1_RED, OUTPUT);
  pinMode(TL1_YELLOW, OUTPUT);
  pinMode(TL1_GREEN, OUTPUT);

  pinMode(TL2_RED, OUTPUT);
  pinMode(TL2_YELLOW, OUTPUT);
  pinMode(TL2_GREEN, OUTPUT);

  pinMode(TL3_RED, OUTPUT);
  pinMode(TL3_YELLOW, OUTPUT);
  pinMode(TL3_GREEN, OUTPUT);

  pinMode(TL4_RED, OUTPUT);
  pinMode(TL4_YELLOW, OUTPUT);
  pinMode(TL4_GREEN, OUTPUT);

// if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
//   Serial.println("Gagal mengatur Static IP");
// }

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(host)) {
    Serial.println("mDNS started");
    Serial.print("Open in browser: http://");
    Serial.print(host);
    Serial.println(".local");
  }

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });

  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    delay(1000);
    ESP.restart();
  }, []() {

    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update Start: %s\n", upload.filename.c_str());

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }

    } else if (upload.status == UPLOAD_FILE_WRITE) {

      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }

    } else if (upload.status == UPLOAD_FILE_END) {

      if (Update.end(true)) {
        Serial.printf("Update Success: %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Jalur 1 dan 3 hijau
  digitalWrite(TL1_RED, LOW);
  digitalWrite(TL1_YELLOW, LOW);
  digitalWrite(TL1_GREEN, HIGH);

  digitalWrite(TL2_RED, HIGH);
  digitalWrite(TL2_YELLOW, LOW);
  digitalWrite(TL2_GREEN, LOW);

  digitalWrite(TL3_RED, LOW);
  digitalWrite(TL3_YELLOW, LOW);
  digitalWrite(TL3_GREEN, HIGH);

  digitalWrite(TL4_RED, HIGH);
  digitalWrite(TL4_YELLOW, LOW);
  digitalWrite(TL4_GREEN, LOW);

  delay(2000);

  // Jalur 1 dan 3 kuning
  digitalWrite(TL1_GREEN, LOW);
  digitalWrite(TL1_YELLOW, HIGH);

  digitalWrite(TL3_GREEN, LOW);
  digitalWrite(TL3_YELLOW, HIGH);

  delay(1000);

  // Jalur 2 dan 4 hijau
  digitalWrite(TL1_RED, HIGH);
  digitalWrite(TL1_YELLOW, LOW);

  digitalWrite(TL2_RED, LOW);
  digitalWrite(TL2_GREEN, HIGH);

  digitalWrite(TL3_RED, HIGH);
  digitalWrite(TL3_YELLOW, LOW);

  digitalWrite(TL4_RED, LOW);
  digitalWrite(TL4_GREEN, HIGH);

  delay(2000);

  // Jalur 2 dan 4 kuning
  digitalWrite(TL2_GREEN, LOW);
  digitalWrite(TL2_YELLOW, HIGH);

  digitalWrite(TL4_GREEN, LOW);
  digitalWrite(TL4_YELLOW, HIGH);

  delay(1000);
}