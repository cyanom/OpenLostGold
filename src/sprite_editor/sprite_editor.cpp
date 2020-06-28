#include "sprite_editor.h"

using namespace uengine::sprite_editor;
using namespace uengine::graphics;

SpriteEditor::SpriteEditor(GraphicsBase * gb) {
    seo = new SpriteEditorOverview(gb);
}

SpriteEditor::~SpriteEditor() {
    delete seo;
}

void SpriteEditor::update() {
    //std::cout << "UPDATE" << std::endl;
    seo->update();
}

void SpriteEditor::renderUI() {
    //std::cout << "RENDERUI" << std::endl;
    seo->renderUI();
}

void SpriteEditor::render(VkCommandBuffer cb) {
    //std::cout << "RENDER" << std::endl;
    seo->render(cb);
}

void SpriteEditor::resize(int width, int height) {
    //std::cout << "RESIZE" << std::endl;
    seo->resize(width, height);
}
