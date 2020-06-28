#include "sprite_manager.h"

namespace fs = std::experimental::filesystem;
using namespace uengine::graphics;

SpriteManager::SpriteManager(fs::path spriteFolder, GraphicsBase * gb_) {
    gb = gb_;

    sprites = std::map<std::string, Sprite *>();
    load(spriteFolder);
}

SpriteManager::~SpriteManager() {
    for (auto& element: sprites) {
        delete element.second;
    }
}

Sprite * SpriteManager::getSprite(std::string name) {
    return sprites.find(name)->second;
}

void SpriteManager::load(fs::path folder) {
    for (auto& entry : fs::directory_iterator(folder)) {
        if (fs::is_regular_file(entry)) {
            if (entry.path().extension() == ".spr") {
                Sprite * sprite = new Sprite(gb);
                sprite->setFilename(entry.path());
                sprite->load();
                if (sprites.find(sprite->getName()) != sprites.end()) {
                    // Name already used!
                    delete sprite;
                    continue;
                }
                sprites.insert({sprite->getName(), sprite});
            }
        }

        if (fs::is_directory(entry)) {
            load(entry);
        }
    }
}
