#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "sensor.h"
#include "webserver.h"
#include "mqtt.h"
#include "persist.h"


AsyncWebServer server(80);


void setupWebServer() {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>");
    response->print("<title>Water Meter " SENSOR_VERSION "</title>");
    response->print("<style>");
    response->print("*{margin:0;padding:0;box-sizing:border-box}");
    response->print("body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:20px;min-height:100vh}");
    response->print(".container{max-width:800px;margin:0 auto}");
    response->print("h1{text-align:center;margin-bottom:30px;font-size:2em;text-shadow:2px 2px 4px rgba(0,0,0,0.3)}");
    response->print(".main-value{background:rgba(255,255,255,0.95);color:#667eea;padding:40px;border-radius:20px;text-align:center;margin-bottom:30px;box-shadow:0 8px 32px rgba(0,0,0,0.3)}");
    response->print(".main-value .label{font-size:1.2em;color:#666;margin-bottom:10px;font-weight:500}");
    response->print(".main-value .value{font-size:4em;font-weight:bold;color:#667eea;margin:10px 0}");
    response->print(".main-value .unit{font-size:2em;color:#764ba2;font-weight:600}");
    response->print(".main-value .subvalue{font-size:1em;color:#888;margin-top:15px}");
    response->print(".section{background:rgba(255,255,255,0.1);backdrop-filter:blur(10px);border-radius:15px;padding:20px;margin-bottom:20px;box-shadow:0 4px 16px rgba(0,0,0,0.2)}");
    response->print(".section h2{font-size:1.3em;margin-bottom:15px;padding-bottom:10px;border-bottom:2px solid rgba(255,255,255,0.3)}");
    response->print(".info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:10px}");
    response->print(".info-item{background:rgba(255,255,255,0.1);padding:12px;border-radius:8px}");
    response->print(".info-item .label{font-size:0.85em;color:rgba(255,255,255,0.7);margin-bottom:5px}");
    response->print(".info-item .value{font-size:1.1em;font-weight:600;word-break:break-all}");
    response->print(".buttons{display:flex;gap:15px;justify-content:center;margin-top:30px}");
    response->print("button{background:rgba(255,255,255,0.9);color:#667eea;border:none;padding:15px 30px;border-radius:10px;font-size:1em;font-weight:600;cursor:pointer;transition:all 0.3s;box-shadow:0 4px 12px rgba(0,0,0,0.2)}");
    response->print("button:hover{background:#fff;transform:translateY(-2px);box-shadow:0 6px 16px rgba(0,0,0,0.3)}");
    response->print("button:active{transform:translateY(0)}");
    response->print("button.danger{background:rgba(220,53,69,0.9);color:#fff}");
    response->print("button.danger:hover{background:rgba(220,53,69,1)}");
    response->print("@media(max-width:600px){.main-value .value{font-size:3em}.buttons{flex-direction:column}}");
    response->print("</style>");
    response->print("<script>const sendDisco = (reg) => { fetch('/discovery?on='+reg, {method: 'POST'}).then(d => console.log(d)); };</script>");
    response->print("</head><body><div class='container'>");
    response->print("<h1>💧 Water Meter " SENSOR_VERSION "</h1>");
    
    // Main value display
    float m3Value = (float)getLiters() / 1000.0;
    response->print("<div class='main-value'>");
    response->print("<div class='label'>Current Water Consumption</div>");
    response->printf("<div class='value'>%.3f</div>", m3Value);
    response->print("<div class='unit'>m³</div>");
    response->printf("<div class='subvalue'>%d liters</div>", getLiters());
    response->print("</div>");
    
    // Device Info Section
    response->print("<div class='section'>");
    response->print("<h2>📱 Device Information</h2>");
    response->print("<div class='info-grid'>");
    response->print("<div class='info-item'><div class='label'>Version</div><div class='value'>" SENSOR_VERSION "</div></div>");
    response->printf("<div class='info-item'><div class='label'>IP Address</div><div class='value'>%s</div></div>", WiFi.localIP().toString().c_str());
    response->printf("<div class='info-item'><div class='label'>MAC Address</div><div class='value'>%s</div></div>", WiFi.macAddress().c_str());
    response->printf("<div class='info-item'><div class='label'>Device ID</div><div class='value'>%s</div></div>", getId());
    response->print("</div></div>");
    
    // MQTT Configuration Section
    response->print("<div class='section'>");
    response->print("<h2>📡 MQTT Configuration</h2>");
    response->print("<div class='info-grid'>");
    response->printf("<div class='info-item'><div class='label'>Control Topic</div><div class='value'>%s</div></div>", controlTopic().c_str());
    response->printf("<div class='info-item'><div class='label'>State Topic (Pulses)</div><div class='value'>%s</div></div>", stateTopicPulses().c_str());
    response->printf("<div class='info-item'><div class='label'>State Topic (m³)</div><div class='value'>%s</div></div>", stateTopicM3().c_str());
    response->printf("<div class='info-item'><div class='label'>Log Topic</div><div class='value'>%s</div></div>", logTopic().c_str());
    response->print("</div></div>");
    
    // Timestamps Section
    response->print("<div class='section'>");
    response->print("<h2>🕐 Status Timestamps</h2>");
    response->print("<div class='info-grid'>");
    response->printf("<div class='info-item'><div class='label'>Last Heartbeat</div><div class='value'>%s</div></div>", getLastHeartBeatTs().c_str());
    response->printf("<div class='info-item'><div class='label'>Last Pulse</div><div class='value'>%s</div></div>", getLastPulseTs().c_str());
    response->printf("<div class='info-item'><div class='label'>Last State Persist</div><div class='value'>%s</div></div>", getLastSavedTs().c_str());
    response->print("</div></div>");
    
    // Discovery Buttons
    response->print("<div class='buttons'>");
    response->print("<button onclick=\"sendDisco(true)\">🔍 Send Discovery</button>");
    response->print("<button class='danger' onclick=\"sendDisco(false)\">🗑️ Remove Discovery</button>");
    response->print("</div>");
    
    response->print("</div></body></html>");
    //send the response last
    request->send(response);
  });

  server.on("/discovery", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;
    if (request->hasParam("on")) {
        message = request->getParam("on")->value();
        int isOn = message.compareTo("true");
        sendDiscovery(!isOn);
    } else {
        message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.begin();

}
