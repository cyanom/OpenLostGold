#include <iostream>

#include "graphics_base.h"
#include "drawable.h"
#include "sprite_editor.h"
#include "sprite.h"
#include "sprite_manager.h"

using namespace uengine::graphics;
using namespace uengine::sprite_editor;

int main(int argc, char ** argv) {
    GraphicsBase * gb = new GraphicsBase(1200, 600);

    SpriteEditor * se = new SpriteEditor(gb);
    gb->addDrawable((Drawable *) se);

    SpriteManager * sm = new SpriteManager("res/sprites/", gb);

    while (!gb->shouldClose()) {
        gb->poolEvents();
        se->update();
        gb->draw();
    }

    delete sm;
    delete se;
    delete gb;

    return 0;
}
