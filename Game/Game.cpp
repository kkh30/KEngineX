
#include <VulkanRenderer/VulkanRenderer.h>

int main() {
	auto& renderer = ke::renderer::VulkanRenderer();
	renderer.Init();
	return 0;
}