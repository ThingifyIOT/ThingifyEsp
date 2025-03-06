# ThingifyEsp

How to start:
 - For ESP8266 - clone https://github.com/ThingifyIOT/Esp8266Template
 - For ESP32 - clone https://github.com/ThingifyIOT/Esp32Template
 - Download thingify IOT app
![image](https://github.com/user-attachments/assets/e85121e4-6f22-4a2c-a120-a6e43dd5023e)


How to setup IOT project for ESP32/ESP8266:
 - Install vscode with platformio
 - Create new platformio project for esp32/esp8266/esp8285
 - In project /lib directory execute: 
     ````
     git submodule add git@github.com:ThingifyIOT/ThingifyEsp.git
     git submodule add git@github.com:ThingifyIOT/ThingifyEspModules.git
     git submodule add git@github.com:toomasz/FixedString.git
  - In src/main.cpp paste:
   ```c++
#include <Arduino.h>
#include <ThingifyEsp.h>

ThingifyEsp thing("Sample Thing");
Node* switchNode;
bool previousSwitchState = false;
const int RelayPin = 4;
const int SwitchPin = 5;

bool OnBoolChanged(void*_, Node *node)
{
  digitalWrite(RelayPin, node->Value.AsBool());
  return true;
}

void setup()
{
  Serial.begin(500000);	
  thing.AddDiagnostics();

  pinMode(SwitchPin, INPUT);
  pinMode(RelayPin, OUTPUT);

  switchNode = thing.AddBoolean("Switch")->OnChanged(OnBoolChanged);
  thing.Start();
}

void loop()
{
  auto switchState = digitalRead(SwitchPin);
  if(switchState != previousSwitchState)
  {
    switchNode->SetValue(NodeValue::Boolean(!switchNode->Value.AsBool()));
  }
  previousSwitchState = switchState;

  thing.Loop();
}


```

![command line dumpo](https://i.imgur.com/dNpemKh.png)


