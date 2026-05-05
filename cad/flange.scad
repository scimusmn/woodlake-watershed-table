$fn = 50;

module hollow_pipe(od, id, h) {
  linear_extrude(height=h) {
    difference() {
      circle(d=od);
      circle(d=id);
    }
  }
}


module hollow_bend(od, id, angle) {
  rotate([0, 0, 90])
  rotate([90, 0, 0])
  translate([-od/2, 0, 0]) {
    rotate_extrude(angle=angle, convexity=10) {
      difference() {
        translate([od/2, 0, 0]) circle(d=od);
        translate([od/2, 0, 0]) circle(d=id);
      }
    }
  }
}


module hollow_bent_pipe(od, id, l1, l2, angle) {
  hollow_pipe(od, id, l1);
  translate([0, 0, l1]) hollow_bend(od, id, angle);
  translate([0,  (cos(angle)-1)*od/2,  l1+sin(angle)*od/2]) rotate([angle, 0, 0]) hollow_pipe(od, id, l2);
}


module hollow_bent_cylinder(di, do, l1, angle, l2) {
  difference() {
    bent_cylinder(do, l1, angle, l2);
    bent_cylinder(di, l1+0.5*(do-di), angle, l2+0.5*(do-di));
  }
}


TUBE_DIA = 6.5;


tube_od = 6.4;
tube_id = 3.2;


part_od = 10;


l1 = 10;
l2 = 10;
inset = 6;

angle = 90;

//hollow_pipe(od, id, l1);
//hollow_bend(od, id, 45);
module pipe_half(part_od, tube_od, tube_id, l1, l2, angle) {
  union() {
    difference() {
      union() {
        translate([0, 0, inset]) hollow_bent_pipe(part_od, tube_id, l1-inset, l2-inset, angle);
        hollow_bent_pipe(part_od, tube_od, l1, l2, angle);
      }
      translate([0, -100, 0]) cube([100, 200, 100]);
      translate([0,  (cos(angle)-1)*part_od/2,  l1+sin(angle)*part_od/2])
      rotate([angle, 0, 0])
      translate([0, 0, l2-0.5*inset])
      intersection() {
        hollow_pipe(part_od, tube_od, inset);
        cube([1, part_od, inset], center=true);
      }
    }
    intersection() {
      hollow_pipe(part_od, tube_od, inset);
      cube([1, part_od, inset], center=true);
    }
  }
}

pipe_half(part_od, tube_od, tube_id, l1, l2, angle);
// rotate([90, 0, 180]) pipe_half(part_od, tube_od, tube_id, l1, l2, angle);
