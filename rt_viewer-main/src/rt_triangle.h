#pragma once

#include "rt_hitable.h"
#include "rt_material.h" // Ensure Material is properly included
#include <memory>        // Include for std::shared_ptr

namespace rt
{

    class Triangle : public Hitable
    {
    public:
        Triangle() {}
        Triangle(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, std::shared_ptr<Material> m) : v0(a), v1(b), v2(c), mat_ptr(m){};
        virtual bool hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const;

        std::shared_ptr<Material> mat_ptr; // Material pointer
        glm::vec3 v0;
        glm::vec3 v1;
        glm::vec3 v2;
    };

    // Ray-triangle test adapted from "Real-Time Collision Detection" book (pages 191--192)
    bool Triangle::hit(const Ray &r, float t_min, float t_max, HitRecord &rec) const
    {
        glm::vec3 n = glm::cross(v1 - v0, v2 - v0);
        float d = glm::dot(-r.direction(), n);
        if (d > 0.0f)
        {
            float temp = glm::dot(r.origin() - v0, n);
            if (temp >= 0.0f)
            {
                glm::vec3 e = glm::cross(-r.direction(), r.origin() - v0);
                float v = glm::dot(v2 - v0, e);
                float w = -glm::dot(v1 - v0, e);
                if (v >= 0.0f && v <= d && w >= 0.0f && v + w <= d)
                {
                    temp /= d;
                    if (temp < t_max && temp > t_min)
                    {
                        rec.t = temp;
                        rec.p = r.point_at_parameter(rec.t);
                        rec.normal = n;
                        rec.mat_ptr = mat_ptr;
                        return true;
                    }
                }
            }
        }
        return false;
    }

} // namespace rt
