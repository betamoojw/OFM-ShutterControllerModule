v 0.6.0
- Refactor: "Helligkeit" (bool) + "Weitere Helligkeitssensoren" (count) zusammengefasst zu "Helligkeitssensoren" (Enum: Nein/1-5 Sensoren)
- Rename: "Helligkeit Sensor 1..5" -> "Ausrichtung Sensor 1..5"
- Breaking: Offset 19 Semantik geaendert (SHC_VerifyVersion 0.5 -> 0.6)
v 0.5.0
- Feature: Dachflaeche bevorzugt unzugeordnete Helligkeitssensoren (z.B. Dachsensor)
- Feature: Wiederherstellung der vorherigen Position nach Fenster offen/gekippt
- Fix: Azimut-/Helligkeit-UI klarer (Helligkeit Sensor 1..5, "Keine Himmelsrichtung (Azimut-Auswertung aus)")
- Fix: Schreibfehler und Himmelsrichtungsbezeichnungen korrigiert
- Doc: Help-Context und Applikationsbeschreibung aktualisiert
v 0.4.2
- Fix: Stopp von Rolladen bei manueller Bedienung durch Jalousiensteuerung
v 0.4.1
- Feature: Bessere Bennenung der Gruppenobjekte
v 0.4.0
- Feature: Invertieren der Fensterkontakt KO
- Feature: Auswahl verhalten der Fensterkontakte
- Feature: Wartezeit für Fensterkontaktauswertung
- Fix: Das setzen einer Sperre führte zum Hänger des gesamten Gerätes, das ist raus, aber wahrscheinlich geht es besser, ich steige nur nicht durch, was zu machen wäre.
- Fix: die KOs für offen und kipp waren vertauscht
- Fix: Wenn man mehrmals zwischen kipp und offen wechselt, fährt der Rollladen beim Schließen nicht in die Position vor dem öffnen, sondern in eine der kipp- oder offen-Positionen.
- Fix: Gruppe offen / gekippt in der ETS Baumansicht vertauscht
v 0.3.0
- Fix: Handsteuerungslogik 
v 0.2.0
- Bugfix: Lammellen-KO wird bei Gerätetpye 'Rollo' angezeigt
- Bugfix: Position anfahren bei Fenster offen/gekippt
- Feature: Sonderfunktionen Tasterbedienung bei geschlossenen Jalousien
- Feature: Neue Sonderfunktion "Beschattung Ein" und "Beschattung Aus"
- Feature: Aussperrverhinderung
