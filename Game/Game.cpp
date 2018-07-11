#include <EngineConstant.h>
#include <RendererFactory.h>

_Use_decl_annotations_
//int  WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
//{
//    D3D12HelloTriangle sample(1280, 720, L"D3D12 Hello Triangle");
//    return Win32Application::Run(&sample, hInstance, nCmdShow);
//}

int 
#ifdef _WIN32
WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
#else
main()
#endif // _WIN32
{
#ifdef _WIN32
    APP_INSTANCE = hInstance;
    CMD_SHOW = nCmdShow;
#endif
    auto& render_factory = ke::renderer::RendererFactory::GetFactory();
    auto renderer = render_factory.Create(ke::renderer::RendererAPI::DirextX);
    renderer->Init();

    renderer->GetFrameGraph();

    renderer->Update();


}