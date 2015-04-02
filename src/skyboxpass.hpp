#ifndef SKYBOXPASS_HPP_INCLUDED
#define SKYBOXPASS_HPP_INCLUDED

#include "gust.hpp"

// The responsibility of this class is to setup a program object for a skybox
// pass such that the skybox will appear to always follow the eye.
class SkyboxPass : public gst::Pass {
public:
    SkyboxPass(std::shared_ptr<gst::Program> program);
    void apply(gst::ModelState & state) final;
private:
    std::shared_ptr<gst::UniformMap> uniforms;
};

#endif
