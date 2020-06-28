#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <vulkan/vulkan.h>

namespace uengine::graphics {

    class Drawable {
        public:
            virtual void renderUI()=0;
            virtual void render(VkCommandBuffer cb)=0;
            virtual void resize(int32_t width, int32_t height)=0;
    };

}

#endif
