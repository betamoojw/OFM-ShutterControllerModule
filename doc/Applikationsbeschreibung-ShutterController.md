# Applikationsbeschreibung Jalousiensteuerung 

Die Jalousiensteuerung bietet unterschiedliche Betriebsarten.
Jede Betriebsart kann durch Ereignisse, Eingangs- oder Messwerte zulässig sein. 
Sind mehrere Betriebsarten zulässig, entscheidet die Priorität über die Auswahl.

## Betriebsarten

Folgende Betriebsarten - gereiht nach der Priorät - stehen zur Verfügung:

### Bereitschaft (Priorität 7 - niedrigste)

In dieser Betriebsart ist die Steuerung im Leerlauf und wartet auf Ereignisse die eine Wechsel der Betriebsart bewirken.

### Beschattung 1 (Priorität 6)

Dieser Modus steht nur zur Verfügung, wenn in der Kanaleinstellung unter "Modus Auswahl" **Beschattungsmodus Anzahl** mindestens 1 eingestellt wurde.

### Beschattung 2 (Priorität 5)

Dieser Modus steht nur zur Verfügung, wenn in der Kanaleinstellung unter "Modus Auswahl" **Beschattungsmodus Anzahl** 2 eingestellt wurde.

### Nachtmodus (Priorität 4)

Dieser Modus steht nur zur Verfügung, wenn in der Kanaleinstellung unter "Modus Auswahl" **Nachtmodus** aktiviert wurde.

Achtung: Wenn kein automatisches Ende konfiguriert ist, muss der Nachtmodus durch Handbetrieb oder durch AUS am Eingang `Nachtmodus Aus-/Einschalten` deaktiviert werden, damit eine Beschattung stattfindet kann.

### Handbetrieb (Priorität 3)

In diese Betriebsart wird gewechselt, sobald eine Handbedienung über einen der Eingänge erkannt wird:

- `Handbetrieb Auf/Ab`
- `Handbetrieb Stopp/Schritt`
- `Handbetrieb Prozent`
- `Handbetrieb Lamelle Prozent` 

zusätzlich kann der Handbetrieb über den Eingang 

`Handbetrieb Aus-/Einschalten` 

manuell aktiviert bzw. deaktiviert werden.

### Fenster gekippt (Priorität 2)

Dieser Modus ist nur verfügbar, wenn 2 Fensterkontakte zur Verfügung stehen.
Der Modus kann in den meisten Betriebsartenkonfiguriationen gesperrt werden.

### Fenster offen (Priorität 1 - Höchste)

Dieser Modus ist nur verfügbar, wenn mindestens ein Fensterkontakte zur Verfügung stehen.
Der Modus kann in den meisten Betriebsartenkonfiguriationen gesperrt werden.

## Diagnose

Die meisten Betriebsarten bieten einen Ausgang der über die aktuelle Aktivierung der Betriebsart informiert. 

Zusätzlich steht ein Ausgang zur Verfügung, der die Betriebsart als Szenennummer ausgibt:

`Aktiver Modus`

Verwendete Szenennummer:

1=Beschattung 1  
2=Beschattung 2  
3=Beschattung 3  
4=Beschattung 4  
10=Bereit  
11=Handbetrieb  
12=Nacht  
13=Fenster Kippstellung  
14=Fenster Geöffnet  

Hinweis: Die Szenenummer wird auf dem KNX Bus beginnend mit 0 abgebildet. 
Szene 1 ist am Bus also 0. 
Bei machen Smart-Home Systemen (Z.B. OpenHAB) muss auf Bus-Wert abgefragt werden.

## Wichtige Informationen zur richtigen Konfiguration der Gruppenadressen

Die richtige Konfiguration ist abhängig von der Betriebart. Bitte daher die Hinweise im Kapitel [Handbetriebseinstellung](#handbetriebseinstellung) beachten.

# Applikationsprogram

<!-- DOC -->
## Allgemein

(c) OpenKNX, Michael Geramb 2024

Die vollständige Anwendungsbeschreibung ist im Web unter https://github.com/OpenKNX/OFM-ShutterControllerModule/blob/v1/doc/Applikationsbeschreibung-ShutterController.md zu finden.

Insbesondere im Bereich Handbedienung sind wichtige Informationen zur richtigen Verbindung der Gruppenadresse im Kapitel "Handbetriebseinstellung" zu finden. 

### WARNUNG und Sicherheitshweis:

Die Jalousiensteuerung darf aus Sicherheitsgründen nicht bei Beschattungseinrichtungen bei Notausgängen verwendet werden, da eine Automatik im Notfall das Öffnen verhindern könnte.

<!-- DOC -->
### Verfügbare Kanäle

Über diese Auswahl werden die Anzahl der benötigten Steuerkanäle festgelegt.

<!-- DOC -->
### Globale Beschattungseinstellung

<!-- DOC -->
#### Tägliche Aktivierung

Legt fest, ob eine deaktivierte Beschattungsautomatik am nächsten Tag wieder aktiviert werden soll.

- **AUS**  
  Es erfolgt keine automatische aktivierung der Beschattungsautomatik

- **EIN**  
  Es wird täglich um Mitternacht die Beschattungsautomatik eingeschalten

- **Über KO, Standard AUS**  
  Es wird nach dem Start die Beschattungsautomatik nicht täglich reaktivert.
  Die Funktion kann jedoch über ein Kommunikationsobjekt eing- bzw. ausgeschalten werden.

- **Über KO, Standard EIN**  
  Es wird nach dem Start die Beschattungsautomatik täglich reaktivert.
  Die Funktion kann jedoch über ein Kommunikationsobjekt ein- bzw. ausgeschalten werden.

- **Über KO, Standard AUS, initial vom Bus lesen**  
  Es wird nach dem Start die Beschattungsautomatik nicht täglich reaktivert.
  Die Funktion kann jedoch über ein Kommunikationsobjekt eing- bzw. ausgeschalten werden.
  Nach dem Gerätestart wird ein Lesetelegram für das Kommunikationsobjekt auf dem Bus gesendet.

- **Über KO, Standard EIN, initial vom Bus lesen**  
  Es wird nach dem Start die Beschattungsautomatik täglich reaktivert.
  Die Funktion kann jedoch über ein Kommunikationsobjekt ein- bzw. ausgeschalten werden.
  Nach dem Gerätestart wird ein Lesetelegram für das Kommunikationsobjekt auf dem Bus gesendet.

<!-- DOC -->
### Verfügbare Messwerteingänge
Verschiedene Messwerte können benutzt werden, um eine automatische Beschattung zu ermöglichen oder zu verhindern.
In diesem Abschnitt wird gewählt, welche Messwerte in der KNX-Anlage zur Verfügung stehen und zur Steuerung verwenden werden sollen.

<!-- DOC -->
### Messwertüberwachung
Die Messwertüberwachung wird verwendet um bei Ausfall eines Messwertes einen Notbetrieb zu Ermöglichen.

<!-- DOC -->
#### Ausfallsüberwachung
Die Ausfallüberwachung legt fest wie lange auf einem Messwert gewartet wird.
Empängt die Steuerung innerhalb der Zeitspannen keinen Wert, wird in den Notfallbetrieb gewechselt.

<!-- DOC -->
#### Verhalten bei Ausfall
Über diese Auswahl wird festgelegt, wie der Notfallbetrieb den fehlenden Messwert behandeln soll.
Wird innerhalb des Notbetriebes der fehlende Wert empfangen, wird der Notbetrieb automatisch beendet.

- **Wert ingnorieren** 
Der Messwert wird bei der Bestimmung ob eine Beschattung zulässig ist nicht mehr berücksichtigt.

- **Leseanforderung schicken, dann ignorieren**  
Es wird einmalig ein Lesetelegram für den Messwert auf den Bus gesandt, erfolgt weiterhin keine Messwertübertragung wird der Wert bei der Bestimmung ob eine Beschattung zulässig ist nicht mehr berücksichtigt.

- **Fixen Wert vorgeben**  
Ein in der Konfiguration fest eingestellter Wert erstetzt den fehlenden Messwert.
Diese Einstellung wird empfohlen, wenn der Messwert entscheidend ist, welcher Beschattungsmodus aktiv werden soll. 
Über die geignet Wert-Vorgabe kann somit ein Modus bevorzugt werden.

- **Leseanforderung schicken, dann fixen Wert vorgeben**  
Diese Einstellung verhält sich gleich wie vorherige, jedoch wird zuerst einmal ein Lesetelegram für den Messwert auf den Bus gesandt. Erfolgt weiterhin keine Messwertübertragung wird der eingetragen Wert anstatt des Messwertes verwendet.

<!-- DOC -->
#### Wert

Der Wert der im Fehlerfall anstatt des Messwertes verwendet werden soll.
Zu beachten bei der Wahl des Wertes ist, dass dieser je nach Konfiguration entscheidend für die Auswahl des Beschattungsmodus sein kann.

<!-- DOC -->
### Messwerte

Es können verschiedene Messwerte für die automatische Beschattung verwendet werden.

<!-- DOC -->
#### Temperatur

Für den Temperatureingang sollte ein Außentemperatur-Fühler verwendet werden.

<!-- DOC -->
#### Temperatur Prognose

Dieser Eingang eignet sich für die prognostizierte Temperatur eines Wetterdienstes. 
Es kann hierfür z.B. die OpenKNX [OAM-InternetServices](https://github.com/OpenKNX/OAM-InternetServices) mit dem [OFM-InternetWeatherModule](https://github.com/OpenKNX/OFM-InternetWeatherModule) verwendet werden.

<!-- DOC -->
#### Helligkeit

Vorgesehen für den Helligkeitswert einer KNX-Wetterstation.

<!-- DOC -->
#### UV-Index

Dieser Eingang eignet sich für den UV-Index eines Wetterdienstes.
Es kann hierfür z.B. die OpenKNX [OAM-InternetServices](https://github.com/OpenKNX/OAM-InternetServices) mit dem [OFM-InternetWeatherModule](https://github.com/OpenKNX/OFM-InternetWeatherModule) verwendet werden.

<!-- DOC -->
#### Regen

Vorgesehen für den Regen-Indikator einer KNX-Wetterstation.

<!-- DOC -->
#### Wolkenbedeckung

Der Bedeckungsgrad durch Wolken in Prozent von einem Wetterdienst.
Es kann hierfür z.B. die OpenKNX [OAM-InternetServices](https://github.com/OpenKNX/OAM-InternetServices) mit dem [OFM-InternetWeatherModule](https://github.com/OpenKNX/OFM-InternetWeatherModule) verwendet werden.

<!-- DOC HelpContext="Kanal" -->
## Kanal 1-n

Auf dieser Seite werden die verschiedenen Betriebsarten der Jalousiensteuerung festgelegt.

<!-- DOC -->
#### Bezeichnung

Die Bezeichnung wird innerhalb der ETS verwenden um den Kanal und die Kanalobjekte zu benennen.
Es wird empfohlen, die Bezeichnung des Raumes oder der Jalousie zu verwenden.
Z.B. Küche, Wohnzimmer Süden, Wohnzimmer Terassentür, Schlafzimmer...

<!-- DOC -->
#### Geräteart

Die Art der Beschattungseinrichtung:

- **Kanal deaktiviert**  
Diese Einstellung soll verwendet werden wenn der Kanal nicht benötigt wird. 
Achtung, alle Gruppenaddressen Verbindungen des Kanals gehen verloren, die Einstellungen bleiben jedoch Erhalten und können werden bei der Wiederaktivierung verwendet.

- **Jalousie**  
Erlaubt eine Positionsvorgabe und Lamellensteuerung.

- **Rollo**  
Erlaubt eine Positionsvorgabe.
Es steht keine Lamellensteuerung zur Verfügung.

<!-- DOC -->
#### Kanal deaktivieren (zu Testzwecken)

Mit dieser Einstellung kann ein Kanal deaktiviert werden, ohne das die Konfigurationswerte und Gruppenadressen an den Kommunikationsobjekten verloren gehen.
Ein deaktivierter Kanal sendet keine Telegramme auf dem KNX-Bus. 

<!-- DOC -->
### Modus Auswahl

<!-- DOC -->
#### Nachtmodus

Über den Nachtmodus kann die Jalousie oder der Rolladen am Abend automatisch geschlossen und in der Früh geöffnet werden.

<!-- DOC -->
#### Fenster offen

Fenster können einen oder zwei Kontakte zur Verfügung stellen. 
Bei zwei Kontakten kann zwischen Kippstellung und vollständiger Fensteröffnung unterschieden werden und die Steuerung der Jalousie unterschiedlich erfolgen. 
Z.B. kann bei der Kippstellung einer Terrassentüre die Lamelle in die Waagrechte Position gebracht werden um einen optimales Luftzug zu ermöglichen während bei der vollständigen Öffnung die Jalousie hochgefahren werden um den Durchgang zu ermöglichen.

<!-- DOC -->
#### Beschattungsmodus Anzahl

Die Steuerung ermöglicht unterschiedliche Beschattungen abhängig von Messwerten.
Beispielsweise könnnen bei Hitzetagen die Jalousienlamellen geschlossen werden um die Hitze bestmöglich abzuschirmen während bei mäßig warmen Tagen die Lamellen an den Sonnenstand angepasst werden um das Tageslicht in den Raum zu lassen.


<!-- DOC -->
### Beschattungseinstellungen

<!-- DOC -->
#### Beschattung nach Handbetrieb unterbrechen

Mit dieser Einstellung wird festgelegt, wie lange die Beschattung nach dem Handbetrieb unterbrochen wird. 
Bei einer Unterbrechung wird das Kommunikationsobjekt "Beschattung Eingeschaltet" auf AUS gesetzt. Mit einem EIN Befehl auf das Kommunikationsobjekt "Beschattung Einschalten" kann die Beschattung jederzeit wieder manuell aktiviert werden.

Für die automatische reaktiverung stehen folgende Einstellungen bereit:

- für diese Periode deaktivieren  
  Bei dieser Einstellung wird für die Beschattungsperiode die sich aus den konfigurierten Sonnenstandsgrenzen ergibt, deaktivert.

- Zeiten von 1 Minute bis zu 12h  
  Achtung, wird diese Zeit sehr kurz gewählt, wird die Jalousie nach einer manuellen Öffnung sehr schnell wieder in die Beschattungsposition gefahren. Es wird daher emfohlen eine Zeit von mindenstens 30 Minuten zu verwenden. 

<!-- DOC -->
#### Nach Beschattung

Diese Einstellung legt fest, was am Ende der Beschattung passieren soll.

- Keine Änderung  
  Die Jalousie bleibt in der letzten Beschattungseinstellung

- Position vor Beschattungsstart
  Die Position und bei Jalousien auch die Lamellenstellung die zuletzt vor der Beschattung verwendet wurde, wird wieder hergestellt.
  Damit die Jalousiensteuerung die Jalousienposition vor der Beschattung richtig bestimmen kann, ist wichtig, dass alle Handeingänge mit den richtigen Gruppeneingängen verbunden werden. 
  Achtung: Bei Jalousienbedienung über Szenen des Aktors kann die Jalousiensteuerung die richtige Position nicht zuverlässig erkennen. 
  In diesem Fall wird dieser Modus nicht empfohlen.

- Fährt Auf
  Die Jalousie wird vollständig geöffnet.

- Lamelle Waagrecht (Einstellung nur bei Geräteart 'Jalousie' vorhanden)  
  Die Jalousie bleibt in der Position der Beschattung, jedoch wird die Lamelle waagrecht (50%) gestellt.

<!-- DOC -->
### Raumbezogene Messwert Eingänge

Auch Messwerte des Raums der Beschattet wird, können für die Entscheidung ob Beschattet werden soll oder nicht heranzgeogen werden.

<!-- DOC -->
#### Heizung

In dieser Einstellung kann festgelegt werden ob die Heizung als Messgröße verwendet wird.
Diese Einstellung ist Sinnvoll um eine Beschattung während der Heizperiode zu unterbinden.
Dabei kann der festgelegt werden, ob am KNX-Bus lediglich die Heizanforderung als Gruppenadresse zur Verfügung steht oder die aktuelle Stellgröße des Heizungsaktors.

<!-- DOC -->
#### Raumtemperatur

Die Raumtemperatur kann verwendet werden um bei zu niedriger Raumtemperatur die Beschattung zu sperren damit die Sonneneinstrahlung als natürliche Wärmequelle benutzen wird.

<!-- DOC -->
## Handbetrieb

<!-- DOC -->
#### Handbetriebseinstellung

#### Anschluss der Gruppenadressen

Abhängig davon, ob die Handbedienung über den Aktor oder über die OpenKNX Jalousiensteuerung erfolgen soll, müssen die Gruppenadressen unterschiedlich verbunden werden.
*** Wichtig *** Unabhängig vom Modus müssen alle Gruppenadressen die für die Handsteuerung verwendet werden an die entsprechenden Handbedienungseingänge der OpenKNX Jalousiensteuerung verbunden werden.

#### Manuelle Bedienung über den Aktor

![](img/UeberAktor.png)  
In dieser Einstellung erfolgt die Bedienung im Handbetrieb weiterhin direkt über den Aktor.
Diese Einstellung bietet eine erhöhte Betriebssicherheit, da bei einem Ausfall der Steuerung die Bedienung weiterhin gewährleistet ist.
Jedoch steht die Möglichkeit der Sperre der Handbedienung in diesem Modus nicht zur Verfügung.

#### Manuelle Bedienung über die OpenKNX Jalousiensteuerung ("Modul sendet AUF/AB zum Aktor" und "Modul sendet 0/100% zum Aktor")

![](img/UeberOpenKNX.png)  
In dieser Einstellung werden getrennte Gruppenadressen für die Verbindung zwischen Steuerung und Aktor benötigt. Die Jalousienaktor Ansteuerung erfolgt ausschließlich über die Steuerung, die bei Bedarf Befehle von den Tastsensoren weiterleitet.
Der Vorteil dieser Einstellung ist, das die Bedienung über die Tasten durch die Steuerung unterbunden werden kann.

Für die Ansteuerung des Aktors stehen die beiden Einstellung "Modul sendet AUF/AB zum Aktor" und "Modul sendet 0/100% zum Aktor" zur Verfügung. 
Welche der beiden Einstellungen verwendet werden soll, hängt dabei vom jeweiligen Jalousienaktor ab und es muss getestet werden, welche das bessere Ergebniss liefert. 
Empfohlen wird mit der Einstellung "Modul sendet 0/100% zum Aktor" zu beginnen, da diese Einstellung der OpenKNX Jalousien Steuerung mehr Kontrolle über den Aktor bietet.

##### Modul sendet AUF/AB zum Aktor

In dieser Konfiguration muss eine eigene Gruppenadresse zur Verbindung der Jalousiensteuerung mit dem Aktor für das AUF/AB Telegram das am Kommunikationsobjekt "Auf/Ab Ausgang" gesendet wird, konfiguriert werden. 
Jeder manuelle Bedienung über das Kommunikationsobjekt "Handbetrieb Auf/Ab" wird and den Ausgang weitergeleitet, so eine Bedienung über Hand aktuell zulässig ist.

##### Modul sendet 0/100% zum Aktor

In dieser Konfiguration werden Eingangs-Telegramme an den Kommunikationsobjekten "Handbetrieb Auf/Ab" auf Ausgangstelegramme an den Kommunikationsobjekten "Jalousie Prozent Ausgang" umgesetzt.

<!-- DOC -->
#### Erstes manuelles AUF ignorieren, wenn bei Beschattungstart geschlossen

Dieser Einstellung soll verwendet werden ein Taster zum gleichzeitig Öffnen von mehrere Jalousien verwendet wird, diese aber nicht alle die Beschattung benutzen.
In diesem Fall verhindert die Einstellung, dass bei aktiver Beschattung die Jalousie sich mit öffnet.
Jalousien ohne aktiver Beschattung werden aber weiterhin normal geöffnet.

Bei Handbedienungseinstellung "Manuelle Bedienung über Aktor" wird zum ignorieren des manuellen Befehls über das Kommunikationsobjekt "Stopp/Schritt Ausgang" ein Stopp gesendet. Es kann dabei trotzdem zu einer minimalen Bewegung oder einem Geräusch der Jalousie kommen. In den beiden anderen Handbedienungseinstellung "Modul sendet AUF/AB zum Aktor" und "Modul sendet 0/100% zum Aktor" tritt dieser Effekt nicht auf, da in diesem Fall der Aktor keinen Fahrbefehl erhält.

<!-- DOC -->
#### Handbedienung bei globaler Kanal-Sperre erlauben 
Diese Option ist nicht verfügbar wenn "Manuelle Bedienung über den Aktor" unter "Handbetriebseinstellung" gewählt wurde.

Mit diese Einstellung wird festgelegt, ob eine Handsteuerung bei aktiver Sperre am Kommunikationsobjekt "Sperre" des Kanals zugelassen ist.

<!-- DOC -->
#### Sperrzeit für Automatiken nach Handbedienung

Innerhalb der eingestellten Zeit wird ein Beschattungsstart oder der Nachtmodus verhindert.

<!-- DOC -->
### Sonderfunktionen Tasterbediendung bei geöffneter Jalousien

Die OpenKNX Jalousiensteuerung kann Fahrbefehle die normalerweise keine Auswirkung auf die Jalousie haben für Steuerbefehle benutzen.

<!-- DOC -->
### Kurzer Druck 'Nach oben'

Ist die Jalousie in der vollständig geöffneten Stellung (0%) und wird ein Eingangstelegram am Kommunikationsobjekt "Handbetrieb Stopp/Schritt" für AUF empfangen, kann dieses für folgende Funktionen benutzt werden:

- Beschattungsautomatik EIN/AUS

Die Beschattungsautomatik wird je nach vorherigem Zustand Aus, bzw. Eingeschalten. 
Damit kann mit den normalen Jalousientaster die Beschattungsautomatik mit einem kurzen Druck gesteuert werden. 
Die Einstellung ist nur sinnvoll Nutzbar wenn die Jalousiensteuerung über 2 Tasten erfolgt.

- Jalousien schließen

Ist diese Einstellung aktiv, kann bei einer 2 Tastenbedienung auch mit einem kurzer Tastendruck die Jalousie geschlossen werden. Dazu muss lediglich der "Auf" Knopf bei vollständig geöffneter Jalousie kurz betätigt werden.

<!-- DOC -->
### Langer Druck 'Nach oben'

Ist die Jalousie in der vollständig geöffneten Stellung (0%) und wird ein Eingangstelegram am Kommunikationsobjekt "Handbetrieb Auf/Ab" für AUF empfangen, kann dieses für folgende Funktionen benutzt werden:

- Beschattungsautomatik EIN/AUS

Die Beschattungsautomatik wird je nach vorherigem Zustand Aus, bzw. Eingeschalten. 
Damit kann mit den normalen Jalousientaster die Beschattungsautomatik mit einem langen Druck gesteuert werden. 
Die Einstellung ist nur sinnvoll Nutzbar wenn die Jalousiensteuerung über 2 Tasten erfolgt.

- Jalousien schließen

Ist diese Einstellung aktiv, kann bei einer 2 Tastenbedienung auch mit einem langem Tastendruck nach oben die Jalousie geschlossen werden. Somit ist egal, ob der AUF oder AB Knopf lange betätigt wird, die Jalousie schließt sich in beiden Fällen.

<!-- DOC -->
## Nachtmodus

Über diesen Modus kann die Jalousien oder der Rolladen Sonnenstand- und/oder Zeitgesteuert geöffnet und/oder geschlossen werden.

<!-- DOC -->
#### Fenster offen Modus erlaubt

Über diese Einstellung wird konfiguriert ob während des aktiven Nachtmodus die Fenster offen/gekippt Stellung verwendet wird.

<!-- DOC -->
### Nacht Begin / Nacht Ende

Beginn und Ende der Nacht kann durch folgende Ergeignisse gesteuert werden:

- Sonnenstand
- Uhrzeit
- Kommunikationsobjekt "Nachtmodus Aus-/Einschalten"

<!-- DOC -->
#### Auslöser

Zur Wahl steht:

- Kein automatischer Start/ Kein automatisches Ende  
Der Nachtmodus wird in dieser Einstellung nur durch das Kommunikationsobjekt "Nachtmodus Aus-/Einschalten" gestartet bzw. beendet.
Achtung: Ist automatisches Ende aktiv, wird eine Beschattung nur aktiv wenn zuvor das Kommunikationsobjekt "Nachtmodus Aus-/Einschalten" ein Telegram AUS empfängt.

- Uhrzeit  
Bei der eingestelltem Uhrzeit wird der Nachtmodus aktiviert/deaktiviert

- Sonne  
Bei dem eingestelltem Sonnenstand, wird der Nachtmodus aktiviert/deaktiviert

- Uhrzeit, Sonne (früheres Ereignis)
Bei der eingestellten Uhrzeit oder Sonnenstand, wird der Nachtmodus aktiviert/deaktiviert

- Uhrzeit, Sonne (späteres Ereignis)
Bei der eingestellten Uhrzeit und Sonnenstand, wird der Nachtmodus aktiviert/deaktiviert

<!-- DOC -->
#### Uhrzeit

Uhrzeit für den automatischen Beginn oder des automatischen Endes des Nachtmodus

<!-- DOC -->
#### Sonne

Direkt bei Sonnenuntergang bzw. -aufgang oder unter Angabe eines zusätzlichen Höhenwinkeloffsets wird der Nachtmodus gestartet bzw. beendet.

<!-- DOC -->
#### Höhenwinkel Offset

Um nach bzw. vor Sonnenauf bzw. Untergang den Nachtmodus zu aktiveren, wird der Höhenwinkel der Sonne über oder unter dem Horizont angegeben.

<!-- DOC -->
#### Position anfahren

Gibt an, ob bei Begin oder Ende des Nachtmodus die Jalousie automatisch bewegt werden soll. 
Wird im Anschnitt "Nacht Begin" "Nein" gewählt, wird die Jalousie nicht automatisch bewegt, jedoch wird trotzdem eine möglich Beschattung durch den Nachtmodus beginn unterbrochen.
Wird im Anschnitt "Nacht Ende" "Nein" gewählt, wird die Jalousie nicht automatisch bewegt, jedoch wird trotzdem der Beschattungsmodus ermöglicht.

<!-- DOC -->
#### Position

Nur verfügbar wenn "Position anfahren" auf "Ja" gesetzt wurde.
Gibt die Jalousienpostion an, die angefahren werden soll.

<!-- DOC -->
#### Lamellenstellung

Nur verfügbar wenn "Position anfahren" auf "Ja" gesetzt wurde und der Gerätetype "Jalousie" gewählt wurde.
Gibt die Lamellenposition an, die eingenommen werden soll.