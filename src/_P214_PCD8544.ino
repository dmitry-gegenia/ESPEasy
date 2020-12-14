#include "_Plugin_Helper.h"

#ifdef USES_P214

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define PLUGIN_214
#define PLUGIN_ID_214     214           
#define PLUGIN_NAME_214   "Display - LCD PCD8544" 
#define PLUGIN_VALUENAME1_214 "Backlight"
#define PLUGIN_VALUENAME2_214 "Contrast" 
#define PLUGIN_VALUENAME3_214 "Rotation" 
#define PLUGIN_214_DEBUG  false        

#define P214_OUTPUT_TYPE_INDEX  2

Adafruit_PCD8544 *lcd2;
byte lines = 6;

boolean Plugin_214(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
   
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_214;                  
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;               
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE; 
      Device[deviceCount].Ports              = 0;                                
      Device[deviceCount].ValueCount         = 0;                                
      Device[deviceCount].PullUpOption       = false;                            
      Device[deviceCount].InverseLogicOption = false;                            
      Device[deviceCount].FormulaOption      = false;                            
      Device[deviceCount].SendDataOption     = false;                            
      Device[deviceCount].TimerOption        = false;                            
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_214);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_214));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_214));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_214));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Display Contrast(0-100)"), F("plugin_214_contrast"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);

      byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
      int optionValues[4] = { 0, 1, 2, 3 };
      String options[4] = { F("0"), F("90"), F("180"), F("270") };
      addFormSelector(F("Display Rotation"), F("plugin_214_rotation"), 4, options, optionValues, choice);

      char deviceTemplate [lines][48];
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
      for (byte varNr = 0; varNr < lines; varNr++)
      {
        addFormTextBox(String(F("Line ")) + (varNr + 1), String(F("Plugin_214_template")) + (varNr + 1), deviceTemplate[varNr], 80);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_214_rotation"));
      Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_214_contrast"));

      char deviceTemplate[lines][48];
      for (byte varNr = 0; varNr < lines; varNr++)
      {
        char argc[25];
        String arg = F("Plugin_214_template");
        arg += varNr + 1;
        arg.toCharArray(argc, 25);
        String tmpString = web_server.arg(argc);
        strncpy(deviceTemplate[varNr], tmpString.c_str(), sizeof(deviceTemplate[varNr]));
      }
      
      SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      if (!lcd2)
      {
        #if defined(ESP8266)
          lcd2 = new Adafruit_PCD8544(D0, D1, D2); 
        #endif
        #if defined(ESP32)
          lcd2 = new Adafruit_PCD8544(D0, D1, D2); 
        #endif
      }
        
      lcd2->begin();                     
      lcd2->setContrast(Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
      lcd2->setRotation(Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
      lcd2->clearDisplay();
      
      char deviceTemplate[lines][48];
      
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
      displayText(deviceTemplate);
      lcd2->display();
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      char deviceTemplate[lines][48];
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
      displayText(deviceTemplate);

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase(F("PCD8544")))
        {
          success = true;
          argIndex = string.lastIndexOf(',');
          tmpString = string.substring(argIndex + 1);
          lcd2->setCursor(event->Par2 - 1, event->Par1 - 1);
          lcd2->println(tmpString.c_str());
        }
        if (tmpString.equalsIgnoreCase(F("PCD8544CMD")))
        {
          success = true;
          argIndex = string.lastIndexOf(',');
          tmpString = string.substring(argIndex + 1);
          if (tmpString.equalsIgnoreCase(F("Clear")))
            lcd2->clearDisplay();
        }
        break;
    }
  } 
  return success;
}   

boolean displayText(char deviceTemplate[][48]) {
  String log = F("PCD8544: ");
  String string = F("");
  lcd2->clearDisplay();
  lcd2->setTextSize(1);
  lcd2->setTextColor(BLACK);
  lcd2->setCursor(0,0);
  for (byte x = 0; x < lines; x++)
  {
    String tmpString = deviceTemplate[x];
    if (tmpString.length()) 
    {
      String newString = parseTemplate(tmpString, 48);
      lcd2->setCursor(0,x*8);
      lcd2->print(newString);
      string+=newString+"\\";
    }
  }
  log += String(F("displayed text: \"")) + String(string) + String(F("\""));
  addLog(LOG_LEVEL_INFO, log);
  lcd2->display();
  return true;
}

#endif // USES_P101