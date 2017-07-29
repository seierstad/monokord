$fn = 200;
flood = 0.2;

use <BOLTS.scad>;
use <threads.scad>;

module bearing (d1, d2, B) {
    r1 = d1 / 2;
    r2 = d2 / 2;
    
    difference() {
        cylinder(B, r2, r2);
        cylinder(B, r1, r1);
    }
}


rotate([0, -90, 0]) {
    axleY = cos(60) * 30;
    difference() {
        linear_extrude(height = 30) {
            hull() {
                translate([15, axleY]) circle(5);
                square([0.001, 30]);
            }
        }
        translate([15, axleY, 12]) cylinder(6, 6, 6);
        
        translate([15, axleY]) {
            //shaft hole drilling guides
            cylinder(1, 2, 0);
            translate([0, 0, 30]) rotate([0, 180, 0]) cylinder(1, 2, 0);

            //shaft hole
            cylinder(30, 2, 2);

            rotate([0, 90, 0]) {
                translate([-6, 0]) metric_thread(diameter=3, pitch = 0.5, length = 6, internal = true);
                translate([-24, 0]) metric_thread(diameter=3, pitch = 0.5, length = 6, internal = true);

            }

        }
        
    }

    %translate([15, axleY]) {
        //shaft
        cylinder(30, 2, 2);
        
        //ball bearings (SMR74-ZZ)
        translate([0, 0, 15.25]) bearing(4, 7, 2.5);
        translate([0, 0, 12.25]) bearing(4, 7, 2.5);
    }

}