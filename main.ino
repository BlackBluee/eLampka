#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN        12
#define NUMPIXELS 16

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

ESP8266WebServer server(80);

struct settings {
  char ssid[30];
  char password[30];
} user_wifi = {};

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels.begin();

  Serial.begin(115200);
  EEPROM.begin(sizeof(struct settings) );
  EEPROM.get( 0, user_wifi );
  
  Serial.print("łączenie z siecią wifi"); 
  WiFi.hostname("elampka.home");  
  WiFi.mode(WIFI_STA);
  WiFi.begin(user_wifi.ssid, user_wifi.password);
  
  byte tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");   
    if (tries++ > 30) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("lampka", "elizkakochana");
      break;
    }
  }
  Serial.println("");
  Serial.print("połączono z: ");
  Serial.print(user_wifi.ssid);
  Serial.println("");

  server.on("/setColor", HTTP_POST, handleSetColor);
  server.on("/color",  handlePortal);
  server.on("/settings",  handle_settings);



  server.begin();
  Serial.println("HTTP Serwer został uruchomiony");
  Serial.print("Użyj URL żeby się połączyć: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  server.handleClient();
  
  }

void handleSetColor() {
  String animation = server.arg("animation");
  String rainbow = server.arg("rainbow");
  String colorString = server.arg("color");
  uint8_t brightness = server.arg("brightness").toInt();
  uint8_t atime = server.arg("time").toInt();
  
  

  long number = (long) strtol( &colorString[1], NULL, 16);
  int r = number >> 16;
  int g = number >> 8 & 0xFF;
  int b = number & 0xFF;

  if(animation == "true"){
    for (int i = 0; i < NUMPIXELS; i++) {
      // Ustawienie koloru dla każdego piksela
      pixels.setPixelColor(i, pixels.Color(r, g, b));
      pixels.show();
      delay(atime);
    }
    Serial.println("wykonano");    
  } else{
    static bool isRainbow = false;

    if (rainbow == "true" && !isRainbow) {
      isRainbow = true;
      rainbowCycle(30);
    } else if (rainbow == "false" && isRainbow) {
      isRainbow = false;
      // Clear the LED strip
      for (int i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 0);
      }
      pixels.show();
    } else {
      for (int i = 0; i < NUMPIXELS; i++) {
        // Ustawienie koloru dla każdego piksela
        pixels.setPixelColor(i, pixels.Color(r, g, b));
      }
    }
  }

  pixels.setBrightness(brightness);
  pixels.show();
  server.send(200, "text/plain", "OK");
}



void handlePortal() {

    
server.send(200, "text/html", updateWebpage());
}


void handle_settings() {

  if (server.method() == HTTP_POST) {

    strncpy(user_wifi.ssid,     server.arg("ssid").c_str(),     sizeof(user_wifi.ssid) );
    strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password) );
    user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = '\0';
    EEPROM.put(0, user_wifi);
    EEPROM.commit();

    server.send(200,   "text/html",  "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Ustawienia połączenia z siecią WiFi</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Ustawienia połączenia z siecią WiFi</h1> <br/> <p>Ustawienia zostały zapisane pomyślnie!<br />Proszę uruchomić urządzenie ponownie.</p></main></body></html>" );
  } else {

    server.send(200,   "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Ustawienia połączenia z siecią WiFi</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/settings' method='post'> <h1 class=''>Ustawienia połączenia z siecią WiFi</h1><br/><div class='form-floating'><label>Nazwa sieci</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br/><label>Hasło</label><input type='password' class='form-control' name='password'></div><br/><br/><button type='submit'>Zapisz</button><p style='text-align: right'></p></form></main> </body></html>" );
  }
}

void rainbowCycle(uint8_t wait) {
   // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 10 * 65536; firstPixelHue += 256)
    {
        for (int i = 0; i < pixels.numPixels(); i++)
        { // For each pixel in strip...
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through strip.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
        }
        pixels.show(); // Update strip with new contents
        delay(wait);  // Pause for a moment
  }}



String updateWebpage(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Inteligentna lampka</title>\n";
  ptr +="<style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{min-height: 100vh;display: flex;flex-direction: column;} footer{margin-top: auto;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 10px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 15px;}\n";
  ptr +=".sbutton {display: block;width: 150px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 20px;margin: 0px auto 35px;cursor: pointer;border-radius: 15px;}";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #3498db;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +=".color-picker {-webkit-appearance: none;-moz-appearance: none;appearance: none;width: 200px;height: 200px;background-color: transparent;border: none;cursor: pointer;}";
  ptr +=".color-picker::-webkit-color-swatch {border-radius: 15px;border: none;}";
  ptr +=".color-picker::-moz-color-swatch {border-radius: 15px;border: none;}";
  ptr +=".cslider {width: 50%;}";
  ptr +="#toggle-button {width: 16px; height: 16px;}";
  ptr +=".centre {position: absolute;top: 0;left: 0;bottom: 0;right: 0;margin: auto;transform: translate(-50%, -50%);}";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<header><h1>Inteligentna lampka💡</h1></header>\n";
  ptr +="<div style=\"centre\">";
  ptr +="<h2>Animacja</h2>";
  ptr +="<h3>Tryb rainbow<input type=\"checkbox\" id=\"roggle-button\"></h3>";
  ptr +="<input type=\"checkbox\" id=\"toggle-button\">";
  ptr +="<input type=\"range\" class=\"cslider\" id=\"timeSlider\" min=\"10\" max=\"1000\" value=\"300\"><br>";
  ptr +="<h2>Wybór koloru</h2>";
  ptr +="<input type=\"color\" class=\"color-picker\" id=\"colorPicker\"><br>";
  ptr +="<h2>Jasność</h2>";
  ptr +="<input type=\"range\" class=\"cslider\" id=\"brightnessSlider\" min=\"0\" max=\"255\" value=\"255\"><br><br><br>";
  ptr +="<button onclick=\"sendData()\" class=\"sbutton\">Ustaw</button>";
  ptr +="</div>";
  ptr +="<script>";
  ptr +="function sendData() {";
  ptr +="console.log('sendData() called');";
  ptr +="var xhttp = new XMLHttpRequest();";
  ptr +="xhttp.onreadystatechange = function() {";
  ptr +="if (this.readyState == 4 && this.status == 200) {";
  ptr +="console.log(this.responseText);}};";
  ptr +="xhttp.open(\"POST\", \"/setColor\", true);";
  ptr +="xhttp.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\");";
  ptr +="var animation = document.getElementById(\"toggle-button\").checked;";
  ptr +="var rainbow = document.getElementById(\"roggle-button\").checked;";
  ptr +="var time = document.getElementById(\"timeSlider\").value;";
  ptr +="var color = document.getElementById(\"colorPicker\").value;";
  ptr +="var brightness = document.getElementById(\"brightnessSlider\").value;";
  ptr +="localStorage.setItem(\"color\", color);";
  ptr +="localStorage.setItem(\"time\", time);";
  ptr +="localStorage.setItem(\"brightness\", brightness);";
  ptr +="localStorage.setItem(\"animation\", animation);";
  ptr +="localStorage.setItem(\"rainbow\", rainbow);";
  ptr +="var params = \"animation=\" + encodeURIComponent(animation) + \"&rainbow=\" + encodeURIComponent(rainbow) + \"&time=\" + encodeURIComponent(time) + \"&color=\" + encodeURIComponent(color) + \"&brightness=\" + encodeURIComponent(brightness);";
  ptr +="xhttp.send(params);}";
  ptr +="window.onload = function() {";
  ptr +="var color = localStorage.getItem(\"color\");";
  ptr +="var animation = localStorage.getItem(\"animation\");";
  ptr +="var rainbow = localStorage.getItem(\"rainbow\");";
  ptr +="var brightness = localStorage.getItem(\"brightness\");";
  ptr +="var time = localStorage.getItem(\"time\");";
  ptr +="document.getElementById(\"colorPicker\").value = color;";
  ptr +="document.getElementById(\"brightnessSlider\").value = brightness;";
  ptr +="document.getElementById(\"timeSlider\").value = time;";
  ptr +="document.getElementById(\"toggle-button\").checked = (animation === \"true\");";
  ptr +="document.getElementById(\"roggle-button\").checked = (rainbow === \"true\");";
  ptr +="}";
  ptr +="</script>";
  ptr +="<footer>...</footer>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
