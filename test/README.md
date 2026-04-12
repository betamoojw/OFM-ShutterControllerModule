# Tests (OFM-ShutterControllerModule)

## Python (Logik-Simulation)

Dieser Test laeuft ohne Hardware und simuliert die Helligkeitslogik.

```bash
python lib/OFM-ShutterControllerModule/test/brightness_logic_test.py
```

## C++ (PlatformIO Unity)

Diese Tests werden ueber PlatformIO ausgefuehrt.

```bash
pio test -e develop_RP2040 -f test_brightness_logic
```

Hinweis: Die Tests laufen im PlatformIO-Umfeld und nutzen Unity.
