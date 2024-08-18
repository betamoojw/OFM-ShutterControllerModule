### Beschattungsmodus

Je nach Konfiguriation stehen verschieden viele Beschattungsmodus zur Verfügung. 
Die Beschattung wird abhängig von den Einstellungen wie Messwerte und Sonnenstand aktiviert. 
Erlauben die Messwerte die aktivierung von mehreren Beschattungsmodus, wird der Beschattungsmodus mit der höchsten Nummer aktiviert.

D.H. Ein Beschattungsmodus mit einer höheren Nummer sollte strengere Regeln festlegen. 
Ein typisches Beispiel wäre Beschattungsmodus 1 für einen normalen Tag zu verwenden, Beschattungsmodus 2 für einen Hitzetag. 
Dafür sollte der Temperaturgrenzwert bei Beschattungsmodus 2 höher eingestellt werden als bei Beschattungsmodus 1.

Da eine Beschattung sehr viele Regeln beinhaltet die für die aktvierung zuständig sind, wird ein Diagnose KO zur Verfügung gestellt.
Dieses beinhaltet Bit codiert die Gründe, warum die Beschattung nicht aktiviert werden kann.

Bit 0: Zeit ist nicht gültig  
Bit 1: KO "Beschattungsmodus X Aktiv" ist AUS  
Bit 2: KO "Sperre" ist EIN  
Bit 3: KO "Beschattungsmodus X Sperre" ist EIN  
Bit 4: Himmelsrichtung der Sonne (Azimut) nicht im Beschattungsbereich  
Bit 5: Höhenwinkel der Sonne (Elevation) ist zu gering  
Bit 6: Sonnen im Beschattungsunterbrechungsbereich  
Bit 7: Aktuelle Jalousienposition lässt Beschattung nicht zu
Bit 8: Wartezeit für Beschattungsstart ist aktiv  
Bit 9: Jalousie wurde manuell bewegt  
Bit 10: Fenster Offen Modus aktiv  
Bit 11: Raumtemperatur zu niedrig
Bit 12: Heizung aktiv
Bit 13: Heizung war vor zu kurzer Zeit aktiv
Bit 14: Regen
Bit 15: Zu finster  
Bit 16: Temperatur zu niedrig  
Bit 17: Vorhergesagte Temperatur zu niedrig  
Bit 18: Bewölkungsgrad zu hoch  
Bit 19: UV-Index zu niedrig  




