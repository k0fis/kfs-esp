/*
 * KFS ESP SCD41 — Nástěnná CO₂ sonda
 * OpenSCAD parametrický model
 *
 * Dvoudílná krabička (přední shell + zadní víko)
 * Snap-fit, ventilační mřížka, keyhole slot
 */

// ============================================================
// PARAMETRY — uprav podle reálných rozměrů
// ============================================================

// Vnější rozměry krabičky
box_w = 45;        // šířka (X)
box_h = 35;        // výška (Y)
box_d = 25;        // hloubka (Z) — kolmo na zeď

// Stěny
wall = 1.6;        // tloušťka stěn
tol = 0.3;         // tolerance pro snap-fit

// ESP32-C3 Super Mini
esp_w = 23;        // šířka PCB
esp_h = 18;        // výška PCB
esp_pcb_t = 1.6;   // tloušťka PCB
esp_usb_w = 12;     // šířka USB-C otvoru
esp_usb_h = 5.5;   // výška USB-C otvoru

// SCD41 modul
scd_w = 21;        // šířka
scd_h = 21;        // výška
scd_t = 8;         // tloušťka (výška senzoru nad PCB)
scd_pcb_t = 1.57;  // tloušťka PCB

// SCD41 montážní otvory (od levého dolního rohu PCB)
// Ze STEP modelu v2.2: rozteč 17.78 × 16.51mm, ⌀2.5mm
scd_hole_d = 2.5;        // průměr otvoru
scd_hole_x_dist = 17.78; // rozteč X (center-to-center)
scd_hole_y_dist = 16.51; // rozteč Y
scd_hole_x_off = 1.57;   // offset prvního otvoru od levé hrany
scd_hole_y_off = 2.50;   // offset prvního otvoru od spodní hrany

// Ventilační mřížka
vent_slot_w = 2;       // šířka jedné štěrbiny
vent_slot_gap = 1.5;   // mezera mezi štěrbinami (plast)
vent_area_w = 22;      // celková šířka mřížky
vent_area_h = 16;      // celková výška mřížky

// Snap-fit klipsy
clip_w = 4;        // šířka klipu
clip_h = 1.5;      // výška zarážky
clip_depth = 1;    // hloubka zarážky

// Keyhole slot (montáž na zeď)
keyhole_big_r = 4;     // velký otvor (hlavička šroubu)
keyhole_small_r = 2;   // malý slot (dřík šroubu)
keyhole_length = 6;    // délka slotu

// Nápis
logo_text = "KFS";
logo_size = 8;         // výška písma (mm)
logo_depth = 0.4;      // hloubka zápustky (= 1 vrstva, vyplní se druhou barvou)

// ============================================================
// ODVOZENÉ HODNOTY
// ============================================================

inner_w = box_w - 2*wall;
inner_h = box_h - 2*wall;
inner_d = box_d - 2*wall;

// Pozice komponent (od spodní hrany vnitřku)
esp_y_offset = 3;              // ESP32 dole
scd_y_offset = inner_h - scd_h - 2;  // SCD41 nahoře

// ============================================================
// MODULY
// ============================================================

// Přední shell (hlavní tělo)
module front_shell() {
    difference() {
        // Vnější objem
        rounded_box(box_w, box_h, box_d - wall, 2);

        // Vnitřní dutina (otevřená dozadu)
        translate([wall, wall, wall])
            cube([inner_w, inner_h, inner_d + 1]);

        // Ventilační mřížka (přední stěna, horní část)
        translate([
            (box_w - vent_area_w) / 2,
            box_h - wall - vent_area_h - 3,
            -1
        ])
            vent_grille(vent_area_w, vent_area_h, wall + 2);

        // USB-C otvor (spodní stěna, u zadní strany — kabel vede dolů podél zdi)
        translate([
            (box_w - esp_usb_w) / 2,
            -1,
            box_d - wall - esp_usb_h - 2   // u zadní stěny (ke zdi)
        ])
            cube([esp_usb_w, wall + 2, esp_usb_h]);

        // KFS logo — zapuštěné do přední stěny (0.4mm = 1 vrstva)
        // Při tisku vyplnit druhou barvou (color change nebo MMU)
        translate([box_w/2, (box_h - vent_area_h - 3) / 2, -1])
            linear_extrude(height = logo_depth + 1)
                mirror([1, 0, 0])
                    text(logo_text, size = logo_size, halign = "center", valign = "center",
                         font = "Liberation Sans:style=Bold");
    }

    // PCB rails pro ESP32-C3
    translate([wall, wall, 0])
        esp_rails();

    // PCB rails pro SCD41
    translate([wall, wall, 0])
        scd_rails();

    // Snap-fit sokety (zarážky na vnitřní straně)
    snap_sockets();
}

// Zadní víko (ke zdi)
module back_cover() {
    difference() {
        // Plochá deska
        rounded_box(box_w, box_h, wall, 2);

        // Keyhole slot
        translate([box_w/2, box_h * 0.39, -1])
            keyhole(keyhole_big_r, keyhole_small_r, keyhole_length, wall + 2);
    }

    // Snap-fit klipy (výstupky)
    translate([0, 0, wall])
        snap_clips();

    // Obrys pro zasunutí (tenký lem)
    difference() {
        translate([wall + tol, wall + tol, wall])
            cube([inner_w - 2*tol, inner_h - 2*tol, 1.5]);
        translate([wall + tol + 1, wall + tol + 1, wall])
            cube([inner_w - 2*tol - 2, inner_h - 2*tol - 2, 2]);
    }
}

// ============================================================
// POMOCNÉ MODULY
// ============================================================

module rounded_box(w, h, d, r) {
    hull() {
        for (x = [r, w-r], y = [r, h-r]) {
            translate([x, y, 0])
                cylinder(h=d, r=r, $fn=20);
        }
    }
}

module vent_grille(w, h, depth) {
    // Horizontální štěrbiny
    slot_count = floor(h / (vent_slot_w + vent_slot_gap));
    for (i = [0 : slot_count - 1]) {
        translate([0, i * (vent_slot_w + vent_slot_gap), 0])
            cube([w, vent_slot_w, depth]);
    }
}

module esp_rails() {
    // Dva L-profily po stranách ESP
    rail_h = 2;      // výška railu nad dnem
    rail_lip = 0.8;  // přesah přes PCB

    esp_x = (inner_w - esp_w) / 2;

    // Levý rail
    translate([esp_x - rail_lip, esp_y_offset, wall])
        cube([rail_lip, esp_h, rail_h]);
    translate([esp_x - rail_lip - 0.5, esp_y_offset, wall])
        cube([0.5 + rail_lip, esp_h, 0.8]);  // podpěra

    // Pravý rail
    translate([esp_x + esp_w, esp_y_offset, wall])
        cube([rail_lip, esp_h, rail_h]);
    translate([esp_x + esp_w, esp_y_offset, wall])
        cube([0.5 + rail_lip, esp_h, 0.8]);
}

module scd_rails() {
    // SCD41 — M2 standoffy s heat-set insertem
    standoff_h = 6;        // výška standoffu (mezera pod PCB pro proudění)
    standoff_od = 5.5;     // vnější průměr standoffu (musí obejmout insert)
    standoff_id = 3.2;     // vnitřní průměr — pro M2 heat-set insert (⌀3.2mm)
    // Alternativa samoředný: standoff_id = 1.7

    scd_x = (inner_w - scd_w) / 2;  // PCB offset od levé vnitřní stěny

    // 4× standoff na pozicích montážních otvorů
    hole_positions = [
        [scd_hole_x_off, scd_hole_y_off],
        [scd_hole_x_off + scd_hole_x_dist, scd_hole_y_off],
        [scd_hole_x_off, scd_hole_y_off + scd_hole_y_dist],
        [scd_hole_x_off + scd_hole_x_dist, scd_hole_y_off + scd_hole_y_dist]
    ];

    for (pos = hole_positions) {
        translate([scd_x + pos[0], scd_y_offset + pos[1], wall])
            difference() {
                cylinder(h=standoff_h, d=standoff_od, $fn=20);
                cylinder(h=standoff_h + 1, d=standoff_id, $fn=16);
            }
    }
}

module snap_sockets() {
    // 2× zarážka na vnitřní straně bočních stěn (u zadní hrany)
    snap_z = box_d - wall - 4;  // 4mm od zadní hrany

    // Levá stěna — výstupek směrem dovnitř
    translate([wall, box_h/2 - clip_w/2, snap_z])
        cube([clip_depth, clip_w, 3]);

    // Pravá stěna — výstupek směrem dovnitř
    translate([box_w - wall - clip_depth, box_h/2 - clip_w/2, snap_z])
        cube([clip_depth, clip_w, 3]);
}

module snap_clips() {
    // Výstupky na víku — pružné háčky, zapadnou za sokety v shellu
    clip_total_h = 4;  // celková výška klipu
    snap_z = box_d - wall - 4;  // musí odpovídat snap_sockets

    // Levý klip — na levé straně víka, směrem dovnitř
    translate([wall + tol, box_h/2 - clip_w/2, wall]) {
        // Nožka
        cube([clip_depth, clip_w, clip_total_h]);
        // Háček (zarážka)
        translate([0, 0, clip_total_h])
            cube([clip_depth + 0.8, clip_w, clip_h]);
    }

    // Pravý klip
    translate([box_w - wall - tol - clip_depth, box_h/2 - clip_w/2, wall]) {
        cube([clip_depth, clip_w, clip_total_h]);
        translate([-0.8, 0, clip_total_h])
            cube([clip_depth + 0.8, clip_w, clip_h]);
    }
}

module keyhole(big_r, small_r, length, depth) {
    // Klasický keyhole — kruh nahoře, slot dolů
    cylinder(h=depth, r=big_r, $fn=30);
    translate([0, -length, 0])
        cylinder(h=depth, r=small_r, $fn=20);
    translate([-small_r, -length, 0])
        cube([small_r*2, length, depth]);
}

// ============================================================
// RENDER — řízeno proměnnou `part` pro CLI export
// openscad -D 'part="shell"' -o shell.stl ...
// openscad -D 'part="cover"' -o cover.stl ...
// openscad -D 'part="all"' → obě části vedle sebe (default)
// ============================================================

part = "all";

if (part == "shell") {
    front_shell();
} else if (part == "cover") {
    back_cover();
} else {
    // "all" — obě části vedle sebe (pro náhled)
    front_shell();
    translate([box_w + 10, 0, 0])
        back_cover();
}
