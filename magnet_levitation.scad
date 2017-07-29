$fn = 200;
overflow = 0.1;

module triple (magnet_d, wall, pole_distance) {
    magnet_r = magnet_d / 2;
    sin60 = sin(60) * pole_distance;
    translate([-3,-sin60 / 3]) { 
        difference() {
            hull() {
                circle(magnet_r + wall);
                translate([pole_distance, 0]) circle(magnet_r + wall);
                translate([pole_distance / 2, sin60]) circle(magnet_r + wall);        
            }
            circle(magnet_r + overflow);
            translate([pole_distance, 0]) circle(magnet_r + overflow);
            translate([pole_distance / 2, sin60]) circle(magnet_r + overflow);
        }
    }
    %cylinder(2, 1, 1);
}


module single (magnet_d, wall) {
    magnet_r = magnet_d / 2;
    difference() {
        circle(magnet_r + wall);
        circle(magnet_r + overflow);
    }
}

module base_triangle_side (side_length, side_width) {
    height = sin(60) * side_length;
    
    union() {
        rotate(60) triple(4, 2, 6);
        difference() {
            translate([0, -side_width / 2, 0]) square([side_length, side_width]);
            hull() rotate(60) triple(4, 2, 6);
            translate([side_length, 0]) rotate(180) hull() triple(4, 2, 6);
        }
    }
}

module top_triangle_side (side_length, side_width) {
    height = sin(60) * side_length;
    
    union() {
        rotate(60) single(4, 2);
        difference() {
            translate([0, -side_width / 2, 0]) square([side_length, side_width]);
            hull() rotate(60) single(4, 2);
            translate([side_length, 0]) rotate(180) hull() triple(4, 2);
        }
    }
}


module base_triangle_2D (side_length, side_width) {
    height = sin(60) * side_length;
    
    union() {
        base_triangle_side(side_length, side_width);
        translate([side_length, 0]) rotate(120) base_triangle_side(side_length, side_width);
        translate([side_length / 2, height]) rotate(240) base_triangle_side(side_length, side_width);
    }
}

module top_triangle_2D (side_length, side_width) {
    height = sin(60) * side_length;
    
    union() {
        top_triangle_side(side_length, side_width);
        translate([side_length, 0]) rotate(120) top_triangle_side(side_length, side_width);
        translate([side_length / 2, height]) rotate(240) top_triangle_side(side_length, side_width);
    }
}

module top_triangle (side_length, side_width, height, platform = 0) {
    union() {
        if (platform != 0) {
            linear_extrude(height = platform) {
                hull() {
                    top_triangle_2D(side_length, side_width);
                }
            }
        }
        translate([0, 0, platform]) linear_extrude(height) {
            top_triangle_2D(side_length, side_width);
        }
    }
}

module base_triangle (side_length, side_width, height, platform = 0) {
    union() {
        if (platform != 0) {
            linear_extrude(height = platform) {
                hull() {
                    base_triangle_2D(side_length, side_width);
                }
            }
        }

        translate([0, 0, platform]) linear_extrude(height) {
            base_triangle_2D(side_length, side_width);
        }
    }
}

module triple_3d (height, platform = 0) {
    union() {
        if (platform != 0) {
            linear_extrude(height = platform) {
                hull() {
                    triple(4, 2, 6);
                }
            }
        }

        translate([0, 0, platform]) linear_extrude(height) {
            triple(4, 2, 6);            
        }
    }
}

//top_triangle(50, 5, 5, 1);

//base_triangle(50, 5, 5, 1);

triple_3d(3, 1);