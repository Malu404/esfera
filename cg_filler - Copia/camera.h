#ifndef CAMERA_H
#define CAMERA_H

#include "ray.h"
#include "vec3.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
class camera {
public:
    camera(
        point3 lookfrom,
        point3 lookat,
        vec3 vup,
        double vfov, // campo de visão vertical em graus
        double aspect_ratio
    ) {
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        auto w = unit_vector(lookfrom - lookat);
        auto u = unit_vector(cross(vup, w));
        auto v = cross(w, u);

        origin = lookfrom;
        horizontal = viewport_width * u;
        vertical = viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - w;
    }

    ray get_ray(double s, double t) const {
        return ray(origin, lower_left_corner + s * horizontal + t * vertical - origin);
    }

private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;

    double degrees_to_radians(double degrees) const {
        return degrees * M_PI / 180.0;
    }
};

#endif