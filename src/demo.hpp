#ifndef DEMO_HPP_INCLUDED
#define DEMO_HPP_INCLUDED

#include "assets.hpp"
#include "skyboxpass.hpp"

#include "gust.hpp"

class Demo : public gst::World {
public:
    Demo(std::shared_ptr<gst::Logger> logger, std::shared_ptr<gst::Window> window);
    bool create() final;
    void update(float delta, float elapsed) final;
    void destroy() final;
private:
    gst::Filter create_filter(std::string const fs_path);
    void create_textures();
    void create_weights();
    void create_luma();
    void create_hblur();
    void create_vblur();
    void create_tonemap();
    void create_scene();
    void create_skybox();
    void create_model();
    void update_input(float delta);

    std::shared_ptr<gst::Logger> logger;
    std::shared_ptr<gst::Window> window;

    gst::EffectComposer composer;
    gst::FirstPersonControl controls;
    gst::Scene scene;

    gst::ProgramPool programs;

    gst::Resolution render_size;
    gst::Resolution bloom_size;

    std::shared_ptr<gst::Texture2D> render_texture;
    std::shared_ptr<gst::Texture2D> luma_texture;
    std::shared_ptr<gst::Texture2D> bloom_texture;
    std::shared_ptr<gst::TextureCube> cube_map;

    std::vector<float> weights;

    gst::Filter luma;
    gst::Filter hblur;
    gst::Filter vblur;
    gst::Filter tonemap;
};

#endif
