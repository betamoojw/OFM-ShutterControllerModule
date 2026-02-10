# Testplan ShutterController

Dieser Testplan deckt Logik-, Integrations- und HIL/Manuell-Tests fuer die Beschattungslogik ab.
Ziel ist, die Auswertung von Helligkeit/Azimut, Prioritaeten der Modi, Fallback-Verhalten
sowie ETS/KNX-Integration sauber nachzuweisen.

## 1) Automatisierte Logiktests (Unit)

### 1.1 Helligkeit / Azimut / Dachflaeche
- TC-BRI-001: Azimut-Auswertung aktiv, 3 Sensoren mit Azimut (O/S/W), gueltiger Sonnenazimut.
  - Erwartung: Azimut-Interpolation verwendet nur Sensoren mit Azimut.
- TC-BRI-002: Dachflaeche, 4 Sensoren mit Azimut + 1 Sensor ohne Azimut.
  - Erwartung: Dachflaeche bevorzugt Sensor(en) ohne Azimut, nimmt Max der unzugeordneten.
- TC-BRI-003: Dachflaeche, nur Sensoren mit Azimut.
  - Erwartung: Max ueber alle Sensoren.
- TC-BRI-004: Keine Himmelsrichtung (Azimut-Auswertung aus), Aggregation=Mean.
  - Erwartung: Mittelwert aller gueltigen Sensoren.
- TC-BRI-005: Keine Himmelsrichtung (Azimut-Auswertung aus), Aggregation=Max.
  - Erwartung: Max aller gueltigen Sensoren.
- TC-BRI-006: Sonnenazimut ungueltig.
  - Erwartung: Azimut-Auswertung faellt auf Aggregatwert.
- TC-BRI-007: Sensor-Watchdog ignoreValue/waitForValue.
  - Erwartung: Nicht-gueltige Sensoren werden nicht in Aggregat/Azimut genutzt.
- TC-BRI-008: Fallback-Mode Provide/Ignore.
  - Erwartung: Fallback-Wert oder Ignorieren gemaess Einstellung.

### 1.2 Modus-Prioritaeten
- TC-MOD-001: Fenster offen aktiv.
  - Erwartung: Fenster-Modus hat hoehere Prioritaet als Handbetrieb/Nacht/Beschattung.
- TC-MOD-002: Handbetrieb aktiv, Fenster nicht aktiv.
  - Erwartung: Handbetrieb vor Nacht/Beschattung.
- TC-MOD-003: Nachtmodus aktiv, keine Sperren.
  - Erwartung: Nachtmodus vor Beschattung.
- TC-MOD-004: Mehrere Beschattungsmodi erlaubt.
  - Erwartung: Hoechste erlaubte Nummer wird aktiv.

### 1.3 Grenzen, Hysterese, Wartezeiten
- TC-LIM-001: Azimut/Elevation innerhalb Grenzen.
  - Erwartung: Beschattung zulassbar.
- TC-LIM-002: Azimut/Elevation ausserhalb Grenzen.
  - Erwartung: Beschattung nicht zulassbar.
- TC-LIM-003: Helligkeit Hysterese.
  - Erwartung: Kein Flattern bei kleinen Schwankungen.
- TC-LIM-004: Beschattungsstart/Beschattungsende Wartezeiten.
  - Erwartung: Aktivierung/Deaktivierung erst nach Ablauf.

### 1.4 Fenster-Offen/Geoeffnet/GeKippt
- TC-WIN-001: Fensterkontakt 1 aktiv, 2 inaktiv.
  - Erwartung: Fenster offen Modus.
- TC-WIN-002: Fensterkontakt 2 aktiv, je nach Konfiguration.
  - Erwartung: Fenster gekippt Modus.
- TC-WIN-003: Restore vorheriger Position nach Fenster-Modus Ende.
  - Erwartung: Position/Slat werden wiederhergestellt.

### 1.5 Szenarien und Ergebnisse

| TC | Szenario (Kurzform) | Automatisiert | Ergebnis |
| --- | --- | --- | --- |
| TC-BRI-001 | 2 Azimut-Sensoren (90/180), Sonnenazimut 135 -> Interpolation | Python | PASS |
| TC-BRI-002 | Dachflaeche, 2 Azimut + 1 ohne Azimut -> Max(ohne Azimut) | Python | PASS |
| TC-BRI-003 | Dachflaeche, nur Azimut-Sensoren -> Max(alle) | Python | PASS |
| TC-BRI-004 | Azimut-Auswertung aus, Aggregation=Mean | Nicht automatisiert | NICHT GETESTET |
| TC-BRI-005 | Azimut-Auswertung aus, Aggregation=Max | Nicht automatisiert | NICHT GETESTET |
| TC-BRI-006 | Sonnenazimut ungueltig -> Aggregatwert | Nicht automatisiert | NICHT GETESTET |
| TC-BRI-007 | Watchdog ignore/wait -> Sensoren ausgeschlossen | Nicht automatisiert | NICHT GETESTET |
| TC-BRI-008 | Fallback Provide/Ignore | Nicht automatisiert | NICHT GETESTET |
| TC-MOD-001 | Fenster offen aktiv -> Prioritaet hoch | Nicht automatisiert | NICHT GETESTET |
| TC-MOD-002 | Handbetrieb aktiv -> Prioritaet vor Nacht/Beschattung | Nicht automatisiert | NICHT GETESTET |
| TC-MOD-003 | Nachtmodus aktiv -> vor Beschattung | Nicht automatisiert | NICHT GETESTET |
| TC-MOD-004 | Mehrere Beschattungsmodi -> hoechste Nummer | Nicht automatisiert | NICHT GETESTET |
| TC-LIM-001 | Azimut/Elevation innerhalb Grenzen | Nicht automatisiert | NICHT GETESTET |
| TC-LIM-002 | Azimut/Elevation ausserhalb Grenzen | Nicht automatisiert | NICHT GETESTET |
| TC-LIM-003 | Helligkeit Hysterese | Nicht automatisiert | NICHT GETESTET |
| TC-LIM-004 | Wartezeiten Start/Ende | Nicht automatisiert | NICHT GETESTET |
| TC-WIN-001 | Kontakt 1 aktiv, Kontakt 2 inaktiv | Nicht automatisiert | NICHT GETESTET |
| TC-WIN-002 | Kontakt 2 aktiv (je nach Konfig) | Nicht automatisiert | NICHT GETESTET |
| TC-WIN-003 | Restore Position/Slat nach Fenster-Modus | Nicht automatisiert | NICHT GETESTET |

Hinweise:
- PlatformIO C++ Testlauf konnte nicht gestartet werden ("pio" nicht verfuegbar, Python-Modul "platformio" nicht installiert).

## 2) Simulation/Integration (teil-automatisiert)

### 2.1 KO-Input-Simulation
- TC-SIM-001: Simuliere KO Eingaben fuer Helligkeit/Temp/Wolken/Regen.
  - Erwartung: Beschattung erlaubt/nicht erlaubt gem. Grenzen.
- TC-SIM-002: Simuliere Fensterkontakte + Handbetrieb.
  - Erwartung: Prioritaet Fenster > Hand > Nacht > Beschattung.

### 2.2 Szenarien (End-to-End Logik)
- TC-SCEN-001: 4 Fassadensensoren + 1 Dachsensor.
  - Erwartung: Fassadenkanaele nutzen Azimut, Dachflaeche nutzt Dachsensor.
- TC-SCEN-002: Nur Dachsensor, Dachflaeche.
  - Erwartung: Dachsensor bestimmt Helligkeit.
- TC-SCEN-003: Nur Fassadensensoren, Dachflaeche.
  - Erwartung: Max ueber alle Sensoren.
- TC-SCEN-004: Azimut-Auswertung aus.
  - Erwartung: Aggregation ueber alle Sensoren.

## 3) HIL / Manuell (ETS + Hardware)

### 3.1 ETS/KNXprod
- TC-ETS-001: Import .knxprod, Seiten/Labels pruefen.
  - Erwartung: "Keine Himmelsrichtung (Azimut-Auswertung aus)" sichtbar.
- TC-ETS-002: Help-Context Links.
  - Erwartung: Alle Parameter zeigen passende Hilfe.

### 3.2 Bus-Telegramme
- TC-BUS-001: Echte Gruppenadressen senden (Helligkeit/Temp/Lock).
  - Erwartung: Beschattung reagiert gemaess Logik.
- TC-BUS-002: Fensteroeffnen/Kippen.
  - Erwartung: Moduswechsel + Restore.

### 3.3 Hardwareverhalten
- TC-HW-001: Positionsfahrt und Lamellenstellung.
  - Erwartung: Werte korrekt angefahren.
- TC-HW-002: Timing/Watchdog.
  - Erwartung: Fallback/Ignore Verhalten.

## 4) Automatisierungs-Optionen

- Unit-Tests als C++ Tests (z.B. in OFM-ShutterControllerModule/test).
- Python-Tests fuer Logik (fuer reine Berechnungen/Mocks).
- Simulation ueber Ko-Eingaben (CI-freundlich, keine Hardware noetig).

## 5) Abnahmekriterien

- Alle Unit-Tests gruen.
- Kritische Szenarien (Dachflaeche, Azimut-Auswertung aus) nachweislich korrekt.
- ETS-UI/Help-Context konsistent zur Doku.
- HIL-Tests erfolgreich fuer reale Telegramme und Fahrverhalten.
