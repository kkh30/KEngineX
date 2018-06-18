#pragma once
#include "INoCopy.h"

namespace ke {
	class IModule
	{
	public:
		IModule() {
		};
		virtual ~IModule() {
		};
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual void ShutDown() = 0;
	private:

	};
}