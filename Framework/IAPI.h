#pragma once
#include "INoCopy.h"
#include <d3d12.h>

namespace ke {
	namespace renderer {
		class IAPI : public INoCopy
		{
		public:
			IAPI() {
			
			};
			virtual ~IAPI() {
			
			};

		protected:
			virtual void Init(bool enable_validation = false) = 0;

		};
	}
}