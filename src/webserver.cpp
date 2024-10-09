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
    response->printf("<!DOCTYPE html><html><head><title>Webpage at %s</title>", request->url().c_str());
    response->print("<script>const sendDisco = (reg) => { fetch('/discovery?on='+reg, {method: 'POST'}).then(d => console.log(d)); };</script>");
    response->print("</head><body><h2>Hello ");
    response->print(request->client()->remoteIP());
    response->print("</h2>");
    response->print("<h3>General</h3>");
    response->print("<ul>");
    response->printf("<li>Version: HTTP/1.%u</li>", request->version());
    response->printf("<li>Method: %s</li>", request->methodToString());
    response->printf("<li>URL: %s</li>", request->url().c_str());
    response->printf("<li>Host: %s</li>", request->host().c_str());
    response->printf("<li>ContentType: %s</li>", request->contentType().c_str());
    response->printf("<li>ContentLength: %u</li>", request->contentLength());
    response->printf("<li>Multipart: %s</li>", request->multipart()?"true":"false");
    response->print("</ul>");
    response->print("<h3>Device</h3>");
    response->print("<ul>");
    response->print("<li>Version: " SENSOR_VERSION "</li>");
    response->printf("<li>Mac address: %s</li>", WiFi.macAddress().c_str());
    response->printf("<li>Id from mac address: %s</li>", getId());
    response->print("</ul>");
    response->print("<h3>MQTT</h3>");
    response->print("<ul>");
    response->printf("<li>Control topic: %s</li>", controlTopic().c_str());
    response->printf("<li>State topic pulses: %s</li>", stateTopicPulses().c_str());
    response->printf("<li>State topic m3: %s</li>", stateTopicM3().c_str());
    response->printf("<li>Log topic: %s</li>", logTopic().c_str());
    response->print("</ul>");
    response->print("<h3>Pulses</h3>");
    response->print("<ul>");
    response->printf("<li>Current number of pulses: %d</li>", getLiters());
    response->print("</ul>");
    response->print("<h3>Timestamps</h3>");
    response->print("<ul>");
    response->printf("<li>Last heartbeat: %s</li>", getLastHeartBeatTs().c_str());
    response->printf("<li>Last pulse: %s</li>", getLastPulseTs().c_str());
    response->printf("<li>Last state persist: %s</li>", getLastSavedTs().c_str());
    response->print("</ul>");
    response->print("<button onclick=\"sendDisco(true)\">Discovery ON</button>");
    response->print("<button onclick=\"sendDisco(false)\">Discovery OFF</button>");
    response->print("</body></html>");
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
