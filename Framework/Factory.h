#pragma once
#include <EngineConstant.h>
#include "INoCopy.h"
#include <memory>

namespace ke
{
   

     template<typename T>
     class Factory : public INoCopy
     {
     public:
         virtual ~Factory()
         {

         };

         Factory()
         {

         };

         template<class ...parameters>
         UniquePtr<T> Create(parameters&& ...p_parameters) = 0;

     private:
        
     };
}