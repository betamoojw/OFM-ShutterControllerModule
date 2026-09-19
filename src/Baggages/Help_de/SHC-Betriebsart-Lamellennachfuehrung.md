### Betriebsart Lamellennachführung

Legt fest, wie die Lamellenstellung automatisch berechnet wird:

- **Tageslicht-optimiert**: Geometrische Berechnung des kritischen Kippwinkels θ, bei dem gerade kein direktes Sonnenlicht durch die Lamellen eindringt (Schattenkante). Hierfür werden Lamellenbreite und Lamellenabstand benötigt.
- **Blendschutz-optimiert**: Die Lamellen werden so gestellt, dass Sonnenstrahlen parallel reflektiert werden (θ = Profilwinkel). Maximaler Blendschutz ohne geometrische Kalibrierung.
- **Über Tabelle**: Die Lamellenstellung wird anhand einer konfigurierbaren Tabelle mit 6 Stützpunkten (Höhenwinkel → Position in %) per linearer Interpolation berechnet. Geeignet für herstellerspezifische Vorgaben (z. B. Warema-Tabellen).

