    /*
 * radiation_shield.scad — Mini Stevenson screen for BMP180
 *
 * TWO PIECES:
 *   BODY = cap (top, solid) + lamels + open tube at bottom with bayonet slots
 *   PLUG = disc with cable slot + bayonet pins, locks into body from below
 *
 * Assembly:
 *   1. Put sensor inside body (from below)
 *   2. Route USB cable through plug's cable slot
 *   3. Insert plug into body from below (align pins with slots)
 *   4. Twist ~30° to lock
 *
 * Print:
 *   Body: upside down (cap on bed), no supports
 *   Plug: flat side down, no supports
 *   Material: white PETG or ASA
 */

// === RENDER MODE ===
// "body" = main shield body
// "plug" = bayonet plug (bottom cap)
// "both" = assembly view
render_mode = "body";

// === PARAMETERS ===

outer_d = 70;               // lamel outer diameter
column_d = 35;              // central column outer diameter
wall = 2.0;                 // column wall thickness

num_lamels = 5;
lamel_t = 1.6;              // lamel thickness
lamel_slope = 12;           // outer edge drop (~30°)
lamel_spacing = 10;         // between consecutive lamel inner edges

cap_t = 2.5;                // top cap thickness (solid, closed)
bottom_ext = 20;            // mm — open tube below lamels (bayonet zone)

// Plug retention
screw_d = 3.2;              // M3 through-hole in column wall
plug_screw_z = 5;           // mm from bottom — height of retaining screw

// Cable
cable_hole_d = 4;           // mm — just cable diameter (not connector)
cable_hole_offset = 2;      // mm — from edge of plug to center of hole

// Mount
mount_hole_d = 4.5;

// === DERIVED ===

outer_r = outer_d / 2;
col_r = column_d / 2;
col_inner_r = col_r - wall;

lamels_h = (num_lamels - 1) * lamel_spacing + lamel_t;
total_h = bottom_ext + lamels_h + cap_t;

$fn = 72;

// === MODULES ===

// Conical lamel — thin shell cone (open from below)
module lamel(z_inner) {
    cone_h = lamel_slope + lamel_t;  // total height of cone section
    z_base = z_inner - lamel_slope;  // bottom of outer edge

    translate([0, 0, z_base]) {
        difference() {
            // Outer cone surface
            cylinder(h=cone_h, r1=outer_r, r2=col_r + 2);
            // Inner cone (slightly smaller = shell thickness)
            translate([0, 0, -0.1])
                cylinder(h=cone_h - lamel_t + 0.1, r1=outer_r - lamel_t, r2=col_r + 2 - lamel_t);
            // Cut below column radius (clean inner edge)
            translate([0, 0, -0.1])
                cylinder(h=cone_h + 0.2, r=col_r, $fn=72);
        }
    }
}

// Mount tab — rounded end sticks OUT, flat end merges into column/lamel
module mount_tab() {
    tab_w = 16;
    tab_l = 20;
    tab_t = 3.5;
    z = total_h / 2 - tab_t / 2;

    translate([0, -(outer_r), z])
        difference() {
            hull() {
                // Rounded end (far from body = sticks out)
                translate([0, -(tab_l - tab_w/2), 0])
                    cylinder(h=tab_t, r=tab_w/2, $fn=32);
                // Flat end (merges all the way to central column)
                translate([-tab_w/2, -5, 0])
                    cube([tab_w, outer_r - col_r + 30, tab_t]);
            }
            // Two screw holes
            translate([0, -(tab_l - tab_w/2) - 3, -0.1])
                cylinder(h=tab_t+0.2, d=mount_hole_d, $fn=24);
            translate([0, -(tab_l - tab_w/2) + 5, -0.1])
                cylinder(h=tab_t+0.2, d=mount_hole_d, $fn=24);
        }
}

// =====================
// BODY (main shield)
// =====================
module body() {
    difference() {
        union() {
            // Central column (full height)
            cylinder(h=total_h, r=col_r);

            // Solid cap on top (wider = drip edge)
            translate([0, 0, total_h - cap_t])
                cylinder(h=cap_t, r=outer_r + 2);

            // Lamels (start above bottom_ext)
            for (i = [0 : num_lamels-1])
                lamel(bottom_ext + i * lamel_spacing);

            // Mount tab
            mount_tab();
        }

        // Hollow out column — OPEN AT BOTTOM, closed at top by cap
        translate([0, 0, -0.1])
            cylinder(h=total_h - cap_t + 0.1, r=col_inner_r);

        // KFS engraved into top of cap (one layer deep = 0.4mm)
        // Prints on bed = sharp lettering
        translate([0, 0, total_h - 0.4])
            linear_extrude(height=0.5)
                text("KFS", size=8, halign="center", valign="center", font="Liberation Sans:style=Bold");

        // Ventilation slots below each lamel (except the bottom one)
        // Tall narrow slots — air enters from below, rises past sensor, exits here
        slot_w = 3;             // slot width (tangential)
        slot_h = lamel_spacing - lamel_t - 2;  // slot height (leaves 1mm margin top+bottom)
        for (i = [1 : num_lamels-1])
            for (a = [0, 60, 120, 180, 240, 300])
                rotate([0, 0, a])
                    translate([-slot_w/2, col_inner_r - 1, bottom_ext + i * lamel_spacing - lamel_spacing/2 - slot_h/2 + lamel_t/2])
                        cube([slot_w, wall + 2, slot_h]);

        // Top vent slots (below cap, above top lamel)
        for (a = [0, 60, 120, 180, 240, 300])
            rotate([0, 0, a])
                translate([-slot_w/2, col_inner_r - 1, total_h - cap_t - slot_h - 1])
                    cube([slot_w, wall + 2, slot_h]);

        // Retaining screw hole (through column wall, +X side)
        // Countersunk from outside — screw head sits flush
        translate([0, 0, plug_screw_z])
            rotate([0, 90, 0])
                translate([0, 0, -1]) {
                    // Through hole ⌀3.2mm
                    cylinder(h=col_r + 2, d=screw_d, $fn=24);
                    // Countersink cone on outside (⌀6mm, 90° cone)
                    translate([0, 0, col_r - 0.5])
                        cylinder(h=2, d1=screw_d, d2=7, $fn=24);
                }
    }

    // Sensor ledges inside
    for (a = [90, 270])
        rotate([0, 0, a])
            translate([col_inner_r - 4, -4, bottom_ext + lamels_h * 0.5])
                cube([4, 8, 1.5]);
}

// =====================
// PLUG (bottom cap, retained by screw)
// =====================
module plug() {
    plug_r = col_inner_r - 0.3;  // fits inside cavity
    plug_h = 10;                  // mm — tall for heat-set insert

    difference() {
        // Solid cylinder
        cylinder(h=plug_h, r=plug_r);

        // Cable slot — round hole + open channel to edge
        // Hole ⌀4mm, 5mm from edge
        translate([0, -(plug_r - cable_hole_offset), -0.1])
            cylinder(h=plug_h + 0.2, d=cable_hole_d, $fn=24);
        // Open channel from hole to edge (so cable can slide in)
        translate([-cable_hole_d/2, -(plug_r+12), -0.1])
            cube([cable_hole_d, plug_r - cable_hole_offset + 1, plug_h + 0.2]);

        // Pilot hole for self-tapping M3 screw (⌀2.5mm, from side)
        translate([plug_r - 6, 0, plug_h/2])
            rotate([0, 90, 0])
                cylinder(h=6.5, d=2.5, $fn=24);
    }
}

// === RENDER ===

if (render_mode == "body") {
    body();
} else if (render_mode == "plug") {
    plug();
} else if (render_mode == "both") {
    color("WhiteSmoke") body();
    color("SteelBlue") translate([0, 0, -5]) plug();  // shown below body
}

// === INFO ===
echo(str("=== Radiation Shield (screw plug) ==="));
echo(str("Body height: ", total_h, " mm"));
echo(str("Diameter: ", outer_d, " mm (cap: ", outer_d+4, " mm)"));
echo(str("Cavity: ⌀", col_inner_r*2, " mm"));
echo(str("Plug: ⌀", (col_inner_r-0.3)*2, " mm disc + cable slot"));
echo(str("Retained by 1x M3 screw from side"));
echo(str("Print body upside down (cap on bed)"));
echo(str("Print plug flat side down"));
