#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <FS.h>

#define DEBUG 1

DNSServer dnsServer;
AsyncWebServer server(80);

File f;
String machine_code ="" ;
String address ="" ;
void setup(){
#if DEBUG
  Serial.begin(9600);
#endif  
  WiFi.softAP("esp-captive");
  SPIFFS.begin();
  if (SPIFFS.exists("/info/config.txt")){
    f=SPIFFS.open("/info/config.txt","r+"); 
    Serial.println("exist");
    char* read;
    //get machine code and address from file
    machine_code= f.readStringUntil(',');
#if DEBUG
    Serial.println(machine_code);
#endif
    address= f.readStringUntil(',');
#if DEBUG
    Serial.println(address);
#endif 
  } else
  {
    //if file doesn't exist, creat a new file
    f=SPIFFS.open("/info/config.txt","w+");
  }
  
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
     AsyncResponseStream *response = request->beginResponseStream("text/html");
     // render web 
    response->printf("<!DOCTYPE html><html lang=\"en\"><head><meta name=\"format-detection\" content=\"telephone=no\" /><meta charset=\"UTF-8\" /><meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\" /><title>Config ESP</title> <script>function c(l){document.getElementById(\"s\").value=l.innerText||l.textContent;p=l.nextElementSibling.classList.contains(\"l\");document.getElementById(\"a\").value=l.innerText||l.textContent;p=l.nextElementSibling.classList.contains(\"l\");document.getElementById(\"p\").disabled=!p;if(p)document.getElementById(\"p\").focus();}</script> <style>.c,body\{text-align:center;font-family:verdana}div,input{padding:5px;font-size:1em;margin:5px 0;box-sizing:border-box}button,input[type=\"button\"],input[type=\"submit\"]{cursor:pointer;border:0;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%}input[type=\"file\"]{border:1px solid #1fa3ec}.wrap{text-align:left;display:inline-block;min-width:260px;max-width:500px}input:disabled{opacity:0.5}</style></head><body class=\"{c}\"><div class=\"wrap\"><form method=\"POST\" action=\"wifisave\"> <label for=\"s\">Mã máy</label><input id=\"s\" name=\"s\" value=\"%s\" autocorrect=\"off\" autocapitalize=\"none\" placeholder=\"\" /><br /> <label for=\"a\">Địa điểm</label><input id=\"a\" name=\"a\" autocorrect=\"off\" autocapitalize=\"none\" value=\"%s\" placeholder=\"\" /><br /> <button type=\"submit\">Save</button></form></div></body></html>",machine_code.c_str(),address.c_str()); 
    request->send(response);
  });
  server.on("/wifisave", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;  
        if (request->hasParam("s", true) &&request->hasParam("a", true) ) {     
          String  number = request->getParam("s", true)->value();
          String  addr   = request->getParam("a",true)->value();
        #if DEBUG
            Serial.println(number);
            Serial.println(machine_code);
            
        #endif 
        // if the values have changed
        if (!number.equals(machine_code)&&!address.equals(addr)){
              f.close();
              f.truncate(0) ;
              f=SPIFFS.open("/info/config.txt","w+");
              f.print(number);
              f.print(',');
              f.print(addr);
              f.print(',');
              f.close();
        #if DEBUG
              Serial.println("saved");
        #endif
              f=SPIFFS.open("/info/config.txt","r+");
        #if DEBUG
              Serial.println(f.readString());
        #endif
              f.close();
              message="success";
              SPIFFS.end();
              server.end();
          } else {
            message="same info";
          #if DEBUG
            Serial.println("the same info");
          #endif
            f.close();
            SPIFFS.end();
            server.end();
          }
        } else {
            message = "No message sent";
        }
        
        request->send(200, "text/plain", "status " + message);
    });
  
  server.begin();
}

void loop(){
  dnsServer.processNextRequest();
}