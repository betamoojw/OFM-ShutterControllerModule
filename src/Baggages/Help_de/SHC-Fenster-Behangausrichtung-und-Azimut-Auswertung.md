### Fenster-/Behangausrichtung und Azimut-Auswertung

Die Fenster-/Behangausrichtung im Kanal steuert, wie die Helligkeitssensoren ausgewertet werden:

- **Ost/Suedost/Sued/Suedwest/West**: Azimut-Auswertung ist aktiv. Es werden nur Sensoren mit Azimut-Zuordnung verwendet.
- **Dachflaeche**: Bevorzugt Sensoren ohne Azimut-Zuordnung (z.B. Dachsensor). Falls keine vorhanden sind, wird der Maximalwert aller Sensoren verwendet.
- **Keine Himmelsrichtung (Azimut-Auswertung aus)**: Es wird die eingestellte Aggregation (Mittelwert/Maximum) ueber alle gueltigen Sensoren verwendet.

Beispiele (vereinfachte Sicht):

| Sensor-Setup | Fenster-/Behangausrichtung | Ergebnis fuer Helligkeit |
| --- | --- | --- |
| 3x Sensor mit Azimut (O/S/W) | Dachflaeche | Max(alle 3 Sensoren) |
| 4x Sensor mit Azimut + 1x Sensor ohne Azimut | Dachflaeche | Max(alle Sensoren ohne Azimut) |
| 4x Sensor mit Azimut + 1x Sensor ohne Azimut | Sued | Azimut-Interpolation nur mit den 4 Azimut-Sensoren |
| 2x Sensor ohne Azimut | Keine Himmelsrichtung (Auswertung aus) | Aggregation ueber alle Sensoren ohne Azimut |
| 1x Sensor mit Azimut | Keine Himmelsrichtung (Azimut-Auswertung aus) | Aggregation ueber alle gueltigen Sensoren |

