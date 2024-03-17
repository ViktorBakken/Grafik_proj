#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace rt {

struct RTContext {
    int width = 500;
    int height = 500;
    std::vector<glm::vec4> image;
    bool freeze = false;
    int current_frame = 0;
    int current_line = 0;
    int max_frames = 1000;
    int max_bounces = 4;
    float epsilon = 2e-4f;
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec3 ground_color = glm::vec3(0.5f, 0.8f, 0.0f);
    glm::vec3 sky_color = glm::vec3(0.5f, 0.7f, 1.0f);
    bool show_normals = false;
    bool show_mesh = false;
    // Add more settings and parameters here

    bool is_metallic_left = true;
    bool is_metallic_right = true;
    bool is_metallic_center = false;

    float fuzz_left = 0.0f;
    float fuzz_center = 0.0f;
    float fuzz_right = 0.0f;

    glm::vec3 color_left = glm::vec3(0.2f);
    glm::vec3 color_center = glm::vec3(0.5f);
    glm::vec3 color_right = glm::vec3(0.7f);

    int sample_count = 1;
};

void setupScene(RTContext &rtx, const char *mesh_filename);
void updateImage(RTContext &rtx);
void resetImage(RTContext &rtx);
void resetAccumulation(RTContext &rtx);

}  // namespace rt
