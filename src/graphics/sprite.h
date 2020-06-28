#ifndef SPRITE_H
#define SPRITE_H

#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <string>
#include <fstream>
#include <streambuf>
#include <experimental/filesystem>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "rapidxml_ext.h"

#include "graphics_base.h"


namespace uengine::graphics {

    struct FrameData {
        glm::vec2 uvPos;  // In-texture coordinates
        glm::vec2 uvSize; // In-texture dimensions
        glm::vec3 offset; // Real World offset
        glm::vec3 size;   // Real world dimensions
        float dt;         // Frame duration
    };

    class Frame {
        public:
            Frame();
            FrameData * getData();
            void setData(FrameData frameData);
            void loadXML(rapidxml::xml_node<> * frameNode);
            rapidxml::xml_node<> * saveXML(rapidxml::xml_document<> * doc);
                
        private:
            FrameData frameData;
    };

    class Animation {
        public:
            Animation();
            ~Animation();
            void setName(std::string name);
            std::string getName();
            Frame * getFrame(int32_t nb);
            std::vector<Frame *> * getFrames();
            int getNbFrames();
            void addFrame(Frame * frame);
            void removeFrame(Frame * frame);
            void loadXML(rapidxml::xml_node<> * animationNode);
            rapidxml::xml_node<> * saveXML(rapidxml::xml_document<> * doc);

        private:
            std::string name;
            std::vector<Frame *> frames;
    };

    class Skin {
        public:
            Skin();
            ~Skin();
            void setName(std::string name);
            std::string getName();
            Animation * getAnimation(std::string name);
            std::map<std::string, Animation *> * getAnimations();
            void addAnimation(Animation * animation);
            void removeAnimation(Animation * animation);
            void renameAnimation(std::string previousname, std::string newName);
            void loadXML(rapidxml::xml_node<> * skinNode);
            rapidxml::xml_node<> * saveXML(rapidxml::xml_document<> * doc);

        private:
            std::string name;
            std::map<std::string, Animation *> animations;
    };

    class Sprite {
        public:
            Sprite(GraphicsBase * gb);
            ~Sprite();

            void setFilename(std::string filename);
            std::string getFilename();
            void load();
            void save(std::string filename);

            void setName(std::string name);
            std::string getName();
            
            VkImageView getImageView();
            VkSampler getSampler();

            int getWidth();
            int getHeight();
            uint8_t * getPixel(int x, int y);
            std::string toString();

            void setTextureFilename(std::string textureFilename);
            void loadTexture(bool keepData = false);
            bool isTextureLoaded();
            void freeTexture();
            
            void addSkin(Skin * skin);
            void removeSkin(Skin * skin);
            void renameSkin(std::string previousName, std::string newName);
            Skin * getSkin(std::string name);
            std::map<std::string, Skin *> * getSkins();
        private:
            std::string filename;
            GraphicsBase * gb;

            bool textureLoaded = false;
            std::string textureFilename;
            VkDeviceMemory textureImageMemory;
            VkImage textureImage;
            VkImageView textureImageView;
            VkSampler textureSampler;
            uint8_t * data;
            int w;
            int h;

            std::string name;
            std::map<std::string, Skin *> skins;
    };

    struct SpriteBoxData {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec3 tint;
        alignas(8) glm::vec2 uvPos;
        alignas(8) glm::vec2 uvSize;
        alignas(4) int textureId;
    };

    class SpriteBox {
        public:
            SpriteBox(Sprite * sprite);
            ~SpriteBox();

            void setTextureId(int textureId);
            Sprite * getSprite();
            Skin * getSkin();
            Animation * getAnimation();
            Frame * getFrame();
            int getFrameId();
            SpriteBoxData * getData();

            void setSkin(Skin * skin);
            void setAnimation(Animation * animation);
            void setFrame(int frameId);

            void update(float dt);

            void setPosition(float x, float y);
            void move(float dx, float dy);
            void resize(float sx, float sy);
            void resize(float s);
            void setTint(float r, float g, float b);
            void mimic(SpriteBox * other);

        private:
            int textureId;
            Sprite * sprite = nullptr;
            Skin * skin = nullptr;
            Animation * animation = nullptr;
            Frame * frame = nullptr;
            int frameId = 0;

            float frameTime = 0.0;

            glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
            glm::vec3 size = glm::vec3(1.0, 1.0, 1.0);
            float xShear = 0.0;
            float yShear = 0.0;
            float angle = 0.0;
            float tint[3] = {1.0, 1.0, 1.0};

            SpriteBoxData spriteBoxData;
    };

}

#endif
