#include "demo.hpp"

#include "image.hpp"
#include "meshloader.hpp"

#include "lib/gl.hpp"

#include <iostream>

Demo::Demo(std::shared_ptr<Window> window, WindowSetting window_setting)
    : window(window),
      width(window_setting.width),
      height(window_setting.height),
      blur_width(width / 2),
      blur_height(height / 2),
      light_position(0.0f, 0.5f, 4.0f, 1.0f),
      rotation_speed(1.0f)
{
}

bool Demo::create()
{
    if (!create_shaders()) {
        return false;
    }

    model = MeshLoader::load("assets/models/suzanne.obj");
    if (!model) {
        std::cerr << "Demo::on_create: unable to load model." << std::endl;
        return false;
    }
    model->update_world_transform();

    create_quad();

    camera.aspect_ratio = width / static_cast<float>(height);
    camera.translate_z(3.5f);
    camera.update_world_transform();

    texture_render.bind();
    texture_render.make(width, height);

    auto make_texture = [](Texture2D & t, int w, int h) -> void
    {
        t.mag_filter = FilterMode::NEAREST;
        t.min_filter = FilterMode::NEAREST;
        t.wrap_s = WrapMode::CLAMP_TO_EDGE;
        t.wrap_t = WrapMode::CLAMP_TO_EDGE;
        t.bind();
        t.make(w, h);
    };

    make_texture(texture_luma, width, height);
    make_texture(texture_vblur, blur_width, blur_height);
    make_texture(texture_blur, blur_width, blur_height);

    rbo_depth.width = width;
    rbo_depth.height = height;
    rbo_depth.make();

    GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };

    fbo_render.bind();
    fbo_render.attach_color(BindTarget::FRAMEBUFFER, texture_render);
    fbo_render.attach_depth(BindTarget::FRAMEBUFFER, rbo_depth);
    glDrawBuffers(1, draw_buffers);
    Framebuffer::status();

    fbo_luma.bind();
    fbo_luma.attach_color(BindTarget::FRAMEBUFFER, texture_luma);
    glDrawBuffers(1, draw_buffers);
    Framebuffer::status();

    fbo_vblur.bind();
    fbo_vblur.attach_color(BindTarget::FRAMEBUFFER, texture_vblur);
    glDrawBuffers(1, draw_buffers);
    Framebuffer::status();

    fbo_hblur.bind();
    fbo_hblur.attach_color(BindTarget::FRAMEBUFFER, texture_blur);
    glDrawBuffers(1, draw_buffers);
    Framebuffer::status();
    Framebuffer::unbind();

    create_gaussian_blur_weights();

    return true;
}

void Demo::update(seconds dt, seconds, Input & input)
{
    update_dimension();

    if (blinn_phong_fs.is_modified()) {
        blinn_phong_fs.recompile();
        blinn_phong_program.attach(blinn_phong_vs, blinn_phong_fs);
    }

    if (luma_fs.is_modified()) {
        luma_fs.recompile();
        luma_program.attach(texture_vs, luma_fs);
    }

    if (input.down(Key::LEFT)) {
        rotation_speed -= 1.0f;
    }

    if (input.down(Key::RIGHT)) {
        rotation_speed += 1.0f;
    }

    model->rotate_y(rotation_speed * dt.count());
    model->update_world_transform();
}

void Demo::render(seconds, seconds)
{
    // draw scene into texture_render using fbo_render
    render_pass1();
    // save luminance above threshold into texture_luma using fbo_luma
    render_pass2();
    // apply vertical blur on texture_luma using fbo_blur
    render_pass3();
    // apply horizontal blur on texture_luma and draw to main framebuffer
    render_pass4();
    // draw render texture from pass 1 to main framebuffer with additive
    // blending
    render_pass5();

    gl_print_error();
}

bool Demo::create_shaders()
{
    texture_vs.compile_from_file("assets/shaders/texture.vs");
    FragmentShader texture_fs;
    texture_fs.compile_from_file("assets/shaders/texture.fs");
    if (!texture_program.attach(texture_vs, texture_fs)) {
        return false;
    }

    blinn_phong_vs.compile_from_file("assets/shaders/blinn_phong.vs");
    blinn_phong_fs.compile_from_file("assets/shaders/blinn_phong.fs");
    if (!blinn_phong_program.attach(blinn_phong_vs, blinn_phong_fs)) {
        return false;
    }

    FragmentShader horizontal_blur_fs;
    horizontal_blur_fs.compile_from_file("assets/shaders/horizontal_blur.fs");
    if (!horizontal_blur_program.attach(texture_vs, horizontal_blur_fs)) {
        return false;
    }

    FragmentShader vertical_blur_fs;
    vertical_blur_fs.compile_from_file("assets/shaders/vertical_blur.fs");
    if (!vertical_blur_program.attach(texture_vs, vertical_blur_fs)) {
        return false;
    }

    luma_fs.compile_from_file("assets/shaders/luma.fs");
    if (!luma_program.attach(texture_vs, luma_fs)) {
        return false;
    }

    return true;
}

void Demo::create_quad()
{
    const float w = 1.0f;
    const float h = 1.0f;
    quad.positions = {
        glm::vec3(-w, -h, 0.0f),
        glm::vec3( w, -h, 0.0f),
        glm::vec3(-w,  h, 0.0f),
        glm::vec3( w,  h, 0.0f),
    };

    quad.indices = {
        0, 1, 2,
        2, 1, 3,
    };

    quad.update_positions = true;
    quad.update_indices = true;
}

void Demo::create_gaussian_blur_weights()
{
    const float sigma = 15.0f;
    const float sigma_sq = sigma * sigma;
    // one dimensional guassian function
    auto gaussian_blur = [sigma_sq](float i) -> float
    {
        return exp(-((i * i) / (2.0f * sigma_sq))) / sqrt(2.0f * PI * sigma_sq);
    };

    // the gaussian function is reflective around 0
    std::vector<float> weight(31);
    weight[0] = gaussian_blur(0);
    float sum = weight[0];
    for (int i = 1; i < 31; i++) {
        weight[i] = gaussian_blur(i);
        sum += 2.0f * weight[i];
    }

    // normalize the weights in order to retain the light, otherwise we could end up
    // with a darker image for a large sigma
    for (int i = 0; i < 31; i++) {
        weight[i] = weight[i] / sum;
    }

    vertical_blur_program.use();
    vertical_blur_program.set_uniform("weight", weight);

    horizontal_blur_program.use();
    horizontal_blur_program.set_uniform("weight", weight);
}

void Demo::update_dimension()
{
    auto dimension = window->dimension();
    if (width != dimension.first || height != dimension.second) {
        width = dimension.first;
        height = dimension.second;
        blur_width = width / 2;
        blur_height = height / 2;

        texture_render.bind();
        texture_render.make(width, height);

        texture_luma.bind();
        texture_luma.make(width, height);

        texture_vblur.bind();
        texture_vblur.make(blur_width, blur_height);

        texture_blur.bind();
        texture_blur.make(blur_width, blur_height);

        rbo_depth.width = width;
        rbo_depth.height = height;
        rbo_depth.make();

        camera.aspect_ratio = width / static_cast<float>(height);
    }
}

void Demo::render_pass1()
{
    glViewport(0, 0, width, height);

    fbo_render.bind();

    blinn_phong_program.use();

    glm::mat4 m = model->world_transform();
    glm::mat4 v = camera.view();
    glm::mat4 p = camera.projection();

    glm::mat4 mv = v * m;
    glm::mat4 mvp = p * mv;
    glm::mat3 nm = glm::inverseTranspose(glm::mat3(mv));

    glm::vec4 light_position_es = v * light_position;

    blinn_phong_program.set_uniform("mv", mv);
    blinn_phong_program.set_uniform("mvp", mvp);
    blinn_phong_program.set_uniform("nm", nm);
    blinn_phong_program.set_uniform("light_position", light_position_es);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model->traverse([&](WorldObject & object) {
        object.draw();
    });

    Framebuffer::status();
}

void Demo::render_pass2()
{
    luma_program.use();

    glm::mat4 v = ortho_camera.view();
    glm::mat4 p = ortho_camera.projection();
    glm::mat4 mvp = p * v;

    luma_program.set_uniform("mvp", mvp);

    fbo_luma.bind();

    texture_render.bind();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);

    quad.draw();

    glDisable(GL_BLEND);

    Framebuffer::status();
}

void Demo::render_pass3()
{
    glViewport(0, 0, blur_width, blur_height);

    vertical_blur_program.use();

    glm::mat4 v = ortho_camera.view();
    glm::mat4 p = ortho_camera.projection();
    glm::mat4 mvp = p * v;

    vertical_blur_program.set_uniform("mvp", mvp);
    vertical_blur_program.set_uniform("height", static_cast<float>(blur_height));

    fbo_vblur.bind();

    texture_luma.bind();

    glClear(GL_COLOR_BUFFER_BIT);

    quad.draw();

    Framebuffer::status();
}

void Demo::render_pass4()
{
    horizontal_blur_program.use();

    glm::mat4 v = ortho_camera.view();
    glm::mat4 p = ortho_camera.projection();
    glm::mat4 mvp = p * v;

    horizontal_blur_program.set_uniform("mvp", mvp);
    horizontal_blur_program.set_uniform("width", static_cast<float>(blur_width));

    fbo_hblur.bind();

    texture_vblur.bind();

    glClear(GL_COLOR_BUFFER_BIT);

    quad.draw();

    Framebuffer::unbind();
}

void Demo::render_pass5()
{
    glViewport(0, 0, width, height);

    texture_program.use();

    glm::mat4 v = ortho_camera.view();
    glm::mat4 p = ortho_camera.projection();
    glm::mat4 mvp = p * v;

    texture_program.set_uniform("mvp", mvp);

    glClear(GL_COLOR_BUFFER_BIT);

    texture_render.bind();
    quad.draw();

    // draw blur texture with additive blending

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    texture_blur.bind();
    quad.draw();

    glDisable(GL_BLEND);
}
