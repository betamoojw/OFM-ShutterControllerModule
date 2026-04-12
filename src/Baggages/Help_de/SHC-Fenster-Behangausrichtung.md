### Fenster-/Behangausrichtung

Legt die Ausrichtung des Fensters bzw. Behangs fuer diesen Kanal fest.
Diese Angabe wird verwendet, um den passenden Helligkeitssensor fuer die Beschattungsauswertung auszuwählen.

- **Ost/Suedost/Sued/Suedwest/West**: Azimut-Auswertung ist aktiv. Es werden nur Sensoren mit Azimut-Zuordnung verwendet.
- **Dachflaeche**: Bevorzugt Sensoren ohne Azimut-Zuordnung (z.B. Dachsensor). Die eingestellte Aggregation wird ignoriert — es gilt immer der Maximalwert. Falls keine Sensoren ohne Azimut-Zuordnung vorhanden sind, greift ein automatischer Fallback: alle Sensoren werden mit Max-Aggregation ausgewertet.
- **Keine Himmelsrichtungsauswertung**: Azimut-basierte Sensorauswahl ist deaktiviert. Alle gueltigen Sensoren werden mit der eingestellten Aggregation (Mittelwert/Maximum) zusammengefasst.

