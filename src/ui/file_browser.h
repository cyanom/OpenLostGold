#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include <experimental/filesystem>
#include <string>
#include <iostream>
#include <vector>
#include <functional>

#include "imgui.h"

namespace uengine::ui {

    class FileBrowser {
    public:
        FileBrowser();
        void setExtensions(std::vector<std::string> extensions_);
        void display();
        void open();
        bool hasSelected();
        std::experimental::filesystem::path getSelected();
        void resetSelected();
        
    private:
        bool closed;
        bool fileSelected;
        std::experimental::filesystem::path path;
        std::vector<std::string> extensions;

        struct Entry {
            std::string name;
            std::string displayName;
            bool isFile;
        };

        const Entry * selectedEntry;
        std::vector<Entry> listing;

        void updateListing();
    };

}

#endif
