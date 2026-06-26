# 3D tisk — outdoor senzorové kryty

## Postup návrhu (poučení z radiation shield)

### 1. Začni funkcí, ne tvarem
- Co chráníš? (senzor, ESP, kabel, konektor)
- Před čím? (slunce, déšť, hmyz, vítr)
- Jak se to bude montovat? (zeď, sloupek, plot, magnet)
- Jak se dovnitř dostaneš? (výměna senzoru, update firmware)

### 2. Jeden kus kde to jde
- Méně dílů = méně problémů se spojováním
- Přístup řeš odnímatelnou zátkou (plug), ne dělením celého těla
- Zátka zdola = gravitace ti pomáhá (nespadne)

### 3. Spoj — jednoduše
- **M3 šroub z boku** (heat-set insert v PETG) — spolehlivé, rozebíratelné
- Bajonet v malém průměru z plastu nefunguje dobře
- Friction-fit závisí na toleranci tiskárny — nespolehlivé

### 4. Kabel
- Kulatá díra (průměr kabelu) + otevřený kanál ke kraji
- Konektor projde před zavřením zátky — díra nemusí být na konektor
- Případně řez flexi pikou při montáži

### 5. Ventilace vs ochrana
- Skořepinové kónusy (ne plné) — lehčí, lepší proudění
- Overlap lamel musí být kladný (slunce se nedostane přímo k senzoru)
- `overlap = lamel_slope - (lamel_spacing - lamel_t)` → musí být > 0
- Otevřený spodek (pod lamelami) — vzduch cirkuluje, self-heating odchází

### 6. Tisk bez supportů
- Přesahy max 45° od horizontály
- Tisk hlavou dolů (cap/střecha na podložce) — kónusy směřují nahoru = tisknutelné
- Bílý PETG nebo ASA (UV odolnost)

### 7. Uchycení (mount tab)
- Rovná hrana zanořená do těla modelu (až k centrální rouře)
- Zaoblený konec ven (tam jsou šrouby/zip-tie)
- Dostatečná tloušťka (3–5 mm)

## Parametry k promyšlení předem

| Otázka | Proč |
|--------|------|
| Jaký senzor/deska? | Rozměr dutiny |
| Napájení odkud? | Délka kabelu, typ konektoru |
| Kde bude viset? | Typ uchycení, orientace |
| Přímé slunce? Celý den? | Počet lamel, sklon |
| Rozebíratelné? | Typ spoje (šroub/plug/permanentní) |

## OpenSCAD tipy

- `hull()` mezi dvěma prstenci = plný klín (solidní ale těžký)
- `difference()` dvou kónusů = tenká skořepina (lepší pro počasí)
- Parametrizuj vše — `outer_d`, `column_d`, `lamel_spacing` atd.
- `render_mode` proměnná pro přepínání dílů při exportu STL
- Echo na konci — rozměry, kontrola overlapu, tiskové pokyny

## Struktura souboru

```
models/
├── {název}.scad          ← parametrický zdroják
├── {název}_body.stl      ← hlavní díl
├── {název}_plug.stl      ← zátka/víko
└── DESIGN.md             ← tento soubor
```
