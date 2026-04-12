### Fenster-/Behangausrichtung und Azimut-Auswertung

Die Fenster-/Behangausrichtung im Kanal steuert, wie die Helligkeitssensoren ausgewertet werden:

- **Ost/Suedost/Sued/Suedwest/West**: Azimut-Auswertung ist aktiv. Es werden nur Sensoren mit Azimut-Zuordnung verwendet.
- **Dachflaeche**: Bevorzugt Sensoren ohne Azimut-Zuordnung (z.B. Dachsensor). Die eingestellte Aggregation wird ignoriert — es gilt immer der Maximalwert. Falls keine Sensoren ohne Azimut-Zuordnung vorhanden sind, greift ein automatischer Fallback: alle Sensoren werden mit Max-Aggregation ausgewertet.
- **Keine Himmelsrichtungsauswertung**: Azimut-basierte Sensorauswahl ist deaktiviert. Alle gueltigen Sensoren werden mit der eingestellten Aggregation (Mittelwert/Maximum) zusammengefasst.

Hinweis: Obwohl "Dachflaeche" und "Keine Himmelsrichtungsauswertung" beide die Azimut-Auswertung deaktivieren, unterscheiden sie sich in zwei Punkten. "Dachflaeche" bevorzugt gezielt Sensoren ohne Azimut-Zuordnung und erzwingt immer Max-Aggregation. "Keine Himmelsrichtungsauswertung" behandelt alle Sensoren gleichwertig und respektiert die konfigurierte Aggregation.

Beispiele (vereinfachte Sicht):

| Sensor-Setup | Fenster-/Behangausrichtung | Ergebnis fuer Helligkeit |
| --- | --- | --- |
| 3x Sensor mit Azimut (O/S/W) | Dachflaeche | Max(alle 3 Sensoren) — Fallback, da kein unzugeordneter Sensor |
| 4x Sensor mit Azimut + 1x Sensor ohne Azimut | Dachflaeche | Max(nur der Sensor ohne Azimut) |
| 4x Sensor mit Azimut + 1x Sensor ohne Azimut | Sued | Azimut-Interpolation nur mit den 4 Azimut-Sensoren |
| 2x Sensor ohne Azimut | Keine Himmelsrichtungsauswertung | Aggregation ueber alle Sensoren ohne Azimut |
| 1x Sensor mit Azimut | Keine Himmelsrichtungsauswertung | Aggregation ueber alle gueltigen Sensoren |

