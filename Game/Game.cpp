
#include <RendererFactory.h>

int main() {


    auto& render_factory = ke::renderer::RendererFactory::GetFactory();
    auto renderer = render_factory.Create(ke::renderer::RendererAPI::Vulkan);
    renderer->Init();
    renderer->Update();


}