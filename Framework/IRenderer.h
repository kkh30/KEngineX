#pragma once
#include "INoCopy.h"
#include "IModule.h"
#include "FrameGraph.h"
namespace ke {
	namespace renderer {

        

        enum RendererAPI
        {
            DirextX,OGL,GLES,Vulkan
        };

		class IRenderer : public INoCopy,public IModule

		{
		public:
			IRenderer()
			{
			}

			virtual ~IRenderer()
			{
			}


            __forceinline virtual FrameGraph* GetFrameGraph() = 0;

		protected:
		};

		
	}
}