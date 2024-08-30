# OFM-ShutterControllerModule

Diese Module dient zur Jalousiensteuerung.

## Anwendungsbeschreibung

Die Anwendungsbeschreibung ist unter [doc/Applikationsbeschreibung-ShutterController.md](doc/Applikationsbeschreibung-ShutterController.md) abgelegt.

## Zugehörige Firmware

Dies Jalousiensteuerung ist in folgenden Firmwares enthalten:

- [OAM-ShutterController](https://github.com/OpenKNX/OAM-ShutterController)

## Hardware Unterstützung

|Prozessor | Status               | Anmerkung                  |
|----------|----------------------|----------------------------|
|RP2040    | Beta                 |                            |
|ESP32     | Beta                 |                            |
|SAMD      | Compilable, Untested |                            |

Getestete Hardware:
- Adafruit ESP32 Feather V2
- OpenKNX Reg1-Base (TP)

## Einbindung in die Anwendung

In das Anwendungs XML muss das OFM-ShutterControlModule aufgenommen werden:

```xml
  <op:define prefix="SHC" ModuleType="23"
    share=   "../lib/OFM-ShutterControllerModule/src/ShutterControllerModule.share.xml"
    template="../lib/OFM-ShutterControllerModule/src/ShutterControllerModule.templ.xml"
    NumChannels="10"
    KoSingleOffset="1000"
    KoOffset="1010">
    <op:verify File="../lib/OFM-ShutterControllerModule/library.json" ModuleVersion="0" /> 
  </op:define>
```
**Hinweis:** Das Module verwendet 9 Global KO's und weiters pro Kanal 42 KO's. Dies muss bei nachfolgenden Modulen bei KoOffset und KoSingleOffset entsprechend berücksichtigt werden. 

In main.cpp muss das ShutterControlModule ebenfalls hinzugefügt werden:

```
[...]
#include "ShutterControlModule.h"
[...]

void setup()
{
    [...]
    openknx.addModule(3, openknxShutterControlModule);
    [...]
}
```

## Lizenz

[GNU GPL v3](LICENSE)