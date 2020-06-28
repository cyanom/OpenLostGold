#include "sprite.h"

namespace fs = std::experimental::filesystem;
using namespace rapidxml;
using namespace uengine::graphics;

/* Frame */
Frame::Frame(): frameData() {
}

FrameData * Frame::getData() {
    return &frameData;
}

void Frame::setData(FrameData frameData_) {
    frameData = frameData_;
}

void Frame::loadXML(xml_node<> * frameNode) {
    frameData.uvPos.x = std::stof(frameNode->first_attribute("x")->value());
    frameData.uvPos.y = std::stof(frameNode->first_attribute("y")->value());
    frameData.uvSize.x = std::stof(frameNode->first_attribute("w")->value());
    frameData.uvSize.y = std::stof(frameNode->first_attribute("h")->value());
    frameData.offset.x = std::stof(frameNode->first_attribute("dx")->value());
    frameData.offset.y = std::stof(frameNode->first_attribute("dy")->value());
    frameData.offset.z = 0;
    frameData.size.x = std::stof(frameNode->first_attribute("sx")->value());
    frameData.size.y = std::stof(frameNode->first_attribute("sy")->value());
    frameData.size.z = 1.0;
    frameData.dt = std::stof(frameNode->first_attribute("dt")->value());
}

xml_node<> * Frame::saveXML(xml_document<> * doc) {
    xml_node<> * frameNode = doc->allocate_node(node_element, "frame");

    frameNode->append_attribute(doc->allocate_attribute("x", doc->allocate_string(std::to_string(frameData.uvPos[0]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("y", doc->allocate_string(std::to_string(frameData.uvPos[1]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("w", doc->allocate_string(std::to_string(frameData.uvSize[0]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("h", doc->allocate_string(std::to_string(frameData.uvSize[1]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("sx", doc->allocate_string(std::to_string(frameData.size[0]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("sy", doc->allocate_string(std::to_string(frameData.size[1]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("dx", doc->allocate_string(std::to_string(frameData.offset[0]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("dy", doc->allocate_string(std::to_string(frameData.offset[1]).c_str())));
    frameNode->append_attribute(doc->allocate_attribute("dt", doc->allocate_string(std::to_string(frameData.dt).c_str())));
    
    return frameNode;
}


/* Animation */

Animation::Animation() {
}

Animation::~Animation() {
    for (auto frame : frames) {
        delete frame;
    }
}

void Animation::setName(std::string name_) {
    name = name_;
}

std::string Animation::getName() {
    return name;
}

Frame * Animation::getFrame(int32_t id) {
    return frames[id];
}

std::vector<Frame *> * Animation::getFrames() {
    return &frames;
}

int Animation::getNbFrames() {
    return frames.size();
}

void Animation::addFrame(Frame * frame) {
    frames.push_back(frame);
}

void Animation::removeFrame(Frame * frame) {
    auto it = std::find(frames.begin(), frames.end(), frame);
    if (it == frames.end())
        return;
    frames.erase(it);
    delete frame;
}

void Animation::loadXML(xml_node<> * animationNode) {
    name = animationNode->first_attribute("name")->value();
    
    for (xml_node<> * frameNode = animationNode->first_node(); frameNode; frameNode = frameNode->next_sibling()) {
        Frame * frame = new Frame();
        frame->loadXML(frameNode);
        addFrame(frame);
    }
}

xml_node<> * Animation::saveXML(xml_document<> * doc) {
    xml_node<> * animationNode = doc->allocate_node(node_element, "animation");
    
    animationNode->append_attribute(doc->allocate_attribute("name", name.c_str()));

    for (auto& frame : frames)
        animationNode->append_node(frame->saveXML(doc));

    return animationNode;
}

/* Skin */

Skin::Skin() {
}

Skin::~Skin() {
    for (auto el : animations) {
        delete el.second;
    }
}

void Skin::setName(std::string name_) {
    name = name_;
}

std::string Skin::getName() {
    return name;
}

Animation * Skin::getAnimation(std::string name) {
    if (name.empty()) {
        if (!animations.size()) {
            return nullptr;
        }
        return animations.begin()->second;
    }
    
    return animations[name];
}

std::map<std::string, Animation *> * Skin::getAnimations() {
    return &animations;
}

void Skin::addAnimation(Animation * animation) {
    animations[animation->getName()] = animation;
}

void Skin::removeAnimation(Animation * animation) {
    animations.erase(animation->getName());
    delete animation;
}

void Skin::renameAnimation(std::string previousName, std::string newName) {
    if (animations[newName]) {
        delete animations[newName];
    }
    animations[newName] = animations[previousName];
    animations[newName]->setName(newName);
    animations.erase(previousName);
}

void Skin::loadXML(xml_node<> * skinNode) {
    name = skinNode->first_attribute("name")->value();
    
    for (xml_node<> * animationNode = skinNode->first_node(); animationNode; animationNode = animationNode->next_sibling()) {
        Animation * animation = new Animation();
        animation->loadXML(animationNode);
        addAnimation(animation);
    }
}

xml_node<> * Skin::saveXML(xml_document<> * doc) {
    xml_node<> * skinNode = doc->allocate_node(node_element, "skin");
    
    skinNode->append_attribute(doc->allocate_attribute("name", name.c_str()));

    for (auto& [key, animation] : animations)
        skinNode->append_node(animation->saveXML(doc));

    return skinNode;
}


/*-------------- Sprite --------------*/

Sprite::Sprite(GraphicsBase * gb_) {
    gb = gb_;
}

Sprite::~Sprite() {
    for (auto el : skins) {
        delete el.second;
    }
    
    if (textureLoaded) {
        gb->deleteTextureImage(&textureImage, &textureImageMemory, &textureImageView, &textureSampler, data);
    }
}

void Sprite::setFilename(std::string filename_) {
    filename = filename_;
}

std::string Sprite::getFilename() {
    return filename;
}

void Sprite::load() {
    skins.clear();
    
    std::ifstream ifs(filename);
    std::string buffer((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    xml_document<> doc;
    doc.parse<0>(&buffer[0]);

    xml_node<> * node = doc.first_node();

    name = node->first_attribute("name")->value();
    textureFilename = (fs::path(filename).parent_path() / node->first_attribute("image")->value()).string();
    
    for (xml_node<> * skinNode = node->first_node(); skinNode; skinNode = skinNode->next_sibling()) {
        Skin * skin = new Skin();
        skin->loadXML(skinNode);
        addSkin(skin);
    }
}

void Sprite::save(std::string filename) {
    std::ofstream file;
    file.open(filename.c_str());
    
    xml_document<> doc;

    xml_node<> * node = doc.allocate_node(node_element, "sprite");
    node->append_attribute(doc.allocate_attribute("name", name.c_str()));
    node->append_attribute(doc.allocate_attribute("image", textureFilename.c_str()));

    for (auto& [key, skin] : skins)
        node->append_node(skin->saveXML(&doc));
    
    doc.append_node(node);
    
    file << doc;
    file.close();
}

void Sprite::setName(std::string name_) {
    name = name_;
}

std::string Sprite::getName() {
    return name;
}

VkImageView Sprite::getImageView() {
    return textureImageView;
}

VkSampler Sprite::getSampler() {
    return textureSampler;
}

int Sprite::getWidth() {
    return w;
}

int Sprite::getHeight() {
    return h;
}

uint8_t * Sprite::getPixel(int x, int y) {
    return data + (x + y * w) * 4;
}

std::string Sprite::toString() {
    std::string res = "Sprite [" + name + "]:\n";

    for (auto el1 : skins) {
        Skin * skin = el1.second;
        res += "\tSkin [" + skin->getName() + "]:\n";

        for (auto el2 : *skin->getAnimations()) {
            Animation * animation = el2.second;
            res += "\t\tAnimation [" + animation->getName() + "]:\n";
            int i = 0;
            for (auto frame : *animation->getFrames()) {
                FrameData * frameData = frame->getData();
                res += "\t\t\tFrame [" + std::to_string(i) + "]:";
                res += " uvPos=" + glm::to_string(frameData->uvPos);
                res += " uvSize=" + glm::to_string(frameData->uvSize);
                res += " offset=" + glm::to_string(frameData->offset);
                res += " size=" + glm::to_string(frameData->size);
                res += " dt=" + std::to_string(frameData->dt) + "\n";
                i++;
            }
        }
    }

    return res;
}

void Sprite::setTextureFilename(std::string textureFilename_) {
    textureFilename = textureFilename_;
}

void Sprite::loadTexture(bool keepData) {
    gb->createTextureImage(textureFilename, &textureImage, &textureImageMemory, &textureImageView, &textureSampler, &data, &w, &h, keepData);
    textureLoaded = true;
}

bool Sprite::isTextureLoaded() {
    return textureLoaded;
}

void Sprite::freeTexture() {
    gb->deleteTextureImage(&textureImage, &textureImageMemory, &textureImageView, &textureSampler, data);
}

void Sprite::addSkin(Skin * skin) {
    skins[skin->getName()] = skin;
}

void Sprite::removeSkin(Skin * skin) {
    skins.erase(skin->getName());
    delete skin;
}

void Sprite::renameSkin(std::string previousName, std::string newName) {
    if (skins[newName]) {
        delete skins[newName];
    }
    skins[newName] = skins[previousName];
    skins[newName]->setName(newName);
    skins.erase(previousName);
}

Skin * Sprite::getSkin(std::string name) {
    if (name.empty()) {
        if (!skins.size())
            return nullptr;
        return skins.begin()->second;
    }
    
    return skins[name];
}

std::map<std::string, Skin *> * Sprite::getSkins() {
    return &skins;
}

/*------------ SPRITEBOX ------------*/

SpriteBox::SpriteBox(Sprite * sprite_) {
    sprite = sprite_;

    skin = nullptr;
    animation = nullptr;
    frame = nullptr;
    frameId = 0;
}

SpriteBox::~SpriteBox() {
}

void SpriteBox::setTextureId(int textureId_) {
    textureId = textureId_;
}

Sprite * SpriteBox::getSprite() {
    return sprite;
}

Skin * SpriteBox::getSkin() {
    return skin;
}

Animation * SpriteBox::getAnimation() {
    return animation;
}

Frame * SpriteBox::getFrame() {
    return frame;
}

int SpriteBox::getFrameId() {
    return frameId;
}

SpriteBoxData * SpriteBox::getData() {
    FrameData * data = frame->getData();
    spriteBoxData.model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), position + data->offset) * glm::scale(glm::mat4(1.0f), size * data->size);
    spriteBoxData.tint = glm::make_vec3(tint);
    spriteBoxData.uvPos = data->uvPos / glm::vec2((float) sprite->getWidth(), (float) sprite->getHeight());
    spriteBoxData.uvSize = data->uvSize / glm::vec2((float) sprite->getWidth(), (float) sprite->getHeight());

    return &spriteBoxData;
}

void SpriteBox::setSkin(Skin * skin_) {
    skin = skin_;
    if (animation) {
        animation = skin->getAnimation(animation->getName());
        frame = animation->getFrame(frameId);
    }
}

void SpriteBox::setAnimation(Animation * animation_) {
    animation = animation_;
    if (animation) {
        frame = animation->getFrame(frameId);
    }
}

void SpriteBox::setFrame(int frameId) {
    if (animation) {
        frame = animation->getFrame(frameId);
    }
}

void SpriteBox::update(float dt) {
    if (!frame || !animation)
        return;

    FrameData * data = frame->getData();
    frameTime += dt;
    while (frameTime >= data->dt) {
        frameTime = std::fmod(frameTime, data->dt);
        frameId = (frameId + 1) % animation->getNbFrames();
        frame = animation->getFrame(frameId);
        data = frame->getData();
    }
}

void SpriteBox::setPosition(float x, float y) {
    position = glm::vec3(x, y, 0.0f);
}

void SpriteBox::move(float dx, float dy) {
    position += glm::vec3(dx, dy, 0.0f);
}

void SpriteBox::resize(float sx, float sy) {
    size = glm::vec3(sx, sy, 0.0f);
}

void SpriteBox::resize(float s) {
    size = glm::vec3(s, s, 0.0f);
}

void SpriteBox::setTint(float r, float g, float b) {
    tint[0] = r;
    tint[1] = g;
    tint[2] = b;
}

void SpriteBox::mimic(SpriteBox * other) {
    if (other->sprite != sprite) {
        std::cout << "Can't mimic different sprite!" << std::endl;
        return;
    }
    textureId = other->textureId;
    skin = other->skin;
    animation = other->animation;
    frame = other->frame;
    frameId = other->frameId;
    frameTime = other->frameTime;
    position = other->position;
    size = other->size;
    xShear = other->xShear;
    yShear = other->yShear;
    angle = other->angle;
    tint[0] = other->tint[0];
    tint[1] = other->tint[1];
    tint[2] = other->tint[2];
}
