v 0.10.0
- Feature: Benutzerdefinierte Position nach Beschattungsende anfahren (Port aus Upstream)
- Fix: Sperrobjekt für Beschattungsmodus wurde ignoriert
- Fix: Beschriftung (Eingang/Ausgang) der KOs „Aktorrückmeldung" und „Beschattungsmodus Sperre Aktiv"
- Fix: Überlappungswarnungen in knxprod unterdrückt
- Breaking: Parameterblock je Kanal um 2 Byte vergrößert (Fenster offen- und Beschattungsmodus-Blöcke verschoben). Bestehende ETS-Projekte müssen neu parametriert werden.

v 0.6.4
- Feature: Neue Einstellung „Betriebsart Lamellennachführung" für die Modi 5 und 6 (Geo. Positions- und Lamellennachführung). Vier Optionen: „Direktlicht abschneiden (Cut-Off)" (bisheriges Verhalten: geometrische Cut-Off-Formel), „Blendschutz (Retro-Reflexion)" (Retro-Reflexion: θ = Profilwinkel), „Adaptiv (Daylight)" (θ = min(θ_CutOff, θ_Retro) – maximale Tageslichtnutzung bei gleichzeitigem Blendschutz) und „Über Tabelle" (6 konfigurierbare Stützpunkte Höhenwinkel → Lamellenposition % mit linearer Interpolation, Standardwerte gemäß Warema-Tabelle).
- Feature: Neuer Parameter „Beschattungsstart" – legt fest, ob die Beschattung sofort (mit Schutzposition) oder erst bei direktem Sonnenlicht (Profilwinkel > 0°) startet.
- Change: Helligkeitsmessung über Azimut auf cosinus-gewichtete Interpolation umgestellt – alle freigegebenen Helligkeitssensoren werden gleichzeitig nach ihrer Winkeldifferenz zur Sonne gewichtet (statt nur die zwei nächstliegenden linear zu interpolieren). Liefert stabilere Werte bei mehreren Sensoren.
- Change: „Status Beschattung Bereit (Benutzer)" wird jetzt auch ohne aktiven Beschattungsmodus korrekt als bereit gemeldet, sobald alle anderen Bedingungen erfüllt sind (vorher fälschlich nur bereit, wenn bereits ein Beschattungsmodus aktiv war).
- Fix: Lamellenwinkel-Skalierung bei geometrischer Berechnung korrigiert – der Nenner war fälschlicherweise „Lamellenwinkel vollständig geschlossen" (Winkel von der Vertikalen), korrekt ist (90° − dieser Wert) (Winkel von der Waagrechten bis vollständig geschlossen). Auswirkung: Lamellen wurden bisher fast immer auf 100% (oder den Clip-Maximalwert) gesetzt, jetzt werden physikalisch korrekte Zwischenstellungen berechnet.
- Fix: KO „Status Beschattung Bereit (Benutzer)" wird beim Geräteneustart explizit mit „false" initialisiert (vorher uninitialisiert auf dem Bus sichtbar).
- Fix: Speicherüberlappung im Beschattungsmodus-Block behoben – die Parameter „Beschattungsstart" (RequireFacadeHit) und der Tabellen-Höhenwinkel (SlatTableMinElevation) lagen fälschlicherweise innerhalb der 4-Byte-Float-Parameter „Minimaler UV-Index" bzw. „Minimale Raumtemperatur" und überschrieben sich gegenseitig. Beide wurden auf eigene Bytes am Blockende verschoben.
- Breaking: Parameterlayout des Beschattungsmodus-Blocks vergrößert (increment 52 → 68 Bytes / 416 → 544 Bit). Betrifft Kanäle mit zwei konfigurierten Beschattungsmodi. Bestehende ETS-Projekte müssen neu parametriert werden.
- Breaking: Parameterwert für „Über Tabelle" verschoben (Value 2 → 3). Bestehende ETS-Projekte mit „Über Tabelle"-Konfiguration müssen neu parametriert werden.

v 0.6.3
- Redesign: Beschattungsmodi Jalousie komplett neu strukturiert – aufbauende Modi-Reihe: Nein → Standard → Lamellenführung (Min/Max) → Geo. Positionsnachführung → Geo. Positionsnachführung (Min/Max) → Geo. Positions- und Lamellennachführung → Geo. Positions- und Lamellennachführung (Min/Max).
- Feature: Neuer Beschattungsmodus 6 „Geo. Positions- und Lamellennachführung (Min/Max)" – entspricht Modus 5 mit konfigurierbarer Min./Max.-Begrenzung für Position und Lamellenstellung. Wenn Min. Position > Max. Position konfiguriert ist, wird die Positionsbegrenzung ignoriert. Die Lamellenstellungsbegrenzung verwendet automatisch min/max der beiden Werte (Reihenfolge egal).
- Feature: Neuer Rollo-Modus „Geo. Positionsnachführung (Min/Max)" (Wert 2) – entspricht Modus 1 mit Min./Max.-Positionsbegrenzung.
- Breaking: Parameterlayout des Beschattungsmodus-Blocks vergrößert (increment 50 → 52). Betrifft nur Kanäle mit zwei oder mehr konfigurierten Beschattungsmodi. Bestehende ETS-Projekte mit nur einem Beschattungsmodus pro Kanal sind nicht betroffen.
- Breaking: Modus 3 (war „Lamellennachführung Experte") wird jetzt „Geo. Positionsnachführung" (Eindringtiefe + feste Lamellenstellung). Der alte Experte-Modus entfällt. Bestehende ETS-Projekte mit Modus 3 müssen neu konfiguriert werden.
- Rename: Beschattungsmodus 4 umbenannt: „Schattenkantennachführung" → „Geo. Positionsnachführung (Min/Max)". Beschattungsmodus 5 umbenannt: „Schattenkanten- und Lamellenführung" → „Geo. Positions- und Lamellennachführung". Die Parameterwerte (4, 5) sind unverändert.
- Rename: ETS-Parameter „Lamellenstellenung an Sonnenstand anpassen" (Tippfehler) → „Positions- und Lamellennachführung".
- Rename: Rollo-Parameter „Schattenkantennachführung" → „Geo. Positionsnachführung" (Wert 1 unverändert).
- Fix: Kritischer Kippwinkel (theta_krit) wurde fälschlicherweise als 90°−atan2(...) berechnet; korrekt ist direkt atan2(...) (Winkelkonvention zur Senkrechten).
- Fix: Bei Dachflächen-Orientierung wurde sin_alpha ohne Betrag berechnet, was bei negativen Neigungswinkeln zu falschen Positionen führte (jetzt fabsf).
- Fix: OffsetSlatPosition wurde in Modus 5 nicht auf die berechnete Lamellenstellung angewendet.
- Fix: Hysterese (MinChangeForSlatAdaption) fehlte in Modus 2 (Standard-Lamellennachführung) vor dem Setzen der Lamellenstellung.
- Change: Wertebereich „Max. Eindringtiefe" angepasst: 1–200 → 10–255 cm.

v 0.6.2
- Feature: Neue Beschattungsmodi 3 „Lamellennachführung Experte", 4 „Schattenkantennachführung", 5 „Schattenkanten- und Lamellennachführung"
- Feature: Schattenkantennachführung für Rollo (neuer Parameter „Positionsnachführung" im Rollo-Kanal)
- Feature: Neuer Kanal-Parameter „Fassadenneigung/Fensterneigung" für geometrische Berechnung
- Breaking/Rename: Beschattungsmodus-Lamellensteuerung Wert 2 umbenannt: „Benutzerdefiniert" → „Lamellennachführung Min/Max". Der Parameterwert (2) ist unverändert. Bestehende ETS-Projekte laufen ohne Migration weiter; lediglich der angezeigte Text im ETS-Dropdown ändert sich nach einem Update der knxprod.

v 0.6.1
- Feature: Positionsprüfung im Status „Beschattung Bereit (Benutzer)" – Beschattungsbereitschaft wird nur signalisiert, wenn die aktuelle Zielposition ≤ dem konfigurierten Grenzwert „Nur wenn Position kleiner als" ist
- Feature: KO-Freigabe-Parameter je Kanal – alle optionalen KOs können nun einzeln in der ETS ein-/ausgeblendet werden (Sperren Kanal, Status aktiver Modus, Aktorrückmeldung, Beschattung ein/aus, Beschattung aktiv, Beschattungsbereitschaft, Handbetrieb Schalten, Handbetrieb Position, Fenster offen/gekippt Status und Sperren)
- Feature: KO-Freigabe-Parameter je Beschattungsmodus – Sperren, Status Aktiv und Status Bereitschaft je Beschattungsmodus separat freigebbar
- Feature: KO-Freigabe-Parameter je Fensterkontakt – Status und Sperren für Fenster offen / Fenster gekippt separat freigebbar
- Rename: KO-Freigabe-Parameter für Beschattungsmodus konsistent benannt (PPP+53 = Sperren freigeben, PPP+54 = Bereitschaft freigeben, PPP+55 = Status Aktiv freigeben)

v 0.6.0
- Refactor: "Helligkeit" (bool) + "Weitere Helligkeitssensoren" (count) zusammengefasst zu "Helligkeitssensoren" (Enum: Nein/1-5 Sensoren)
- Rename: "Helligkeit Sensor 1..5" -> "Ausrichtung Sensor 1..5"
- Breaking: Offset 19 Semantik geaendert (SHC_VerifyVersion 0.5 -> 0.6)

v 0.5.0
- Feature: Neues KO "Status Beschattung Bereit" je Kanal
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
