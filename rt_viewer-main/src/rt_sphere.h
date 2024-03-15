#pragma once

#include "rt_hitable.h"
#include "rt_material.h"  // Ensure Material is properly included
#include <memory> // Include for std::shared_ptr

namespace rt {

class Sphere : public Hitable {
public:
    Sphere() {}
    Sphere(const glm::vec3 &cen, float r, std::shared_ptr<Material> m)
        : center(cen), radius(r), mat_ptr(m) {};

    virtual bool hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const override {
        glm::vec3 oc = r.origin() - center;
        float a = glm::dot(r.direction(), r.direction());
        float b = glm::dot(oc, r.direction());
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - a * c;

        if (discriminant > 0) {
            float sqrtD = sqrt(discriminant);
            float temp = (-b - sqrtD) / a;  // First root
            if (temp < t_max && temp > t_min) {
                rec.t = temp;
                rec.p = r.point_at_parameter(rec.t);
                rec.normal = (rec.p - center) / radius;
                rec.mat_ptr = mat_ptr;
                return true;
            }
            temp = (-b + sqrtD) / a;  // Second root
            if (temp < t_max && temp > t_min) {
                rec.t = temp;
                rec.p = r.point_at_parameter(rec.t);
                rec.normal = (rec.p - center) / radius;
                rec.mat_ptr = mat_ptr;
                return true;
            }
        }
        return false;
    }

    glm::vec3 center;
    float radius;
    std::shared_ptr<Material> mat_ptr;  // Material pointer
};

}  // namespace rt
