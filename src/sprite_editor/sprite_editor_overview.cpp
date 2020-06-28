#include "sprite_editor_overview.h"

using namespace uengine::sprite_editor;
using namespace uengine::ui;
using GraphicsBase = uengine::graphics::GraphicsBase;
using Sprite = uengine::graphics::Sprite;
using Skin = uengine::graphics::Skin;
using Animation = uengine::graphics::Animation;
using Frame = uengine::graphics::Frame;
using FrameData = uengine::graphics::FrameData;
using SpritePreview = uengine::graphics::SpritePreview;
using GraphicsGrid = uengine::graphics::GraphicsGrid;
using GraphicsQuads = uengine::graphics::GraphicsQuads;
namespace fs = std::experimental::filesystem;

SpriteEditorOverview::SpriteEditorOverview(GraphicsBase * gb_) {
    gb = gb_;
    
    overview.mv.seor = new SpriteEditorOverviewRenderer(gb);

    fileBrowser = FileBrowser();

    resetModel();
    resetView();
    
    overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
    overview.mv.seor->setBackgroundColor(overview.mv.backgroundColor);
    overview.mv.seor->getGrid()->setExtended(overview.mv.grid.extended);
    overview.mv.seor->getGrid()->setTileSize(overview.mv.grid.unitSize);
    overview.mv.seor->getGrid()->setOffset(overview.mv.grid.offset);
    overview.mv.seor->getGrid()->setColor1(overview.mv.grid.color1);
    overview.mv.seor->getGrid()->setColor2(overview.mv.grid.color2);
    float pos1[2] = {-overview.mv.tileset.size[0] / 2, -overview.mv.tileset.size[1] / 2};
    float pos2[2] = {overview.mv.tileset.size[0] / 2, overview.mv.tileset.size[1] / 2};
    overview.mv.seor->getGrid()->setBoundaries(pos1, pos2);
    
    load("res/sprites/characters/alex.spr");
}

SpriteEditorOverview::~SpriteEditorOverview() {
    clearSpritePanelData();
    delete overview.sprite;
    delete overview.mv.seor;
}

void SpriteEditorOverview::update() {
    if (overview.sp.deleteRequest) {
        // Deleting only now because we don't want to interfere with rendering
        deleteAnimation();
        overview.sp.deleteRequest = false;
    }

    updateParameters();
    updatePreviews();
}

void SpriteEditorOverview::renderUI() {
    //ImGui::ShowDemoWindow(nullptr);
    
    showMenuBar();
    showGeneralPanel();
    showMainView();
    showSpritePanel();
}

void SpriteEditorOverview::render(VkCommandBuffer cb) {
    overview.mv.seor->render(cb);
    if (overview.sp.selectedAnimationPreview) {
        overview.sp.selectedAnimationPreview->render(cb);
    }
    for (auto preview : overview.sp.animationPreviews) {
        preview->render(cb);
    }
}

void SpriteEditorOverview::resize(int32_t width, int32_t height) {
    overview.size.x = width;
    overview.size.y = height;
    
    adjust();

    resetProjection();
    overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
    overview.mv.seor->resize(width, height);
}

void SpriteEditorOverview::updatePreviews() {
    if (overview.sp.selectedAnimationPreview) {
        overview.sp.selectedAnimationPreview->update();
    }
    for (auto preview : overview.sp.animationPreviews) {
        preview->update();
    }
}

void SpriteEditorOverview::updateCurrentSelection() {
    GraphicsQuads * quads = overview.mv.seor->getCurrentSelectionQuads();
    quads->clear();
    Rect rect = overview.mv.rect;
    quads->addRect(rect.pos[0], rect.pos[1], rect.pos[0] + rect.size[0], rect.pos[1] + rect.size[1],
                   rect.color[0], rect.color[1], rect.color[2], rect.color[3]);
    for (Rect rect : overview.mv.subRects) {
        quads->addRect(rect.pos[0], rect.pos[1], rect.pos[0] + rect.size[0], rect.pos[1] + rect.size[1],
                       rect.color[0], rect.color[1], rect.color[2], rect.color[3]);
    }
}

void SpriteEditorOverview::load(fs::path file) {
    overview.mb.fi.file = file;
    overview.mb.fi.filename = file.filename();
    if (overview.sprite) {
        delete overview.sprite;
    }
    overview.sprite = new Sprite(gb);
    std::cout << "OPENING " << file << std::endl;
    overview.sprite->setFilename(file);
    overview.sprite->load();
    overview.sprite->loadTexture(true); // true indicates we want to have access to pixel data
    overview.mv.tileset.size[0] = overview.sprite->getWidth();
    overview.mv.tileset.size[1] = overview.sprite->getHeight();
    overview.mb.fi.loaded = true;
    resetModel();
    resetView();
    
    overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
    float pos1[2] = {-overview.mv.tileset.size[0] / 2, -overview.mv.tileset.size[1] / 2};
    float pos2[2] = {overview.mv.tileset.size[0] / 2, overview.mv.tileset.size[1] / 2};
    overview.mv.seor->getGrid()->setBoundaries(pos1, pos2);

    std::cout << overview.sprite->toString() << std::endl;

    clearSpritePanelData();
    fillSpritePanelData();
}

void SpriteEditorOverview::clearSpritePanelData() {
    overview.sp.selectedSkin = nullptr;

    overview.sp.selectedAnimation = nullptr;
    if (overview.sp.selectedAnimationPreview) {
        delete overview.sp.selectedAnimationPreview;
    }
    overview.sp.selectedAnimationPreview = nullptr;
    
    overview.sp.animations.clear();
    for (auto preview : overview.sp.animationPreviews) {
        delete preview;
    }
    overview.sp.animationPreviews.clear();
}

void SpriteEditorOverview::fillSpritePanelData(Skin * skin) {
    if (!skin) {
        overview.sp.selectedSkin = overview.sprite->getSkin("");
        if (!overview.sp.selectedSkin) {
            return;
        }
    } else {
        overview.sp.selectedSkin = skin;
    }

    float backgroundColor[4] = {0, 0, 0, 0};

    overview.sp.selectedAnimation = overview.sp.selectedSkin->getAnimation("");
    if (!overview.sp.selectedAnimation) {
        return;
    } else {
        overview.sp.selectedAnimationPreview = new SpritePreview(gb, overview.sprite, 224, 224);
        overview.sp.selectedAnimationPreview->setViewProjection(glm::mat4(1.0f));
        overview.sp.selectedAnimationPreview->setBackgroundColor(backgroundColor);
        overview.sp.selectedAnimationPreview->getSpriteBox()->setSkin(overview.sp.selectedSkin);
        overview.sp.selectedAnimationPreview->getSpriteBox()->setAnimation(overview.sp.selectedAnimation);
        overview.sp.selectedAnimationPreview->update();
    }

    for (auto el : *overview.sp.selectedSkin->getAnimations()) {
        Animation * animation = el.second;
        overview.sp.animations.push_back(animation);
        SpritePreview * spritePreview = new SpritePreview(gb, overview.sprite, 32, 32);
        spritePreview->setViewProjection(glm::mat4(1.0f));
        spritePreview->setBackgroundColor(backgroundColor);
        spritePreview->getSpriteBox()->setSkin(overview.sp.selectedSkin);
        spritePreview->getSpriteBox()->setAnimation(animation);
        spritePreview->update();
        overview.sp.animationPreviews.push_back(spritePreview);
    }
}

void SpriteEditorOverview::updateParameters() {
    glm::vec3 col(overview.mv.view[0][0], overview.mv.view[0][1], overview.mv.view[0][2]);
    overview.mv.parameters.scale = glm::length(col);
    overview.mv.parameters.position[0] = overview.mv.view[3][0];
    overview.mv.parameters.position[1] = overview.mv.view[3][1];
}

void SpriteEditorOverview::resetModel() {
    overview.mv.tileset.model = glm::scale(glm::mat4(1.0f), glm::vec3((float) overview.mv.tileset.size[0] / 2.0f, (float) overview.mv.tileset.size[1] / 2.0f, 1.0f));
}

void SpriteEditorOverview::resetView() {
    overview.mv.view = glm::mat4(1.0f);
    /*
    overview.mv.view = glm::lookAt(
           glm::vec3(0.0f,0.0f,100.0f),
           glm::vec3(1000.0f,100.0f,0.0f),
           glm::vec3(0.0f,0.0f,1.0f));
    */
}

void SpriteEditorOverview::resetProjection() {
    overview.mv.projection = glm::scale(glm::mat4(1.0f),
                                        glm::vec3((float) 2.0f / overview.mv.size.x,
                                                  (float) 2.0 / overview.mv.size.y,
                                                  1.0f)) *
                             glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    /*
    overview.mv.projection =
        glm::perspective(
        glm::radians(45.0f),
        (float) overview.mv.size.x / overview.mv.size.y,
        0.1f,
        1000.0f);
    overview.mv.projection[1][1] *= -1;
    */
}

void SpriteEditorOverview::translateView(float dx, float dy, float dz) {
    overview.mv.view = glm::translate(glm::mat4(1.0), glm::vec3(dx, dy, dz)) * overview.mv.view;
}

void SpriteEditorOverview::scaleAbsoluteView(float factor) {
    overview.mv.view = glm::scale(glm::mat4(1.0), glm::vec3(factor, factor, 1.0)) * overview.mv.view;
}

void SpriteEditorOverview::scaleRelativeView(float factor) {
    overview.mv.view = glm::scale(glm::mat4(1.0), glm::vec3(1.0 + factor, 1.0 + factor, 1.0)) * overview.mv.view;
}

void SpriteEditorOverview::scaleFromPointView(float factor, float dx, float dy) {
    translateView(dx, dy, 0.0);
    scaleRelativeView(factor);
    translateView(-dx, -dy, 0.0);
}

glm::vec2 SpriteEditorOverview::screenToWorld(glm::vec2 pos) {
    glm::mat4 ivp = glm::inverse(overview.mv.projection * overview.mv.view);
    glm::vec4 z = glm::vec4(pos / (glm::vec2(overview.mv.size) / 2.0f) - 1.0f, 1.0f, 1.0f);
    z.z = -(ivp[0][2] * z.x + ivp[1][2] * z.y + ivp[3][2] * z.w) / ivp[2][2];
    glm::vec4 hpos = ivp * z;
    return hpos.xy() / hpos.w;
}

void SpriteEditorOverview::refineSelection() {
    if (!overview.sprite) {
        return;
    }

    correctSelection();
    
    if (overview.gp.sel.mode == SelectionMode_Free) {

        float x1 = (int) std::min(std::max(overview.mv.rect.pos[0] + overview.mv.tileset.size[0] / 2, 0.0f), (float) overview.mv.tileset.size[0]);
        float y1 = (int) std::min(std::max(overview.mv.rect.pos[1] + overview.mv.tileset.size[1] / 2, 0.0f), (float) overview.mv.tileset.size[1]);
        float x2 = (int) std::min(std::max(overview.mv.rect.pos[0] + overview.mv.rect.size[0] + overview.mv.tileset.size[0] / 2, 0.0f), (float) overview.mv.tileset.size[0]);
        float y2 = (int) std::min(std::max(overview.mv.rect.pos[1] + overview.mv.rect.size[1] + overview.mv.tileset.size[1] / 2, 0.0f), (float) overview.mv.tileset.size[1]);
        float w = x2 - x1;
        float h = y2 - y1;

        if (w == 0 || h == 0) {
            return;
        }

        if (overview.gp.sel.free.mode == SelectionFreeMode_None) {
            freeNone(x1, y1, x2, y2);
        }

        if (overview.gp.sel.free.mode == SelectionFreeMode_Crop) {
            freeCrop(x1, y1, x2, y2);
        }

        if (overview.gp.sel.free.mode == SelectionFreeMode_Split) {
            freeSplit(x1, y1, x2, y2);
        }

    } else if (overview.gp.sel.mode == SelectionMode_Tileset) {
        float x1 = ((int) (overview.mv.rect.pos[0] + overview.mv.tileset.size[0] / 2 - overview.gp.sel.tileset.offset[0]) / overview.gp.sel.tileset.size[0]) * overview.gp.sel.tileset.size[0] + overview.gp.sel.tileset.offset[0];
        float y1 = ((int) (overview.mv.rect.pos[1] + overview.mv.tileset.size[1] / 2 - overview.gp.sel.tileset.offset[1]) / overview.gp.sel.tileset.size[1]) * overview.gp.sel.tileset.size[1] + overview.gp.sel.tileset.offset[1];
        float x2 = ((int) (overview.mv.rect.pos[0] + overview.mv.tileset.size[0] / 2 + overview.mv.rect.size[0] - overview.gp.sel.tileset.offset[0]) / overview.gp.sel.tileset.size[0] + 1) * overview.gp.sel.tileset.size[0] + overview.gp.sel.tileset.offset[0];
        float y2 = ((int) (overview.mv.rect.pos[1] + overview.mv.tileset.size[1] / 2 + overview.mv.rect.size[1] - overview.gp.sel.tileset.offset[1]) / overview.gp.sel.tileset.size[1] + 1) * overview.gp.sel.tileset.size[1] + overview.gp.sel.tileset.offset[1];
        x1 = std::min(std::max(x1, overview.gp.sel.tileset.offset[0]), ((int) overview.mv.tileset.size[0] / overview.gp.sel.tileset.size[0] + 1) * overview.gp.sel.tileset.size[0] - overview.gp.sel.tileset.offset[0]);
        y1 = std::min(std::max(y1, overview.gp.sel.tileset.offset[1]), ((int) overview.mv.tileset.size[1] / overview.gp.sel.tileset.size[1] + 1) * overview.gp.sel.tileset.size[1] - overview.gp.sel.tileset.offset[1]);
        x2 = std::min(std::max(x2, overview.gp.sel.tileset.offset[0]), ((int) (overview.mv.tileset.size[0] - overview.gp.sel.tileset.offset[0]) / overview.gp.sel.tileset.size[0]) * overview.gp.sel.tileset.size[0] + overview.gp.sel.tileset.offset[0]);
        y2 = std::min(std::max(y2, overview.gp.sel.tileset.offset[1]), ((int) (overview.mv.tileset.size[1] - overview.gp.sel.tileset.offset[1]) / overview.gp.sel.tileset.size[1]) * overview.gp.sel.tileset.size[1] + overview.gp.sel.tileset.offset[1]);
        if (overview.gp.sel.tileset.crop) {
            tilesetCrop(x1, y1, x2, y2);
        } else {
            tilesetNoCrop(x1, y1, x2, y2);
        }
    }
    updateCurrentSelection();
}

void SpriteEditorOverview::correctSelection() {
    overview.mv.rect.size[0] = overview.mv.cur2[0] - overview.mv.cur1[0];
    overview.mv.rect.pos[0] = overview.mv.cur1[0];
    if (overview.mv.rect.size[0] < 0) {
        overview.mv.rect.size[0] = -overview.mv.rect.size[0];
        overview.mv.rect.pos[0] = overview.mv.cur2[0];
    }
    overview.mv.rect.size[1] = overview.mv.cur2[1] - overview.mv.cur1[1];
    overview.mv.rect.pos[1] = overview.mv.cur1[1];
    if (overview.mv.rect.size[1] < 0) {
        overview.mv.rect.size[1] = -overview.mv.rect.size[1];
        overview.mv.rect.pos[1] = overview.mv.cur2[1];
    }
}

void SpriteEditorOverview::freeNone(float x1, float y1, float x2, float y2) {
    Rect rect = {
        {(float) x1 - overview.mv.tileset.size[0] / 2, (float) y1 - overview.mv.tileset.size[1] / 2},
        {(float) x2 - x1, (float) y2 - y1},
        {1, 1, 0, 1}
    };
    overview.mv.subRects.clear();
    overview.mv.subRects.push_back(rect);
}

bool SpriteEditorOverview::emptyColumn(float y1, float y2, float i) {
    for (int j = y1; j < y2; j++) {
        uint8_t * p = overview.sprite->getPixel(i, j);
        if (*(p + 3)) {
            return false;
        }
    }
    return true;
}

bool SpriteEditorOverview::emptyRow(float x1, float x2, float j) {
    for (int i = x1; i < x2; i++) {
        uint8_t * p = overview.sprite->getPixel(i, j);
        if (*(p + 3)) {
            return false;
        }
    }
    return true;
}

void SpriteEditorOverview::freeCrop(float x1, float y1, float x2, float y2) {
    int r1;
    for (r1 = y1; emptyRow(x1, x2, r1) && r1 < y2; r1++);
    if (r1 == y2)
        return;
    int r2;
    for (r2 = y2 - 1; emptyRow(x1, x2, r2) && r2 >= y1; r2--);
    r2++;
    
    int c1;
    for (c1 = x1; emptyColumn(y1, y2, c1) && c1 < x2; c1++);
    if (c1 == x2)
        return;
    int c2;
    for (c2 = x2 - 1; emptyColumn(y1, y2, c2) && c2 >= x1; c2--);
    c2++;

    Rect rect = {
        {(float) c1 - overview.mv.tileset.size[0] / 2, (float) r1 - overview.mv.tileset.size[1] / 2},
        {(float) c2 - c1, (float) r2 - r1},
        {1, 1, 0, 1}
    };
    overview.mv.subRects.clear();
    overview.mv.subRects.push_back(rect);
}

void SpriteEditorOverview::freeSplit(float x1, float y1, float x2, float y2) {
    overview.mv.subRects.clear();
    
    int c1, c2, nb = 0;
    bool previousEmpty = true;
    for (int i = x1; i < x2; i++) {
        bool empty = emptyColumn(y1, y2, i);
        if (!empty && previousEmpty) {
            c1 = i;
        }
        if ((empty && !previousEmpty) || (i == x2-1 && !empty)) {
            c2 = i + (i == x2-1 && !empty);

            int r1;
            for (r1 = y1; emptyRow(c1, c2, r1) && r1 < y2; r1++);
            if (r1 == x2)
                return;
            int r2;
            for (r2 = y2 - 1; emptyRow(c1, c2, r2) && r2 >= y1; r2--);
            r2++;
            
            Rect rect = {
                {(float) c1 - overview.mv.tileset.size[0] / 2, (float) r1 - overview.mv.tileset.size[1] / 2},
                {(float) c2 - c1, (float) r2 - r1},
                {1, 1, 0, 1}
            };
            overview.mv.subRects.push_back(rect);       
        }
        
        previousEmpty = empty;
    }
}

void SpriteEditorOverview::tilesetNoCrop(float x1, float y1, float x2, float y2) {
    overview.mv.subRects.clear();

    for (int j = y1; j < y2; j += overview.gp.sel.tileset.size[1]) {
        for (int i = x1; i < x2; i += overview.gp.sel.tileset.size[0]) {
            Rect rect = {
                {(float) i - overview.mv.tileset.size[0] / 2, (float) j - overview.mv.tileset.size[1] / 2},
                {(float) overview.gp.sel.tileset.size[0], (float) overview.gp.sel.tileset.size[1]},
                {1, 1, 0, 1}
            };
            overview.mv.subRects.push_back(rect);
        }
    }
}

void SpriteEditorOverview::tilesetCrop(float x1, float y1, float x2, float y2) {
    overview.mv.subRects.clear();

    for (int j = y1; j < y2; j += overview.gp.sel.tileset.size[1]) {
        for (int i = x1; i < x2; i += overview.gp.sel.tileset.size[0]) {
            int r1;
            for (r1 = j; emptyRow(i, i + overview.gp.sel.tileset.size[0], r1) && r1 < j + overview.gp.sel.tileset.size[1]; r1++);
            if (r1 >= j + overview.gp.sel.tileset.size[1])
                continue;
            int r2;
            for (r2 = j + overview.gp.sel.tileset.size[1] - 1; emptyRow(i, i + overview.gp.sel.tileset.size[0], r2) && r2 >= j; r2--);
            r2++;
            
            int c1;
            for (c1 = i; emptyColumn(j, j + overview.gp.sel.tileset.size[1], c1) && c1 < i + overview.gp.sel.tileset.size[0]; c1++);
            if (c1 >= i + overview.gp.sel.tileset.size[0])
                continue;
            int c2;
            for (c2 = i + overview.gp.sel.tileset.size[0] - 1; emptyColumn(j, j + overview.gp.sel.tileset.size[1], c2) && c2 >= i; c2--);
            c2++;
            
            Rect rect = {
                {(float) c1 - overview.mv.tileset.size[0] / 2, (float) r1 - overview.mv.tileset.size[1] / 2},
                {(float) c2 - c1, (float) r2 - r1},
                {1, 1, 0, 1}
            };
            overview.mv.subRects.push_back(rect);
        }
    }
}

void SpriteEditorOverview::newAnimation() {
    std::string animationName = overview.sp.newAnimationName;
    Animation * animation = new Animation();
    animation->setName(animationName);

    for (auto rect : overview.mv.subRects) {
        Frame * frame = new Frame();
        FrameData frameData = {
            glm::make_vec2(rect.pos) + glm::vec2((float) overview.mv.tileset.size[0], (float) overview.mv.tileset.size[1]) / 2.0f,
            glm::make_vec2(rect.size),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(glm::make_vec2(rect.size) / glm::make_vec2(overview.gp.ingameUnitSize), 1.0f),
            0.25
        };
        frame->setData(frameData);
        
        animation->addFrame(frame);
    }
    
    overview.sp.selectedSkin->addAnimation(animation);
    std::cout << overview.sprite->toString() << std::endl;

    overview.sp.animations.push_back(animation);
    SpritePreview * spritePreview = new SpritePreview(gb, overview.sprite, 32, 32);
    spritePreview->setViewProjection(glm::mat4(1.0f));
    spritePreview->getSpriteBox()->setSkin(overview.sp.selectedSkin);
    spritePreview->getSpriteBox()->setAnimation(animation);
    spritePreview->update();
    overview.sp.animationPreviews.push_back(spritePreview);
    
    if (!overview.sp.selectedAnimation) { // Create main preview if not already created
        overview.sp.selectedAnimationPreview = new SpritePreview(gb, overview.sprite, 224, 224);
        overview.sp.selectedAnimationPreview->setViewProjection(glm::mat4(1.0f));
        overview.sp.selectedAnimationPreview->getSpriteBox()->setSkin(overview.sp.selectedSkin);
        overview.sp.selectedAnimationPreview->getSpriteBox()->setAnimation(animation);
        overview.sp.selectedAnimationPreview->update();
    } else {
        overview.sp.selectedAnimationPreview->getSpriteBox()->mimic(spritePreview->getSpriteBox());
    }
    overview.sp.selectedAnimation = animation;
}

void SpriteEditorOverview::renameAnimation() {
    overview.sp.selectedSkin->renameAnimation(overview.sp.selectedAnimation->getName(), overview.sp.newAnimationRename);
}

void SpriteEditorOverview::deleteAnimation() {
    overview.sp.selectedSkin->removeAnimation(overview.sp.selectedAnimation);
    Skin * skin = overview.sp.selectedSkin;

    clearSpritePanelData();
    fillSpritePanelData(skin);
}

void SpriteEditorOverview::adjust() {
    overview.gp.pos.x = 0;
    overview.gp.pos.y = overview.mb.size.y;
    overview.gp.size.x = 256;
    overview.gp.size.y = overview.size.y - overview.mb.size.y;
    
    overview.sp.pos.x = overview.size.x - 256;
    overview.sp.pos.y = overview.mb.size.y;
    overview.sp.size.x = 256;
    overview.sp.size.y = overview.size.y - overview.mb.size.y;

    overview.mv.pos.x = overview.gp.size.x;
    overview.mv.pos.y = overview.mb.size.y;
    overview.mv.size.x = std::max(overview.size.x - overview.gp.size.x - overview.sp.size.x, 256.0f);
    overview.mv.size.y = std::max(overview.size.y - overview.mb.size.y, 256.0f);
}

void SpriteEditorOverview::showMenuBar() {
    // Main menu bar
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File"))
        {
            showMenuBarMenu();
            ImGui::EndMenu();
        }
        overview.mb.size.y = ImGui::GetWindowSize().y;
        adjust();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleVar();

    fileBrowser.display();
    if (fileBrowser.hasSelected())
        load(fileBrowser.getSelected());
}

void SpriteEditorOverview::showMenuBarMenu() {
    // Main menu bar menu
    if (ImGui::MenuItem("Open")) {
        fileBrowser.setExtensions({".jpg", ".png", ".spr"});
        fileBrowser.open();
    }
    if (ImGui::MenuItem("Save")) {}
}

void SpriteEditorOverview::showGeneralPanel() {
    // General panel
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    
    ImGui::SetNextWindowPos(overview.gp.pos, 0);
    ImGui::SetNextWindowSize(overview.gp.size, 0);

    ImGui::Begin("general panel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    {

        ImGui::Text("Current file:\n%s", (overview.mb.fi.loaded ? overview.mb.fi.filename.c_str() : "None"));
        

        ImGui::Separator();
        
        ImGui::Text("In-Game unit size");
        ImGui::PushID("Free options");
        if (ImGui::InputFloat2("", overview.gp.ingameUnitSize)) {}
        ImGui::PopID();

        ImGui::Separator();

        ImGui::Text("View:");
        if (ImGui::Button("Reset view")) {
            resetView();
            overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
        }
        if (ImGui::InputFloat("Scale", &overview.mv.parameters.scale)) {
            resetView();
            scaleAbsoluteView(overview.mv.parameters.scale);
            translateView(overview.mv.parameters.position[0], overview.mv.parameters.position[1]);
            overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
        }
        if (ImGui::InputFloat2("Position", overview.mv.parameters.position)) {
            resetView();
            scaleAbsoluteView(overview.mv.parameters.scale);
            translateView(overview.mv.parameters.position[0], overview.mv.parameters.position[1]);
            overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
        }
        if (ImGui::ColorEdit4("BG color", overview.mv.backgroundColor)) {
            overview.mv.seor->setBackgroundColor(overview.mv.backgroundColor);
        }


        ImGui::Separator();
        
        ImGui::Text("Grid:");
        if (ImGui::Checkbox("Extended", &overview.mv.grid.extended)) {
            overview.mv.seor->getGrid()->setExtended(overview.mv.grid.extended);
        }
        if (ImGui::InputFloat2("Unit size", overview.mv.grid.unitSize)) {
            if (overview.gp.sel.tileset.tiedToGrid) {
                overview.gp.sel.tileset.size[0] = overview.mv.grid.unitSize[0];
                overview.gp.sel.tileset.size[1] = overview.mv.grid.unitSize[1];
                refineSelection();
                overview.mv.currentSelectionChanged = true;
            }
            overview.mv.seor->getGrid()->setTileSize(overview.mv.grid.unitSize);
        }
        if (ImGui::InputFloat2("Offset", overview.mv.grid.offset)) {
            if (overview.gp.sel.tileset.tiedToGrid) {
                overview.gp.sel.tileset.offset[0] = overview.mv.grid.offset[0];
                overview.gp.sel.tileset.offset[1] = overview.mv.grid.offset[1];
                refineSelection();
                overview.mv.currentSelectionChanged = true;
            }
            overview.mv.seor->getGrid()->setOffset(overview.mv.grid.offset);
        }
        if (ImGui::ColorEdit4("Color1", overview.mv.grid.color1)) {
            overview.mv.seor->getGrid()->setColor1(overview.mv.grid.color1);
        }
        if (ImGui::ColorEdit4("Color2", overview.mv.grid.color2)) {
            overview.mv.seor->getGrid()->setColor2(overview.mv.grid.color2);
        }
        
        
        ImGui::Separator();
        
        ImGui::Text("Selection:");

        ImGui::Text("Selection mode:");
        if (ImGui::RadioButton("Free", overview.gp.sel.mode == SelectionMode_Free)) {
            overview.gp.sel.mode = SelectionMode_Free;
            refineSelection();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Tileset", overview.gp.sel.mode == SelectionMode_Tileset)) {
            overview.gp.sel.mode = SelectionMode_Tileset;
            refineSelection();
        }
            
        if (overview.gp.sel.mode == SelectionMode_Free) {
            ImGui::PushID("Free options");
            ImGui::Text("Free options:");
            if (ImGui::RadioButton("None", overview.gp.sel.free.mode == SelectionFreeMode_None)) {
                overview.gp.sel.free.mode = SelectionFreeMode_None;
                refineSelection();
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Crop", overview.gp.sel.free.mode == SelectionFreeMode_Crop)) {
                overview.gp.sel.free.mode = SelectionFreeMode_Crop;
                refineSelection();
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Split", overview.gp.sel.free.mode == SelectionFreeMode_Split)) {
                overview.gp.sel.free.mode = SelectionFreeMode_Split;
                refineSelection();
            }
            ImGui::PopID();
        } else {
            ImGui::PushID("Tileset options");
            ImGui::Text("Tileset options:");
            if (ImGui::Checkbox("Tied to grid", &overview.gp.sel.tileset.tiedToGrid)) {
                overview.gp.sel.tileset.offset[0] = overview.mv.grid.offset[0];
                overview.gp.sel.tileset.offset[1] = overview.mv.grid.offset[1];
                overview.gp.sel.tileset.size[0] = overview.mv.grid.unitSize[0];
                overview.gp.sel.tileset.size[1] = overview.mv.grid.unitSize[1];
                refineSelection();
            }
            if (!overview.gp.sel.tileset.tiedToGrid) {
                if (ImGui::InputFloat2("Tileset offset", overview.gp.sel.tileset.offset)) {
                    overview.gp.sel.tileset.offset[0] = std::fmod(overview.gp.sel.tileset.offset[0], overview.gp.sel.tileset.size[0]);
                    overview.gp.sel.tileset.offset[1] = std::fmod(overview.gp.sel.tileset.offset[1], overview.gp.sel.tileset.size[1]);
                    refineSelection();
                }
                if (ImGui::InputFloat2("Tileset size", overview.gp.sel.tileset.size)) {
                    overview.gp.sel.tileset.size[0] += overview.gp.sel.tileset.size[0] == 0;
                    overview.gp.sel.tileset.size[1] += overview.gp.sel.tileset.size[1] == 0;
                    refineSelection();
                }
            }
            if (ImGui::Checkbox("Crop", &overview.gp.sel.tileset.crop)) {
                refineSelection();
            }
            ImGui::PopID();
        }
    
    }

    ImGui::End();
    
    ImGui::PopStyleVar(2);
}

void SpriteEditorOverview::showMainView() {
    // Tileset view
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    
    ImGui::SetNextWindowPos(overview.mv.pos, 0);
    ImGui::SetNextWindowSize(overview.mv.size, 0);

    ImGui::Begin("tileset view", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
 
    {
        
        overview.mv.seor->resize(overview.mv.size[0], overview.mv.size[1]);
        if (overview.mv.seor->getTexture()) {
            ImGui::Image(overview.mv.seor->getTexture(), overview.mv.size);
        }
        
        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered()) {
            // Mouse events: Draging with right click
            ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
            if ((dragDelta.x || dragDelta.y) && !io.MouseReleased[ImGuiMouseButton_Middle] && !io.KeyCtrl) {
                int dx = dragDelta.x - overview.mv.interaction.dragDelta.x;
                int dy = dragDelta.y - overview.mv.interaction.dragDelta.y;
                if (dx || dy) {
                    overview.mv.interaction.dragDelta = dragDelta;
                    translateView((float) dx, (float) dy, 0.0f);
                }
            } else {
                overview.mv.interaction.dragDelta.x = 0;
                overview.mv.interaction.dragDelta.y = 0;
            }

            // Left click selection
            if (io.MouseDown[ImGuiMouseButton_Left]) {
                glm::vec2 pos = glm::vec2(ImGui::GetMousePos());
                pos = screenToWorld(pos - glm::vec2(overview.mv.pos));
                overview.mv.cur1[0] = pos.x;
                overview.mv.cur1[1] = pos.y;
                refineSelection();
            }

            // Right click selection
            if (io.MouseDown[ImGuiMouseButton_Right]) {
                glm::vec2 pos = glm::vec2(ImGui::GetMousePos());
                pos = screenToWorld(pos - glm::vec2(overview.mv.pos));
                overview.mv.cur2[0] = pos.x;
                overview.mv.cur2[1] = pos.y;
                refineSelection();
            }


            // Mousewheel events: Zooming
            if (io.MouseWheel && io.KeyCtrl) {
                ImVec2 pos = ImGui::GetMousePos();
                float dx = overview.mv.size.x / 2.0f - (pos.x - overview.mv.pos.x);
                float dy = overview.mv.size.y / 2.0f - (pos.y - overview.mv.pos.y);
                float factor = 1.1f;
                if (io.MouseWheel < 0) {
                    factor = 1.0 / factor - 1.0;
                } else {
                    factor = factor - 1.0;
                }
                scaleFromPointView(factor, dx, dy);
                overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
            }
        }

        // Keyboard events: Translating
        if (ImGui::IsWindowFocused()) {
            float step = 4.0;
            if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) {
                translateView(0.0, step, 0.0);
                overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
            }
            if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) {
                translateView(0.0, -step, 0.0);
                overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
            }
            if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
                translateView(-step, 0.0, 0.0);
                overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
            }
            if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) {
                translateView(step, 0.0, 0.0);
                overview.mv.seor->setViewProjection(overview.mv.projection * overview.mv.view);
            }
        }

    }

    ImGui::End();
    
    ImGui::PopStyleVar(3);
}

void SpriteEditorOverview::showSpritePanel() {
    // sprite panel
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    
    ImGui::SetNextWindowPos(overview.sp.pos, 0);
    ImGui::SetNextWindowSize(overview.sp.size, 0);

    ImGui::Begin("sprite panel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    {

        if (!overview.sp.selectedSkin) { // Sprite has bo data!
            ImGui::Text("Current skin: None");
        } else {
            ImGui::Text((std::string("Current skin: ") + overview.sp.selectedSkin->getName()).c_str());
            ImGui::PushID("Skin combo");
            if (ImGui::BeginCombo("", overview.sp.selectedSkin->getName().c_str(), ImGuiComboFlags_HeightLargest)) {
                for (auto el : *overview.sprite->getSkins()) {
                    Skin * skin = el.second;
                    bool isSelected = (overview.sp.selectedSkin == skin); 
                    if (ImGui::Selectable(skin->getName().c_str(), isSelected)) {
                        clearSpritePanelData();
                        fillSpritePanelData(skin);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
        }

        ImGui::Separator();

        if (!overview.sp.selectedAnimation) {
            ImGui::Text("No animations available!");
        } else {
            ImGui::Image(overview.sp.selectedAnimationPreview->getTexture(), ImVec2(224, 224));

            ImGui::PushID("Animations combo");
            ImGui::Text("Current animation:");
            if (ImGui::BeginCombo("", overview.sp.selectedAnimation->getName().c_str(), ImGuiComboFlags_HeightLargest)) {
                for (int i = 0; i < overview.sp.animations.size(); i++) {
                    Animation * animation = overview.sp.animations[i];
                    SpritePreview * spritePreview = overview.sp.animationPreviews[i];
                    bool isSelected = (overview.sp.selectedAnimation->getName() == animation->getName()); 
                    
                    ImGui::PushID(animation->getName().c_str());
                    if (ImGui::Selectable("", isSelected, 0, ImVec2(0, 32))) {
                        overview.sp.selectedAnimation = animation;
                        overview.sp.selectedAnimationPreview->getSpriteBox()->mimic(spritePreview->getSpriteBox());
                    }
                    ImGui::SameLine();
                    ImGui::Image(spritePreview->getTexture(), ImVec2(32, 32));
                    ImGui::SameLine();
                    ImGui::Text(animation->getName().c_str());
                    ImGui::PopID();

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();

            ImGui::SameLine();
            
            if (ImGui::Button("Delete", ImVec2(-1.0f, 0.0f))) {
                overview.sp.deleteRequest = true;
            }
        
            ImGui::PushID("Rename animation");
            if (ImGui::InputText("", overview.sp.newAnimationRename, 100)) {}
            ImGui::SameLine();
            if (ImGui::Button("Rename", ImVec2(-1.0f, 0.0f))) {
                if (!overview.sp.selectedSkin->getAnimation(std::string(overview.sp.newAnimationRename))) {
                    renameAnimation();
                }
            }
            ImGui::PopID();
        }

        ImGui::PushID("New animation");
        if (ImGui::InputText("", overview.sp.newAnimationName, 100)) {}
        ImGui::PopID();
        ImGui::SameLine();

        if (ImGui::Button("New", ImVec2(-1.0f, 0.0f))) {
            if (!overview.sp.selectedSkin->getAnimation(std::string(overview.sp.newAnimationName))) {
                newAnimation();
            }
        }

        
        ImGui::Separator();
    }

    ImGui::End();
    
    ImGui::PopStyleVar(2);
}
