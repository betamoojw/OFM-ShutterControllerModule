### Szenen

Jedem Kanal können bis zu 16 KNX-Szenen zugeordnet werden. Eine Szene fährt die Höhe an (bei Jalousie zusätzlich die Lamelle) und kann die Sperren für Beschattungssteuerung, Nachtmodus und Handbetrieb setzen.

Aktiviert wird die Funktion mit dem Parameter "Szenen" auf der Kanalseite. Danach erscheint die Unterseite "Szenen" mit dem Kommunikationsobjekt "Szene" (DPT 17.001/18.001) und der Szenentabelle.

**Abruf und Lernen**

Über das Objekt "Szene" wird eine Szene abgerufen (DPT 17.001) oder gelernt (DPT 18.001, Bit 7 gesetzt). Beim Lernen wird die aktuelle Höhe (und Lamelle) einer als "speicherbar" markierten Szene im Gerät gespeichert und bei künftigen Abrufen anstelle der ETS-Werte angefahren. Gelernte Werte bleiben nach einem Neustart und einem erneuten ETS-Download erhalten, bis die Szene neu gelernt wird.

**Tabellenspalten**

- **freigeben**: Schaltet die Szenenzeile frei. Nur freigegebene Zeilen reagieren auf das Kommunikationsobjekt "Szene".
- **KNX-Szene**: Die KNX-Szenennummer (1–64), auf die diese Zeile reagiert.
- **speicherbar**: Legt fest, ob die Szene durch Lernen (DPT 18, Bit 7) überschrieben werden kann.
- **Höhe anfahren / Höhe**: Ist "Höhe anfahren" gesetzt, wird die danebenstehende Höhe in % angefahren. Ist es nicht gesetzt, bleibt die aktuelle Höhe unverändert.
- **Lamelle anfahren / Lamelle** (nur Jalousie): wie "Höhe anfahren/Höhe", für die Lamellenstellung.
- **Sperren**: Setzt beim Abruf die Sperren für Beschattungssteuerung, Nachtmodus und – sofern "Handbetriebseinstellung" auf eine Steuerung über den Controller eingestellt ist – zusätzlich für den Handbetrieb. Jede der drei Funktionen kann unabhängig auf "unverändert", "sperren" oder "freigeben" gesetzt werden.
- **Nach Abruf**: Legt fest, was nach dem Szenenabruf gilt.
  - *Automatik sofort frei* (Standard): Die Szene fährt die Position an, danach ist sofort wieder die Automatik zuständig – klassisches KNX-Szenenverhalten. Sperren, die diese Szene setzt, bleiben bestehen (sie wirken wie ein Sperr-Telegramm).
  - *Szene hält*: Die Szene bleibt aktiv und hält die Beschattung zurück, bis eine Handbedienung, der Nachtmodus, die Kanalsperre oder eine andere Szene sie ablöst. Sperren, die diese Szene gesetzt hat, werden beim Verlassen wieder zurückgenommen.
- **Verzögerung**: Zeit (hh:mm:ss, max. 12:00:00) zwischen Szenenabruf und Ausführung. Ein weiterer Abruf während der Verzögerung ersetzt die wartende Szene, eine Handbedienung verwirft sie.
- **Beschreibung**: Freitext nur für die ETS, ohne Speicherung im Gerät.

**Priorität und Verlassen**

Der Szenen-Modus reiht sich in die Betriebsarten wie folgt ein: Handbetrieb > Nachtmodus > **Szene** > Beschattung 2 > Beschattung 1 > Bereitschaft. Ist die Kanalsperre aktiv, wird ein Szenenabruf ignoriert.

Wie lange der Szenen-Modus aktiv bleibt, steuert die Spalte **Nach Abruf**:

- Bei *Automatik sofort frei* ist der Szenen-Modus nur für den Moment des Abrufs aktiv. Die angefahrene Position bleibt so lange stehen, bis die Automatik selbst einen Grund hat zu fahren – die Beschattung übernimmt also, sobald ihre Bedingungen erfüllt sind.
- Bei *Szene hält* bleibt der Szenen-Modus aktiv und verhindert damit, dass die Beschattung fährt. Er endet erst durch eine Handbedienung, den Start des Nachtmodus, die Kanalsperre oder den Abruf einer anderen Szene; eine dadurch unterbrochene Szene wird danach nicht fortgesetzt. Beim Verlassen wird der Zustand der von der Szene gesetzten Sperren wiederhergestellt, sofern er seit dem Szenenstart nicht durch ein Kommunikationsobjekt verändert wurde.

**Hinweis:** Wird *Szene hält* verwendet, obwohl weder Nachtmodus noch Handbedienung genutzt werden, bleibt die Beschattung dauerhaft zurückgehalten. Das Diagnoseobjekt "'Nicht erlaubt' Bits" zeigt in diesem Fall Bit 25 bzw. "'Nicht erlaubt' Grund" den Wert 26.

Solange der Szenen-Modus aktiv ist, meldet das Objekt "Aktiver Modus" den Wert 20 + Szenenplatz (Buswert = Nummer − 1), also 21–36 für die Szenenplätze 1–16. Bei *Automatik sofort frei* ist das nur kurzzeitig der Fall; das Objekt meldet danach wieder den übernehmenden Automatikmodus.

