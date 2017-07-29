use <futaba_3f_servo_shaft.scad>;
use <threads.scad>;

$fn = 200;

precision = 0.2;

module tunerShaft(diameter, flatWidth, length) {
    radius = diameter / 2;
    intersection () {
        cylinder(length, radius + precision, radius + precision, false);
        translate([-radius - precision, (flatWidth / -2) - precision, 0]) {
            cube([diameter + precision * 2, flatWidth + precision * 2, length]);
        }
    }
}

module futaba_f3_to_esp_bass_tuning_machine () {

    difference() {
        cylinder(20, 4, 4);
        translate([0, 0, 1/2048]) union() {
            servo_head(FUTABA_3F_SPLINE());

            translate([0, 0, 0]) {
                tunerShaft(4.5, 3.7, 20);
            }

            cylinder(10, 1.25, 1.25);
        }
        /*
        translate([0, 6, 15]) {
            rotate([90, 0, 0]) {
                metric_thread(diameter=3, pitch = 1, length = 12, internal = true);
            }
        }
        translate([0, 5, 15]) {
            rotate([90, 0, 0]) {
                cylinder(0.75, 2.5, 0.5);
            }
        }
        translate([0, -5, 15]) {
            rotate([-90, 0, 0]) {
                cylinder(0.75, 2.5, 0.5);
            }
        }
        */
    }
}

futaba_f3_to_esp_bass_tuning_machine();