#ifndef DEMO_HPP_INCLUDED
#define DEMO_HPP_INCLUDED

#include "assets.hpp"

#include "gust.hpp"

class Demo : public gst::World {
public:
    Demo(std::shared_ptr<gst::Logger> logger, std::shared_ptr<gst::Window> window);
    bool create() final;
    void update(float delta, float elapsed) final;
    void destroy() final;
private:
    void create_shaded_pass(gst::ProgramPool & programs);
    void create_copy_pass(gst::ProgramPool & programs);
    void create_effect_scene(gst::MeshFactory & mesh_factory);
    void create_scene();
    void create_suzanne(gst::MeshFactory & mesh_factory);
    void create_light();
    void update_input(float delta);

    std::shared_ptr<gst::Logger> logger;
    std::shared_ptr<gst::Window> window;

    gst::Renderer renderer;
    gst::Scene scene;
    gst::FirstPersonControl controls;

    std::shared_ptr<gst::ShadedPass> shaded_pass;
    std::shared_ptr<gst::BasicPass> copy_pass;

    gst::Scene effect_scene;
    std::shared_ptr<gst::Framebuffer> effect_target;
};

#endif
