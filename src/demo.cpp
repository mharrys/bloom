#include "demo.hpp"

Demo::Demo(std::shared_ptr<gst::Logger> logger, std::shared_ptr<gst::Window> window)
    : logger(logger),
      window(window),
      controls(true, 2.8f, 1.5f)
{
}

bool Demo::create()
{
    auto device = std::make_shared<gst::GraphicsDeviceImpl>();
    auto synchronizer = std::make_shared<gst::GraphicsSynchronizer>(device, logger);
    auto render_state = std::make_shared<gst::RenderState>(device, synchronizer);
    renderer = gst::Renderer(device, render_state, logger);

    gst::MeshFactory mesh_factory(device, logger);
    gst::ProgramFactory program_factory(device, logger);
    gst::ProgramPool programs(program_factory);

    create_shaded_pass(programs);
    create_copy_pass(programs);
    create_effect_scene(mesh_factory);
    create_scene();
    create_suzanne(mesh_factory);
    create_light();

    window->set_pointer_lock(true);

    return true;
}

void Demo::update(float delta, float)
{
    update_input(delta);
    scene.update();

    renderer.render(scene, effect_target);
    renderer.check_errors();

    renderer.render(effect_scene);
    renderer.check_errors();
}

void Demo::destroy()
{
    window->set_pointer_lock(false);
}

void Demo::create_shaded_pass(gst::ProgramPool & programs)
{
    shaded_pass = std::make_shared<gst::ShadedPass>();
    shaded_pass->cull_face = gst::CullFace::BACK;
    shaded_pass->depth_test = true;
    shaded_pass->viewport = window->get_size();
    shaded_pass->program = programs.create(BLINNPHONG_VS, BLINNPHONG_FS);
}

void Demo::create_copy_pass(gst::ProgramPool & programs)
{
    copy_pass = std::make_shared<gst::BasicPass>();
    copy_pass->cull_face = gst::CullFace::BACK;
    copy_pass->viewport = window->get_size();
    copy_pass->program = programs.create(COPY_VS, COPY_FS);
}

void Demo::create_effect_scene(gst::MeshFactory & mesh_factory)
{
    const auto size = window->get_size();

    std::vector<unsigned char> copy_data = {};
    auto color = std::make_shared<gst::Texture2D>(size, copy_data);
    color->set_min_filter(gst::FilterMode::NEAREST);
    color->set_mag_filter(gst::FilterMode::NEAREST);
    color->set_wrap_s(gst::WrapMode::CLAMP_TO_EDGE);
    color->set_wrap_t(gst::WrapMode::CLAMP_TO_EDGE);

    auto depth = std::make_shared<gst::RenderbufferImpl>(size, gst::RenderbufferFormat::DEPTH_COMPONENT32);

    effect_target = std::make_shared<gst::FramebufferImpl>();
    effect_target->set_color({ color });
    effect_target->set_depth({ depth });

    auto formatter = std::make_shared<gst::AnnotationBasic>();
    auto uniforms = std::make_shared<gst::UniformMapImpl>(formatter);
    uniforms->get_uniform("resolution") = glm::vec2(size.get_width(), size.get_height());

    auto effect = gst::Effect(copy_pass, uniforms);
    effect.get_textures().push_back(color);

    auto quad = mesh_factory.create_quad(1.0f, 1.0f);
    auto model = std::make_shared<gst::Model>(quad, effect);
    auto model_node = std::make_shared<gst::ModelNode>(model);

    auto camera = std::make_shared<gst::OrthoCamera>();
    auto eye = std::make_shared<gst::CameraNode>(camera);

    effect_scene = gst::Scene(eye);
    effect_scene.add(model_node);
    effect_scene.update();
}

void Demo::create_scene()
{
    const auto size = window->get_size();

    auto camera = std::make_shared<gst::PerspectiveCamera>(45.0f, size, 0.1f, 1000.0f);
    auto eye = std::make_shared<gst::CameraNode>(camera);
    eye->translate_z(3.5f);

    scene = gst::Scene(eye);
}

void Demo::create_suzanne(gst::MeshFactory & mesh_factory)
{
    auto uniforms = std::make_shared<gst::UniformMapImpl>(
        std::make_shared<gst::AnnotationStruct>("material")
    );
    uniforms->get_uniform("ambient") = glm::vec3(0.2f);
    uniforms->get_uniform("diffuse") = glm::vec3(0.0f, 1.0f, 1.0f);
    uniforms->get_uniform("specular") = glm::vec3(1.0f);
    uniforms->get_uniform("emission") = glm::vec3(0.0f);
    uniforms->get_uniform("shininess") = 15.0f;

    auto effect = gst::Effect(shaded_pass, uniforms);

    for (auto mesh : mesh_factory.create_from_file(SUZANNE_OBJ)) {
        auto model = std::make_shared<gst::Model>(mesh, effect);
        auto model_node = std::make_shared<gst::ModelNode>(model);
        scene.add(model_node);
    }
}

void Demo::create_light()
{
    auto uniforms = std::make_shared<gst::UniformMapImpl>(
        std::make_shared<gst::AnnotationArray>("point_lights")
    );
    uniforms->get_uniform("ambient") = glm::vec3(0.2f);
    uniforms->get_uniform("diffuse") = glm::vec3(1.0f);
    uniforms->get_uniform("specular") = glm::vec3(1.0f);
    uniforms->get_uniform("attenuation.constant") = 1.0f;
    uniforms->get_uniform("attenuation.linear") = 0.5f;
    uniforms->get_uniform("attenuation.quadratic") = 0.03f;

    auto light = std::make_shared<gst::Light>(uniforms);
    auto light_node = std::make_shared<gst::LightNode>(light);
    light_node->position = glm::vec3(0.0f, 0.5f, 4.0f);

    scene.add(light_node);
}

void Demo::update_input(float delta)
{
    const auto input = window->get_input();

    controls.update(delta, input, *scene.get_eye());
}
