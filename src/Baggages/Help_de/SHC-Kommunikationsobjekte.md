### Kommunikationsobjekte

### Übersicht

Alle Kommunikationsobjekte sind mit dem Präfix "Jalousie %C%: " (Kanal-KOs) bzw. "Jalousiensteuerung: " (globale KOs) beschriftet; dieser Präfix ist in der Tabelle weggelassen.

#### Globale Kommunikationsobjekte

Die absolute KO-Nummer ergibt sich aus `KoSingleOffset + KO`. In der Jalousiensteuerung ist `KoSingleOffset = 400`, die globalen KOs liegen also bei 400-412.

| KO | DPT | Bezeichnung | Erklärung |
|---:|---|---|---|
| 0 | 1.001 | Beschattung täglich aktivieren | Eingang, Schalten. Nur bei "Tägliche Aktivierung" = "Über KO" |
| 1 | 1.011 | Beschattung täglich aktivieren Status | Ausgang. Nur bei "Tägliche Aktivierung" = "Über KO" |
| 2 | 9.001 | Temperatur | Eingang, °C |
| 3 | 9.001 | Temperaturprognose | Eingang, °C |
| 4 | 9.004 | Helligkeit | Eingang, Lux |
| 5 | 9.031 | UV-Index | Eingang, Gleitkomma |
| 6 | 1.001 | Regen Ja/Nein | Eingang, Schalten |
| 7 | 5.001 | Bewölkung | Eingang, Prozent |
| 8 | 1.005 | Fehlender Messwert | Ausgang, Alarm |
| 9 | 9.004 | Helligkeit 2 | Eingang, Lux |
| 10 | 9.004 | Helligkeit 3 | Eingang, Lux |
| 11 | 9.004 | Helligkeit 4 | Eingang, Lux |
| 12 | 9.004 | Helligkeit 5 | Eingang, Lux |

#### Kommunikationsobjekte pro Kanal

Jeder Kanal belegt einen festen Block von 53 aufeinanderfolgenden KOs (Offset `+0`..`+52`). Die absolute KO-Nummer ergibt sich aus `KoOffset + (Kanal - 1) * 53 + Offset`; in der Jalousiensteuerung ist `KoOffset = 420`, Kanal 1 liegt also bei 420-472, das letzte KO von Kanal 32 bei 2115. Nicht alle KOs sind immer sichtbar; die meisten lassen sich im Abschnitt "Kommunikationsobjekte freigeben" des jeweiligen Kanals einzeln ein-/ausblenden.

|  KO | DPT | Bezeichnung | Erklärung |
|----:|---|---|---|
|  +0 | 5.001 | Position setzen | Ausgang, Prozent |
|  +1 | 5.001 | Lamellenstellung setzen | Ausgang, Prozent (nur Jalousie) |
|  +2 | 1.008 | Auf/Ab setzen | Ausgang, Auf=0 / Ab=1 |
|  +3 | 1.007 | Stopp/Schritt setzen | Ausgang |
|  +4 | 5.001 | Aktorrückmeldung Höhe absolut | Eingang, Prozent |
|  +5 | 5.001 | Aktorrückmeldung Lamellenstellung | Eingang, Prozent (nur Jalousie) |
|  +6 | 1.001 | Beschattung Einschalten | Eingang, Schalten |
|  +7 | 1.011 | Beschattung Eingeschaltet | Ausgang |
|  +8 | 1.001 | Beschattung Aktiv | Ausgang |
|  +9 | 1.001 | Sperre | Eingang, Sperre=1 |
| +10 | 1.011 | Sperre Aktiv | Ausgang |
| +11 | 17.001 | Aktiver Modus | Ausgang, Zahl. Siehe Diagnose für die Belegung, Szenen für die Werte 21-36 |
| +12 | 1.010 | Handbetrieb Aus-/Einschalten | Eingang, Schalten |
| +13 | 1.011 | Handbetrieb Aktiv | Ausgang |
| +14 | 1.001 | Handbetrieb Sperre | Eingang, Sperre=1 |
| +15 | 1.011 | Handbetrieb Sperre Aktiv | Ausgang |
| +16 | 1.008 | Handbetrieb Auf/Ab | Eingang, Ab=1 / Auf=0 |
| +17 | 1.007 | Handbetrieb Stopp/Schritt | Eingang, Erhöhen=1 |
| +18 | 5.001 | Handbetrieb Position setzen | Eingang, Prozent |
| +19 | 5.001 | Handbetrieb Lamellenstellung setzen | Eingang, Prozent (nur Jalousie) |
| +20 | 1.011 | Nachtmodus Aktiv | Ausgang |
| +21 | 1.001 | Nachtmodus Aus-/Einschalten | Eingang, Schalten |
| +22 | 1.001 | Nachtmodus Sperre | Eingang, Sperre=1 |
| +23 | 1.011 | Nachtmodus Sperre Aktiv | Ausgang |
| +24 | 5.001 / 1.001 | Heizung Stellwert / Heizung Aktiv | Eingang, Prozent bzw. Eingang, Aktiv=1, je nach "Heizungsanforderung" |
| +25 | 9.001 | Raumtemperatur | Eingang, °C |
| +26 | 1.001 | Handbetrieb Auf/Ab (ohne Sonderfunktion) | Eingang, Ab=1 / Auf=0 |
| +27 | 1.001 | Status Beschattung Bereit | Ausgang |
| +28..+31 | – | Fenster offen (Instanz 1) | Aktiv, Kontakt, Sperre, Sperre Aktiv |
| +32..+35 | – | Fenster gekippt (Instanz 2) | Aktiv, Kontakt, Sperre, Sperre Aktiv |
| +36..+43 | – | Beschattungsmodus 1 | Aktiv, Sperre, Sperre Aktiv, Beschattungsunterbrechung Sperre, Beschattungsunterbrechung Sperre Aktiv, 'Nicht erlaubt' Bits, 'Nicht erlaubt' Grund, Bereitschaft |
| +44..+51 | – | Beschattungsmodus 2 | wie Beschattungsmodus 1 |
| +52 | 18.001 | Szene | Eingang, Szene. Abruf und Lernen |
