#include "demo.hpp"

Demo::Demo(std::shared_ptr<gst::Logger> logger, std::shared_ptr<gst::Window> window)
    : logger(logger),
      window(window),
      composer(gst::EffectComposer::create(logger)),
      controls(true, 2.8f, 4.5f),
      programs(logger),
      render_size(window->get_size()),
      bloom_size(render_size.get_width() / 4, render_size.get_height() / 4),
      weights(5)
{
}

bool Demo::create()
{
    window->set_pointer_lock(true);

    create_textures();
    create_weights();
    create_luma();
    create_hblur();
    create_vblur();
    create_tonemap();

    create_scene();
    create_skybox();
    create_model();

    composer.set_color_format(gst::TextureFormat::RGB16F);

    return true;
}

void Demo::update(float delta, float)
{
    update_input(delta);
    scene.update();

    composer.set_size(render_size);
    composer.render(scene);
    composer.render_to_texture(render_texture);
    composer.render_filter(luma);
    composer.render_to_texture(luma_texture);

    composer.set_size(bloom_size);
    composer.render_filter(hblur, luma_texture);
    composer.render_filter(vblur);
    for (int i = 0; i < 2; i++) {
        composer.render_filter(hblur);
        composer.render_filter(vblur);
    }
    composer.render_to_texture(bloom_texture);

    composer.set_size(render_size);
    composer.render_filter(tonemap, render_texture);
    composer.render_to_screen();
}

void Demo::destroy()
{
    window->set_pointer_lock(false);
}

gst::Filter Demo::create_filter(std::string const fs_path)
{
    auto material = gst::Material::create_free();

    auto program = programs.create(COPY_VS, fs_path);
    auto pass = std::make_shared<gst::BasicPass>(program);
    pass->set_cull_face(gst::CullFace::BACK);

    return gst::Filter(material, pass);
}

void Demo::create_textures()
{
    render_texture = std::make_shared<gst::Texture2D>(gst::Texture2D::create_empty(render_size));
    render_texture->set_internal_format(gst::TextureFormat::RGB16F);
    render_texture->set_wrap_s(gst::WrapMode::CLAMP_TO_EDGE);
    render_texture->set_wrap_t(gst::WrapMode::CLAMP_TO_EDGE);

    luma_texture = std::make_shared<gst::Texture2D>(gst::Texture2D::create_empty(render_size));
    luma_texture->set_internal_format(gst::TextureFormat::RGB16F);
    luma_texture->set_wrap_s(gst::WrapMode::CLAMP_TO_EDGE);
    luma_texture->set_wrap_t(gst::WrapMode::CLAMP_TO_EDGE);

    bloom_texture = std::make_shared<gst::Texture2D>(gst::Texture2D::create_empty(bloom_size));
    bloom_texture->set_internal_format(gst::TextureFormat::RGB16F);
    bloom_texture->set_wrap_s(gst::WrapMode::CLAMP_TO_EDGE);
    bloom_texture->set_wrap_t(gst::WrapMode::CLAMP_TO_EDGE);
}

void Demo::create_weights()
{
    auto variance = 8.0f;

    auto gauss = [variance](float i)
    {
        return expf(-((i * i) / (2.0f * variance))) / sqrtf(2.0f * PI * variance);
    };

    // the gaussian function is reflective around 0
    weights[0] = gauss(0);
    auto sum = weights[0];
    for (auto i = 1u; i < weights.size(); i++) {
        weights[i] = gauss(i);
        sum += 2.0f * weights[i];
    }

    // normalize or we could end up with a darker image
    for (auto i = 0u; i < weights.size(); i++) {
        weights[i] = weights[i] / sum;
    }
}

void Demo::create_luma()
{
    luma = create_filter(LUMA_FS);
}

void Demo::create_hblur()
{
    hblur = create_filter(HBLUR_FS);
    hblur.get_uniform("weights").set_float(weights);
}

void Demo::create_vblur()
{
    vblur = create_filter(VBLUR_FS);
    vblur.get_uniform("weights").set_float(weights);
}

void Demo::create_tonemap()
{
    tonemap = create_filter(TONEMAP_FS);

    auto unit = 1;
    tonemap.get_textures()[unit] = bloom_texture;
    tonemap.get_uniform("bloom") = unit;
}

void Demo::create_scene()
{
    scene = gst::Scene::create_perspective({ 45.0f, render_size, 0.1f, 1000.0f });
    scene.get_eye().position = glm::vec3(0.0f, 1.5f, 8.0f);
}

void Demo::create_skybox()
{
    std::string path = UFFIZI_CROSS_HDR;

    gst::ImageFactory factory(logger);
    auto image = factory.create_from_file(path, false);
    auto image_width = image.get_width();
    auto pixels = image.get_float_pixels();

    // Expecting a vertical cross map, top left is (0, 0)
    //
    //  -    top      -
    // left  front    right
    // -     bottom   -
    // -     back     -

    auto face_cols = image_width;
    auto face_size = face_cols / 3u;
    auto cube_map_cols = (face_cols * 3u);
    auto cube_map_row  = cube_map_cols * face_size;
    std::vector<float> face_pixels(face_size * face_size * 3u);

    cube_map = std::make_shared<gst::TextureCube>(gst::TextureCube::create_empty(face_size));
    cube_map->set_internal_format(gst::TextureFormat::RGB16F);
    cube_map->set_wrap_s(gst::WrapMode::CLAMP_TO_EDGE);
    cube_map->set_wrap_t(gst::WrapMode::CLAMP_TO_EDGE);
    cube_map->set_wrap_r(gst::WrapMode::CLAMP_TO_EDGE);

    auto copy_face_pixels = [&](unsigned int row, unsigned int col)
    {
        auto start = cube_map_row * row + (face_cols * col);
        auto step = cube_map_cols;
        auto face_offset = 0u;
        for (auto i = 0u; i < face_size; i++) {
            auto index = start + (step * i);
            for (auto j = 0u; j < face_cols; j++) {
                face_pixels[face_offset + j] = pixels[index + j];
            }
            face_offset += face_cols;
        }
    };

    auto update_face_pixels = [&](gst::CubeFace face)
    {
        cube_map->update_data(face).set_float(face_pixels);
    };

    // top
    copy_face_pixels(0, 1);
    update_face_pixels(gst::CubeFace::POSITIVE_Y);
    // left
    copy_face_pixels(1, 0);
    update_face_pixels(gst::CubeFace::NEGATIVE_X);
    // front
    copy_face_pixels(1, 1);
    update_face_pixels(gst::CubeFace::POSITIVE_Z);
    // right
    copy_face_pixels(1, 2);
    update_face_pixels(gst::CubeFace::POSITIVE_X);
    // bottom
    copy_face_pixels(2, 1);
    update_face_pixels(gst::CubeFace::NEGATIVE_Y);
    // back, also rotate 180 degrees counterclockwise while preserving RGB order
    copy_face_pixels(3, 1);
    auto size = face_pixels.size();
    for (auto i = 0u; i < (size / 2); i += 3) {
        std::swap(face_pixels[i + 2], face_pixels[size - 1 - i]);
        std::swap(face_pixels[i + 1], face_pixels[size - 2 - i]);
        std::swap(face_pixels[i + 0], face_pixels[size - 3 - i]);
    }
    update_face_pixels(gst::CubeFace::NEGATIVE_Z);

    auto skybox_program = programs.create(SKYBOX_VS, SKYBOX_FS);
    auto skybox_pass = std::make_shared<SkyboxPass>(skybox_program);
    skybox_pass->set_cull_face(gst::CullFace::FRONT);
    skybox_pass->set_depth_mask(false);

    const auto unit = 1;
    auto material = gst::Material::create_free();
    material.get_textures()[unit] = cube_map;
    material.get_uniform("env") = unit;

    gst::MeshFactory mesh_factory(logger);
    auto mesh = mesh_factory.create_cube(10.0f);
    auto model = gst::Model(mesh, material, skybox_pass);
    auto model_node = std::make_shared<gst::ModelNode>(model);
    scene.add(model_node);
}

void Demo::create_model()
{
    auto shaded_program = programs.create(REFLECT_VS, REFLECT_FS);
    auto shaded_pass = std::make_shared<gst::ShadedPass>(shaded_program);
    shaded_pass->set_cull_face(gst::CullFace::BACK);
    shaded_pass->set_depth_test(true);

    auto material = gst::Material::create_free();

    const auto unit = 1;
    material.get_textures()[unit] = cube_map;
    material.get_uniform("env") = unit;

    gst::MeshFactory mesh_factory(logger);
    for (auto mesh : mesh_factory.create_from_file(SPHERE_OBJ)) {
        auto model = gst::Model(mesh, material, shaded_pass);
        auto model_node = std::make_shared<gst::ModelNode>(model);
        scene.add(model_node);
    }
}

void Demo::update_input(float delta)
{
    controls.update(delta, window->get_input(), scene.get_eye());
}
