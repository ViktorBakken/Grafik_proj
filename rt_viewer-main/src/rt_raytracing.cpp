#include "rt_raytracing.h"
#include "rt_ray.h"
#include "rt_hitable.h"
#include "rt_sphere.h"
#include "rt_triangle.h"
#include "rt_box.h"
#include "rt_material.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "cg_utils2.h" // Used for OBJ-mesh loading
#include <stdlib.h>    // Needed for drand48()

namespace rt
{

    // Store scene (world) in a global variable for convenience
    struct Scene
    {
        Sphere ground;
        std::vector<Sphere> spheres;
        std::vector<Box> boxes;
        std::vector<Triangle> mesh;
        Box mesh_bbox;
    } g_scene;

    bool hit_world(const Ray &r, float t_min, float t_max, HitRecord &rec, RTContext &rtx)
    {
        HitRecord temp_rec;
        bool hit_anything = false;
        float closest_so_far = t_max;

        // Add a bounding volume test for the mesh
        if (rtx.show_mesh && !g_scene.mesh_bbox.hit(r, t_min, closest_so_far, temp_rec))
            return false;

        if (g_scene.ground.hit(r, t_min, closest_so_far, temp_rec))
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
        if (!rtx.show_mesh)
        {
            for (int i = 0; i < g_scene.spheres.size(); ++i)
            {
                if (g_scene.spheres[i].hit(r, t_min, closest_so_far, temp_rec))
                {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }
        }
        for (int i = 0; i < g_scene.boxes.size(); ++i)
        {
            if (g_scene.boxes[i].hit(r, t_min, closest_so_far, temp_rec))
            {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        if (rtx.show_mesh)
        {
            for (int i = 0; i < g_scene.mesh.size(); ++i)
            {
                if (g_scene.mesh[i].hit(r, t_min, closest_so_far, temp_rec))
                {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }
        }
        return hit_anything;
    }

    // This function should be called recursively (inside the function) for
    // bouncing rays when you compute the lighting for materials, like this
    //
    // if (hit_world(...)) {
    //     ...
    //     return color(rtx, r_bounce, max_bounces - 1);
    // }
    //
    // See Chapter 7 in the "Ray Tracing in a Weekend" book

    glm::vec3 color(RTContext &rtx, const Ray &r, int max_bounces)
    {
        if (max_bounces < 0)
            return glm::vec3(0.0f);

        HitRecord rec;
        if (hit_world(r, 0.001f, 9999.0f, rec, rtx))
        {

            Ray scattered;
            glm::vec3 attenuation;
            if (rtx.show_normals)
            {
                return rec.normal;
            }

            else
            {
                if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
                    return attenuation * color(rtx, scattered, max_bounces - 1);
            }

            return glm::vec3(0, 0, 0);
        }

        glm::vec3 unit_direction = glm::normalize(r.direction());
        float t = 0.5f * (unit_direction.y + 1.0f);
        return (1.0f - t) * rtx.ground_color + t * rtx.sky_color;
    }

    // MODIFY THIS FUNCTION!
    void setupScene(RTContext &rtx, const char *filename)
    {
        auto material_ground = std::make_shared<Lambertian>(rtx.ground_color);
        auto material_mesh = std::make_shared<Lambertian>(rtx.color_center);
        auto material_left = std::make_shared<Metal>(rtx.color_left, rtx.fuzz_left);
        auto material_right = std::make_shared<Metal>(rtx.color_right, rtx.fuzz_right);

        cg::OBJMesh j;
        cg::objMeshLoad(j, filename);

        for (int i = 0; i < j.indices.size(); i += 3)
        {
            auto v0 = j.vertices[j.indices[i]];
            auto v1 = j.vertices[j.indices[i + 1]];
            auto v2 = j.vertices[j.indices[i + 2]];

            g_scene.mesh.push_back(Triangle(v0, v1, v2, material_mesh));
        }

        std::cout << g_scene.mesh.size() << std::endl;

        // Compute the bounding box of the mesh
        glm::vec3 minBounds = glm::vec3(0.0f);
        glm::vec3 maxBounds = glm::vec3(0.0f);

        for (glm::vec3 &vertex : j.vertices)
        {
            minBounds = glm::min(minBounds, vertex);
            maxBounds = glm::max(maxBounds, vertex);
        }

        // Store the bounding box in the scene struct
        g_scene.mesh_bbox = Box((minBounds + maxBounds) * 0.5f, (maxBounds - minBounds) * 0.5f);

        g_scene.ground = Sphere(glm::vec3(0.0f, -1000.5f, 0.0f), 1000.0f, material_ground);
        g_scene.spheres = {
            Sphere(glm::vec3(-0.5f, 0.0f, 0.0f), 0.5f, material_left),
            Sphere(glm::vec3(0.5f, 0.0f, 0.0f), 0.5f, material_right),
        };
    }

    // MODIFY THIS FUNCTION!
    void updateLine(RTContext &rtx, int y)
    {
        int nx = rtx.width;        // Image width in pixels
        int ny = rtx.height;       // Image height in pixels
        int ns = rtx.sample_count; // Number of samples per pixel for anti-aliasing, adjust as needed for quality vs performance

        // Aspect ratio of the image
        float aspect = float(nx) / float(ny);

        // Define the viewport
        glm::vec3 lower_left_corner(-1.0f * aspect, -1.0f, -1.0f);
        glm::vec3 horizontal(2.0f * aspect, 0.0f, 0.0f);
        glm::vec3 vertical(0.0f, 2.0f, 0.0f);
        glm::vec3 origin(0.0f, 0.0f, 0.0f);

        // Inverse of the view matrix to transform rays from view space to world space
        glm::mat4 world_from_view = glm::inverse(rtx.view);

        // Parallelize loop with OpenMP for improved performance
        // #pragma omp parallel for schedule(dynamic)
        for (int x = 0; x < nx; ++x)
        {
            glm::vec3 col(0, 0, 0); // Accumulator for the color of the pixel

            // Generate multiple samples per pixel
            for (int s = 0; s < ns; s++)
            {
                // Calculate u, v coordinates with random jitter for anti-aliasing
                float u = (float(x) + drand48()) / float(nx);
                float v = (float(y) + drand48()) / float(ny);

                // Generate a ray from the camera through the pixel
                Ray r(origin, lower_left_corner + u * horizontal + v * vertical);
                r.A = glm::vec3(world_from_view * glm::vec4(r.A, 1.0f));
                r.B = glm::vec3(world_from_view * glm::vec4(r.B, 0.0f));

                // Accumulate color from this sample
                col += color(rtx, r, rtx.max_bounces);
            }

            // Average the color by the number of samples
            col /= float(ns);

            // Blend with the old image for the first frame to smoothen the transition
            if (rtx.current_frame <= 0)
            {
                glm::vec4 old = rtx.image[y * nx + x];
                rtx.image[y * nx + x] = glm::clamp(old / glm::max(1.0f, old.a), 0.0f, 1.0f);
            }

            // Update the pixel color in the image buffer
            rtx.image[y * nx + x] += glm::vec4(col, 1.0f);
        }
    }

    void updateImage(RTContext &rtx)
    {
        if (rtx.freeze)
            return;                               // Skip update
        rtx.image.resize(rtx.width * rtx.height); // Just in case...

        updateLine(rtx, rtx.current_line % rtx.height);

        if (rtx.current_frame < rtx.max_frames)
        {
            rtx.current_line += 1;
            if (rtx.current_line >= rtx.height)
            {
                rtx.current_frame += 1;
                rtx.current_line = rtx.current_line % rtx.height;
            }
        }
    }

    void resetImage(RTContext &rtx)
    {
        rtx.image.clear();
        rtx.image.resize(rtx.width * rtx.height);
        rtx.current_frame = 0;
        rtx.current_line = 0;
        rtx.freeze = false;
    }
    void updateMaterials(RTContext &rtx)
    {
        // if (rtx.is_metallic_center && rtx.show_mesh)
        // {
        //     auto material_mesh = std::make_shared<Metal>(rtx.color_center, rtx.fuzz_center);
        //     for (size_t i = 0; i < g_scene.mesh.size(); i++)
        //     {
        //         g_scene.mesh[i].mat_ptr = material_mesh;
        //     }
        // }
        // else if (rtx.is_metallic_center && rtx.show_mesh)
        // {
        //     auto material_mesh = std::make_shared<Lambertian>(rtx.color_center);
        //     for (size_t i = 0; i < g_scene.mesh.size(); i++)
        //     {
        //         g_scene.mesh[i].mat_ptr = material_mesh;
        //     }
        // }

        if (rtx.is_metallic_left)
        {
            auto material_left = std::make_shared<Metal>(rtx.color_left, rtx.fuzz_left);
            g_scene.spheres[0].mat_ptr = material_left;
        }
        else
        {
            auto material_left = std::make_shared<Lambertian>(rtx.color_left);
            g_scene.spheres[0].mat_ptr = material_left;
        }

        if (rtx.is_metallic_right)
        {
            auto material_right = std::make_shared<Metal>(rtx.color_right, rtx.fuzz_right);
            g_scene.spheres[1].mat_ptr = material_right;
        }
        else
        {
            auto material_right = std::make_shared<Lambertian>(rtx.color_right);
            g_scene.spheres[1].mat_ptr = material_right;
        }

        auto material_ground = std::make_shared<Lambertian>(rtx.ground_color);
        g_scene.ground.mat_ptr = material_ground;
    }

    void resetAccumulation(RTContext &rtx)
    {
        updateMaterials(rtx);
        rtx.current_frame = -1;
    }

} // namespace rt
