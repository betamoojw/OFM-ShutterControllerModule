### Schaltpunkte

Jeder Kanal hat 8 Schaltpunkte. Ein Schaltpunkt legt fest, an welchen Wochentagen und unter welcher Bedingung eine Stufe auslöst. Mehrere Schaltpunkte derselben Stufe sind ODER-verknüpft: Der erste erfüllte Schaltpunkt löst die Stufe aus.

- **Stufe**: Die Stufe, die der Schaltpunkt auslöst, oder "nicht aktiv".
- **Mo-So**: Die Wochentage, an denen der Schaltpunkt gilt. Für Abendstufen zählt der Tag, an dem der Nachtzyklus begonnen hat (eine Zeit nach Mitternacht am Freitag gehört noch zum Freitag). An Feiertagen gelten je nach Einstellung "Feiertage" die Einstellungen für Sonntag.
- **Auslöser**:
  - Uhrzeit
  - bei Sonnenuntergang / Sonnenaufgang
  - Sonnenuntergang / Sonnenaufgang minus bzw. plus Zeitversatz (hh:mm)
  - Ende bzw. Beginn der bürgerlichen Dämmerung (Sonne 6° unter dem Horizont)
  - Ende bzw. Beginn der nautischen Dämmerung (Sonne 12° unter dem Horizont)
  - Sonnenuntergang / Sonnenaufgang über bzw. unter Horizont (Höhenwinkel in Grad)
  - dunkler als / heller als (Lux), nur wenn "Helligkeit im Nachtmodus" verwendet wird
- **Wert**: Uhrzeit, Zeitversatz, Höhenwinkel oder Lux, je nach Auslöser.
- **Helligkeit / Lux**: Verknüpft den Auslöser innerhalb des Schaltpunkts mit der Helligkeit: "und dunkler als" (beides muss erfüllt sein) oder "oder dunkler als" (eines genügt). Morgens entsprechend "heller als".
- **Bedingung / Zeit**: "frühestens um" verhindert ein Auslösen vor dieser Zeit. "spätestens um" löst zu dieser Zeit auch dann aus, wenn Auslöser und Helligkeit noch nicht erfüllt sind.

Ausgewertet wird in dieser Reihenfolge: (Auslöser und/oder Helligkeit), danach die Bedingung.

Beispiele:

| Stufe | Tage | Auslöser | Helligkeit | Bedingung | Ergebnis |
|---|---|---|---|---|---|
| Nacht | Mo-So | bei Sonnenuntergang | oder dunkler als 20 Lux | spätestens um 22:00 | schließt bei Sonnenuntergang oder Dunkelheit, spätestens um 22:00 |
| Nacht | Mo-So | dunkler als 20 Lux | | frühestens um 17:00 | kein Schließen bei einem Gewitter am Nachmittag |
| Tag | Mo-Fr | bei Sonnenaufgang | | frühestens um 06:00 | Wochentags nicht vor 06:00 |
| Tag | Sa, So | Uhrzeit 08:30 | | | am Wochenende um 08:30 |
| Vorstufe Abend | Mo-So | Sonnenuntergang minus Zeitversatz 00:30 | | | 30 Minuten vor Sonnenuntergang |

Uhrzeiten von Abendstufen vor 12:00 gelten als "nach Mitternacht". Uhrzeiten von Morgenstufen ab 12:00 werden wie 11:59 behandelt.

