#ifndef DEMO_HPP_INCLUDED
#define DEMO_HPP_INCLUDED

#include "fragmentshader.hpp"
#include "framebuffer.hpp"
#include "mesh.hpp"
#include "perspectivecamera.hpp"
#include "orthocamera.hpp"
#include "program.hpp"
#include "renderbuffer.hpp"
#include "texture2d.hpp"
#include "vertexshader.hpp"
#include "world.hpp"

#include <memory>

class Demo : public World {
public:
    Demo(std::shared_ptr<Window> window, WindowSetting window_setting);
    bool create() override;
    void update(seconds delta, seconds elapsed, Input & input) override;
    void render(seconds delta, seconds elapsed) override;
private:
    bool create_shaders();
    void create_quad();
    void create_gaussian_blur_weights();

    void update_dimension();

    void render_pass1();
    void render_pass2();
    void render_pass3();
    void render_pass4();
    void render_pass5();

    std::shared_ptr<Window> window;
    int width;
    int height;
    int blur_width;
    int blur_height;

    VertexShader blinn_phong_vs;
    FragmentShader blinn_phong_fs;
    VertexShader texture_vs;
    FragmentShader luma_fs;

    Program texture_program;
    Program blinn_phong_program;
    Program luma_program;
    Program horizontal_blur_program;
    Program vertical_blur_program;

    PerspectiveCamera camera;
    OrthoCamera ortho_camera;

    std::unique_ptr<WorldObject> model;
    glm::vec4 light_position;

    Mesh quad;

    Texture2D texture_render;
    Texture2D texture_luma;
    Texture2D texture_vblur;
    Texture2D texture_blur;

    Renderbuffer rbo_depth;

    Framebuffer fbo_render;
    Framebuffer fbo_luma;
    Framebuffer fbo_vblur;
    Framebuffer fbo_hblur;

    float rotation_speed;
};

#endif
