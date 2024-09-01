### Erstes manuelles AUF ignorieren, wenn bei Beschattungstart geschlossen

Dieser Einstellung soll verwendet werden ein Taster zum gleichzeitig Öffnen von mehrere Jalousien verwendet wird, diese aber nicht alle die Beschattung benutzen.
In diesem Fall verhindert die Einstellung, dass bei aktiver Beschattung die Jalousie sich mit öffnet.
Jalousien ohne aktiver Beschattung werden aber weiterhin normal geöffnet.

Bei Handbedienungseinstellung "Manuelle Bedienung über Aktor" wird zum ignorieren des manuellen Befehls über das Kommunikationsobjekt "Stopp/Schritt Ausgang" ein Stopp gesendet. Es kann dabei trotzdem zu einer minimalen Bewegung oder einem Geräusch der Jalousie kommen. In den beiden anderen Handbedienungseinstellung "Modul sendet AUF/AB zum Aktor" und "Modul sendet 0/100% zum Aktor" tritt dieser Effekt nicht auf, da in diesem Fall der Aktor keinen Fahrbefehl erhält.

