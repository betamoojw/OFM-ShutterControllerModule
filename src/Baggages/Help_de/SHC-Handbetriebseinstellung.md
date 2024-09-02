### Handbetriebseinstellung

In diesem Abschnitt werden Optionen festgelegt, wie die Steuerung sich im Fall einer Handbedienung verhalten soll.

#### Anschluss der Gruppenadressen

Abhängig davon, ob die Handbedienung über den Aktor oder über die OpenKNX Jalousiensteuerung erfolgen soll, müssen die Gruppenadressen unterschiedlich verbunden werden.
**Wichtig** Unabhängig vom Modus müssen alle Gruppenadressen die für die Handsteuerung verwendet werden an die entsprechenden Handbedienungseingänge der OpenKNX Jalousiensteuerung verbunden werden.

#### Manuelle Bedienung über den Aktor

In dieser Einstellung erfolgt die Bedienung im Handbetrieb weiterhin direkt über den Aktor.
Diese Einstellung bietet eine erhöhte Betriebssicherheit, da bei einem Ausfall der Steuerung die Bedienung weiterhin gewährleistet ist.
Jedoch steht die Möglichkeit der Sperre der Handbedienung in diesem Modus nicht zur Verfügung.

#### Manuelle Bedienung über die OpenKNX Jalousiensteuerung ("Modul sendet AUF/AB zum Aktor" und "Modul sendet 0/100% zum Aktor")

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

