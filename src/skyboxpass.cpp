#include "skyboxpass.hpp"

#include "annotationfree.hpp"
#include "program.hpp"
#include "shadoweddata.hpp"
#include "uniformmapimpl.hpp"

SkyboxPass::SkyboxPass(std::shared_ptr<gst::Program> program)
    : Pass(program),
      uniforms(std::make_shared<gst::UniformMapImpl>(std::unique_ptr<gst::AnnotationFormatter>(new gst::AnnotationFree())))
{
}

void SkyboxPass::apply(gst::ModelState & model_state)
{
    uniforms->get_uniform("view") = glm::toMat4(glm::toQuat(model_state.view)); // only orientation from camera
    uniforms->get_uniform("projection") = model_state.projection;
    program->merge_uniforms(*uniforms);
}
