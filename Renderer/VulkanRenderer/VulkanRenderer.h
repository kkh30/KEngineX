#pragma once
#include <IModule.h>
#include "VulkanAPI.h"

namespace ke {
	namespace renderer {
		class VulkanRenderer : public IModule,public INoCopy
		{
		public:
			VulkanRenderer();
			~VulkanRenderer();
			// Inherited via IModule
			virtual void Init() override;

			virtual void Update() override;

			virtual void ShutDown() override;
		private:


			

		};

		
	}
}