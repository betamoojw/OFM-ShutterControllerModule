# OFM-ShutterController

Diese Module dient zur Jalousiensteuerung.

## Features


## Hardware Unterstützung

|Prozessor | Status | Anmerkung                  |
|----------|--------|----------------------------|
|RP2040    | Alpha  |                            |
|ESP32     | Alpha  |                            |

Getestete Hardware:
- Adafruit ESP32 Feather V2

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