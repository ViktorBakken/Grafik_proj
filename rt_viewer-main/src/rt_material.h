#pragma once

#include "rt_ray.h"
#include "rt_hitable.h"
#include "cg_utils2.h"

namespace rt
{

    class Material
    {
    public:
        virtual bool scatter(const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const = 0;
    };

    class Metal : public Material
    {
    public:
        // Metal(const glm::vec3 &a, float f = 0.0) : albedo(a), fuzz(f < 1 ? f : 1) {}

        // virtual bool scatter(const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const override
        // {
        //     glm::vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
        //     scattered = Ray(rec.p, reflected + fuzz * cg::random_in_unit_sphere());
        //     attenuation = albedo;
        //     return (glm::dot(scattered.direction(), rec.normal) > 0);
        // }

        Metal(const glm::vec3 &a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

        bool scatter(const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered)
            const override
        {
            glm::vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
            scattered = Ray(rec.p, reflected + fuzz * cg::random_in_unit_sphere());
            attenuation = albedo;
            return (glm::dot(scattered.direction(), rec.normal) > 0);
        }

    private:
        glm::vec3 albedo;
        float fuzz;

        glm::vec3 reflect(const glm::vec3 &v, const glm::vec3 &n) const
        {
            return v - 2 * glm::dot(v, n) * n;
        }
    };

    class Lambertian : public Material
    {
    public:
        Lambertian(const glm::vec3 &a) : albedo(a) {}

        bool scatter(const Ray &r_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered)
            const override
        {
            auto scatter_direction = rec.normal + cg::random_in_unit_sphere();

            // Catch degenerate scatter direction
            if (near_zero(scatter_direction))
            {
                scatter_direction = rec.normal;
            }
            scattered = Ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }

    private:
        glm::vec3 albedo; // Base color of the material

        bool near_zero(glm::highp_vec3 e) const
        {

            // Return true if the vector is close to zero in all dimensions.
            auto s = 1e-8;
            return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
        }
    };

} // namespace rt
