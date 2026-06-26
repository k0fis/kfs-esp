# KFS ESP SCD41 — Enclosure (3D print)

Nástěnná CO₂/teplotní sonda. Minimální rozměr, ventilační mřížka pro SCD41.

## Specifikace

| Parametr | Hodnota |
|----------|---------|
| Tvar | Plochý slot box |
| Montáž | Nástěnná (šroubek nebo oboustranná páska) |
| Materiál | PLA/PETG |
| Barva | Bílá (splyne se zdí) |

## Vnitřní komponenty

| Součást | Rozměr (mm) | Umístění |
|---------|-------------|----------|
| ESP32-C3 Super Mini | 23 × 18 × 5 | dolní část, USB-C dozadu |
| SCD41 senzor | 21 × 21 × 8 | horní část, pod mřížkou |

## Rozměry krabičky

```
Vnější: 45 × 35 × 20 mm (š × v × h)
Stěny: 1.6 mm
Vnitřní prostor: ~42 × 32 × 17 mm
```

## Pohledy

```
        FRONT (ze zdi)              SIDE
    ┌────────────────────┐      ┌──────────────┐
    │  ░░░░░░░░░░░░░░░░  │      │  ░░░░░░░░░░  │ ← mřížka
    │  ░░░ VZDUCH ░░░░░  │      │──────────────│
    │  ░░░░░░░░░░░░░░░░  │      │   SCD41      │
    │                    │      │──────────────│
    │    (plný plast)    │      │   ESP32-C3   │
    │                    │      │         ═══  │ ← USB-C
    └────────────────────┘      └──────────────┘
             45mm                     20mm

        BACK (ke zdi)               TOP
    ┌────────────────────┐      ┌──────────────┐
    │                    │      │  ░░░░░░░░░░  │
    │   ○    montáž   ○  │      │              │
    │                    │      └──────────────┘
    │       ════         │          35mm
    │      USB-C         │
    └────────────────────┘
```

## Ventilace

SCD41 potřebuje volný přístup vzduchu pro měření CO₂.
- **Přední mřížka**: výřezy 2mm široké, přes celou šířku senzoru (~20×20mm oblast)
- **Boční štěrbiny** (volitelně): 1mm sloty pro průtah
- Senzor NESMÍ být uzavřený — CO₂ se jinak hromadí a měření je nepřesné

## Montáž komponent

```
VNITŘEK (pohled zepředu, otevřeno):

    ┌────────────────────────────────┐
    │                                │
    │   ┌─────────────┐             │  ← SCD41 (slot/klip)
    │   │   SCD41     │             │
    │   │  21×21mm    │             │
    │   └─────────────┘             │
    │                                │
    │   ┌──────────────────┐        │  ← ESP32-C3 (slot/klip)
    │   │  ESP32-C3 Mini   │        │
    │   │  23×18mm    [USB]├──══    │  ← USB-C skrz stěnu
    │   └──────────────────┘        │
    │                                │
    └────────────────────────────────┘
```

### Uchycení
- **SCD41**: 2× rail/klip (zasune se z boku)
- **ESP32-C3**: 2× rail/klip + USB-C otvor jako dorazy
- **Bez šroubů** uvnitř — press-fit / snap

### Víko
- Víko = zadní stěna (ke zdi)
- Snap-fit klipsy (2×) nebo M2 šrouby (2×)
- Montážní otvor/slot pro šroubek do zdi (keyhole slot)

## Zapojení uvnitř

4 vodiče (dupont / pájené):
```
ESP32-C3 GPIO8  ──── SDA ──── SCD41
ESP32-C3 GPIO9  ──── SCL ──── SCD41
ESP32-C3 3V3    ──── VCC ──── SCD41
ESP32-C3 GND    ──── GND ──── SCD41
```

Délka vodičů: ~30mm (komponenty jsou 10mm od sebe).

## TODO

- [ ] Změřit přesné rozměry ESP32-C3 (s USB konektorem — přesah)
- [ ] OpenSCAD parametrický model
- [ ] Test print → ověřit snap-fit tolerance
- [ ] Keyhole slot pro montáž na zeď
