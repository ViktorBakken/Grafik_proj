#pragma once

#include "rt_ray.h"
#include "rt_hitable.h"

namespace rt {

class Material {
public:
    virtual bool scatter(const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const = 0;
};

class Metal : public Material {
public:
    glm::vec3 albedo;
    float fuzz;

    Metal(const glm::vec3& a, float f = 0.0) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const override {
        glm::vec3 reflected = reflect(glm::normalize(r_in.direction()), rec.normal);
        scattered = Ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (glm::dot(scattered.direction(), rec.normal) > 0);
    }
private:
    glm::vec3 reflect(const glm::vec3& v, const glm::vec3& n) const {
        return v - 2 * glm::dot(v, n) * n;
    }

    glm::vec3 random_in_unit_sphere() const {
        glm::vec3 p;
        do {
            p = 2.0f * glm::vec3(drand48(), drand48(), drand48()) - glm::vec3(1, 1, 1);
        } while (glm::length(p) >= 1.0f);
        return p;
    }
};

// Include this in rt_material.h or your materials header file

class Lambertian : public Material {
public:
    glm::vec3 albedo;  // Base color of the material

    Lambertian(const glm::vec3& a) : albedo(a) {}

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const override {
        glm::vec3 scatter_direction = rec.normal + random_in_unit_sphere();
        // Catch degenerate scatter direction
        if (glm::length(scatter_direction) < 0.0001f) {
            scatter_direction = rec.normal;
        }
        scattered = Ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    glm::vec3 random_in_unit_sphere() const {
        glm::vec3 p;
        do {
            p = 2.0f * glm::vec3(drand48(), drand48(), drand48()) - glm::vec3(1, 1, 1);
        } while (glm::length(p) >= 1.0f);
        return p;
    }
};


} // namespace rt
