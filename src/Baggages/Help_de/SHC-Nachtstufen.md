### Nachtstufen

Der Nachtmodus kennt vier Stufen:

| Stufe | Bedeutung | Fahrrichtung |
|---|---|---|
| Vorstufe Abend | optionale Zwischenstellung am Abend, z. B. 70 % / Lamelle 50 %, damit noch Restlicht hereinkommt | schließen |
| Nacht | Nachtstellung, z. B. ganz geschlossen | schließen |
| Vorstufe Morgen | optionale Zwischenstellung am Morgen | öffnen |
| Tag | Ende der Nacht, z. B. ganz geöffnet | öffnen |

Eine Stufe löst aus, sobald einer ihrer Schaltpunkte erfüllt ist. Was dabei angefahren wird, legt die Tabelle Stufen fest. Die Vorstufen sind nur aktiv, wenn ein Schaltpunkt sie verwendet; ohne solche Schaltpunkte arbeitet der Nachtmodus einstufig wie bisher.

Ablauf:

- Der Nachtmodus ist von der ersten Abendstufe bis zur Stufe "Tag" aktiv. Das Kommunikationsobjekt "Nachtmodus Aktiv" und die Einstellung "In der Nacht anders" der Fensterkontakte gelten daher schon ab der Vorstufe Abend. Die aktuelle Stufe meldet das Kommunikationsobjekt "Nachtstufe".
- Ein Nachtzyklus läuft von 12:00 bis 11:59 des Folgetags. Jede Stufe löst darin höchstens einmal aus. Abendstufen werten ab 12:00 aus (auch nach Mitternacht), Morgenstufen von 00:00 bis 11:59.
- Hat die Stufe "Nacht" bereits ausgelöst, entfällt die Vorstufe Abend in diesem Zyklus; ebenso entfällt die Vorstufe Morgen, wenn "Tag" schon ausgelöst hat.
- Das Kommunikationsobjekt "Nachtmodus Aus-/Einschalten" startet mit 1 die Stufe "Nacht" und beendet mit 0 den Nachtmodus mit der Stufe "Tag".
- Wird der Nachtmodus durch Handbetrieb oder Sperre unterbrochen, fährt er bei der Rückkehr nicht erneut. Hat in der Zwischenzeit eine neue Stufe ausgelöst, wird diese bei der Rückkehr angefahren.
- Nach einem Neustart wird die aktuelle Stufe aus den Schaltpunkten wiederhergestellt. Zwischen 12:00 und 23:59 wird sie angefahren, zwischen 00:00 und 11:59 nur übernommen, ohne zu fahren. Ein vorher über das Kommunikationsobjekt "Nachtmodus Aus-/Einschalten" geändertes Verhalten geht dabei verloren.
- Ist bei einem Fenster "'Fenster offen' Modus erlaubt" aktiv, begrenzt ein geöffnetes Fenster wie bisher die Position. Die Position der Stufe wird gemerkt und nach dem Schließen des Fensters angefahren. Ist die Einstellung deaktiviert, greift im Nachtmodus keine Aussperrverhinderung.

