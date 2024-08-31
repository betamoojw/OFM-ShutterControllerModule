# Applikationsbeschreibung Jalousiensteuerung 

Die Jalousiensteuerung bietet unterschiedliche Betriebsarten.
Jede Betriebsart kann durch Ereignisse, Eingangs- oder Messwerte zulässig sein. 
Sind meherer Betriebsarten zulässig, entscheidet die Priorität über die Auswahl.

## Betriebsarten

Folgende Betriebsarten - gereiht nach der Priorät - stehen zur Verfügung:

### Bereitschaft (Priorität 1 - niedrigste)

In dieser Betriebsart ist die Steuerung im Leerflauf und wartet auf Ereignisse die eine Wechsel der Betriebsart bewirken.

### Beschattung 1 (Priorität 2)

Dieser Modus steht nur zur Verfügung, wenn in der Kanaleinstellung unter Modus Auswahl **Beschattungsmodus Anzahl** mindestens 1 eingestellt wurde.

### Beschattung 2 (Priorität 3)

Dieser Modus steht nur zur Verfügung, wenn in der Kanaleinstellung unter Modus Auswahl **Beschattungsmodus Anzahl** 2 eingestellt wurde.

### Nachtmodus (Priorität 4)

Dieser Modus steht nur zur Verfügung, wenn in der Kanaleinstellung unter Modus Auswahl **Nachtmodus** aktiviert wurde.

Achtung: Wenn kein Automatisches Ende konfiguriert ist, muss der Nachtmodus durch Handbetrieb oder durch AUS auf den Eingang `Nachtmodus Aus-/Einschalten` deaktiviert werden, damit eine Beschattung stattfindet kann.

### Handbetrieb (Priorität 5)

In diese Betriebsart wird gewechselt, sobald eine Handbedienung über eine der Eingänge erkannt wird:

- `Handbetrieb Auf/Ab`
- `Handbetrieb Stopp/Schritt`
- `Handbetrieb Prozent`
- `Handbetrieb Lamelle Prozent` 

zusätzlich kann der Handbetrieb über den Eingang 

`Handbetrieb Aus-/Einschalten` 

manuell aktiviert bzw. deaktiviert werden.

## Fenster gekippt (Priorität 6)

Dieser Modus ist nur verfügbar, wenn 2 Fensterkontakte zur Verfügung stehen.
Der Modus kann in den meisten Betriebsartenkonfiguriationen gesperrt werden.

### Fenster offen (Priorität 7 - Höchste)

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

<!-- DOC -->
# Handbetriebseinstellung

# Anschluss der Gruppenadressen

Abhängig davon, ob die Handbedienung über den Aktor oder über die OpenKNX Jalousiensteuerung erfolgen soll, müssen die Gruppenadressen unterschiedlich verbunden werden.

## Manuelle Bedienung über den Aktor

![](img/UeberAktor.drawio.png) In dieser Einstellung erfolgt die Bedienung im Handbetrieb weiterhin direkt über den Aktor.
Diese Einstellung bietet eine erhöhte Betriebssicherheit, da bei einem Ausfall der Steuerung die Bedienung weiterhin gewährleistet ist.
Jedoch steht die Möglichkeit der Sperre der Handbedienung in diesem Modus nicht zur Verfügung.

## Manuelle Bedienung über die OpenKNX Jalousiensteuerung

![](img/UeberOpenKNX.drawio.png) In dieser Einstellung werden getrennte Gruppenadressen für die Verbindung zwischen Steuerung und Aktor benötigt. Die Jalousienaktor Ansteuerung erfolgt ausschließlich über die Steuerung, die bei Bedarf Befehle von den Tastsensoren weiterleitet.
Der Vorteil dieser Einstellung ist, das die Bedienung über die Tasten durch die Steuerung unterbunden werden kann.

# Applikationsprogram

<!-- DOC -->
## Allgemein

 (c) OpenKNX, Michael Geramb 2024

 In diesem Abschnitt werden die Basiseinstellungen der Jalousien Steuerung konfiguriert.

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
### Raumbezogene Messwert Eingänge

Auch Messwerte des Raums der Beschattet wird, können für die Entscheidung ob Beschattet werden soll oder nicht heranzgeogen werden.

<!-- DOC -->
#### Heizung

In dieser Einstellung kann festgelegt werden ob die Heizung als Messgröße verwendet wird.
Diese Einstellung ist Sinnvoll um eine Beschattung während der Heizperiode zu unterbinden.
Dabei kann der festgelegt werden, ob am KNX-Bus lediglich die Heizanforderung als Gruppenadresse zur Verfügung steht oder die aktuelle Stellgröße des Heizungsaktors.

<!-- DOC -->
### Raumtemperatur

Die Raumtemperatur kann verwendet werden um bei zu niedriger Raumtemperatur die Beschattung zu sperren damit die Sonneneinstrahlung als natürliche Wärmequelle benutzen wird.

