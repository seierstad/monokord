$fn = 200;
overflow = 0.1;

module pipe (inner_diameter, outer_diameter, length) {
    ir = inner_diameter / 2;
    or = outer_diameter / 2;
    
    difference() {
        cylinder(length, or, or);
        cylinder(length, ir + overflow, ir + overflow);
    }
}

difference() {
    union() {
        cylinder(7, 5, 5);
        rotate([0, 90]) translate([-3.5, 0, -16.5]) pipe(5, 7, 33);
        rotate([0, 90]) translate([-3.5, 0, -17.5]) cylinder(1, 3.5, 3.5);
        rotate([0, 90, 90]) translate([-3.5, 0, -16.5]) pipe(5, 7, 33);
        rotate([0, 90, 90]) translate([-3.5, 0, -17.5]) cylinder(1, 3.5, 3.5);
        translate([0, 0, 7]) pipe(5, 7, 14);
        translate([0, 0, 21]) cylinder(1, 3.5, 3.5);
    }
    
    cylinder(7, 2.5 + overflow, 2.5 + overflow);
    rotate([0, 90]) translate([-3.5, 0, -5]) cylinder(10, 2.5 + overflow, 2.5 + overflow);
    rotate([0, 90, 90]) translate([-3.5, 0, -5]) cylinder(10, 2.5 + overflow, 2.5 + overflow);    
}


