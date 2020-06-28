#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <vector>
#include <map>
#include <experimental/filesystem>
#include <string>
#include <iostream>
#include <functional>

#include "graphics_base.h"
#include "sprite.h"

namespace uengine::graphics {

    class SpriteManager {
        public:
            SpriteManager(std::experimental::filesystem::path spriteFolder, GraphicsBase * gb);
            ~SpriteManager();

            Sprite * getSprite(std::string name);

        private:
            GraphicsBase * gb;
            std::map<std::string, Sprite *> sprites;

            void load(std::experimental::filesystem::path folder);
    };

}

#endif
