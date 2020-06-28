#include "file_browser.h"
#include "imgui.h"

using namespace uengine::ui;
namespace fs = std::experimental::filesystem;

FileBrowser::FileBrowser() {
    path = fs::current_path();
    fileSelected = false;
    selectedEntry = nullptr;
    closed = true;
    updateListing();
    extensions = {};
}

void FileBrowser::setExtensions(std::vector<std::string> extensions_) {
    extensions = extensions_;
}

void FileBrowser::display() {
    if (closed)
        return;
    
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_Once);

    ImGui::OpenPopup("Select a file");

    bool p_open = true;
    if (ImGui::BeginPopupModal("Select a file", &p_open, 0)) {
    
        ImGui::BeginChild("path", ImVec2(0, ImGui::GetFontSize() * 2.5), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        int levelId = 0;
        int newLevelId = -1;

        for (auto name: path) {
            if (levelId > 0)
                ImGui::SameLine();
            ImGui::PushID(levelId);
            if (ImGui::SmallButton(name.u8string().c_str())) {
                newLevelId = levelId;
            }
            ImGui::PopID();
            ++levelId;
        }
        ImGui::EndChild();

        if (newLevelId >= 0) {
            fs::path newDir;
            for (auto name: path) {
                newDir /= name;
                if (newLevelId-- == 0)
                    break;
            }
            path = newDir;
            updateListing();
        }

        ImGui::BeginChild("listing", ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetFontSize() * 8), true, 0);
        
        ImGui::PushID(path.string().c_str());
        fs::path newPath;
        bool setNewPath = false;
        for (auto const& entry: listing) {
            
            bool selected = (selectedEntry? selectedEntry->name  == entry.name : false);
            if (entry.isFile)
                ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0.0f, 1.0f, 0.0f, 1.0f));
            else    
                ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Selectable(entry.displayName.c_str(), selected, 0)) {
                selectedEntry = &entry;
            }
            ImGui::PopStyleColor();

            if (ImGui::IsItemClicked(0) && ImGui::IsMouseDoubleClicked(0)) {
                if (entry.isFile) {
                    fileSelected = true;
                    closed = true;
                } else {
                    if (selectedEntry->name == "..")
                        newPath = path.parent_path();
                    else
                        newPath = path / entry.name;
                    setNewPath = true;
                }
            }
        }
        ImGui::PopID();
        if (setNewPath && !fileSelected) {
            path = newPath;
            updateListing();
        }
        ImGui::EndChild();

        if (ImGui::Button("ok")) {
            closed = true;
            if (selectedEntry) {
                if (selectedEntry->isFile) {
                    fileSelected = true;
                }
            }
        }

        ImGui::SameLine();
        
        if (ImGui::Button("close")) {
            closed = true;
        }

        ImGui::EndPopup();
    }
    closed |= !p_open;
}

void FileBrowser::open() {
    closed = false;
}

bool FileBrowser::hasSelected() {
    return fileSelected;
}

fs::path FileBrowser::getSelected() {
    fileSelected = false;
    return path / selectedEntry->name;
}

void FileBrowser::updateListing() {
    listing = { Entry{"..", "[D] ..", false} };

    for (auto const& p: fs::directory_iterator(path)) {
        Entry entry = {};
        entry.name = p.path().filename();
        entry.isFile = fs::is_regular_file(p);
        entry.displayName = (entry.isFile ? "[F] ": "[D] ") + entry.name;
        if (!entry.isFile || std::find(std::begin(extensions), std::end(extensions), p.path().extension()) != std::end(extensions))
            listing.push_back(entry);
    }

    std::sort(listing.begin(), listing.end(),
        [](const Entry &e1, const Entry &e2) {
            return (e1.isFile ^ e2.isFile) ? !e1.isFile : (e1.name < e2.name);
        });
    selectedEntry = nullptr;
}
