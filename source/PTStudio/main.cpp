#include <omni/core/OmniInit.h>
#include <omni/Function.h>

#include <carb/ClientUtils.h>
#include <carb/StartupUtils.h>
#include <carb/input/IInput.h>
#include <carb/windowing/IWindowing.h>
#include <carb/settings/ISettings.h>
#include <carb/simplegui/ISimpleGui.h>
#include <carb/filesystem/IFileSystem.h>
#include <carb/extras/WindowsPath.h>
#include <chrono>

#include <usd_includes.h>

CARB_GLOBALS("PTStudio");

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

struct ScopedAction
{
    ScopedAction(omni::function<void()> action) : _action(action) {}
    ~ScopedAction() { if (_action) _action(); }
private:
    omni::function<void()> _action;
};

int main(int argc, char** argv)
{

#if CARB_PLATFORM_WINDOWS
    carb::extras::adjustWindowsDllSearchPaths();
#endif

    OMNI_CORE_INIT(argc, argv);
    if (!carb::getFramework())
    {
        CARB_LOG_FATAL("Failed to acquire the framework");
        return EXIT_FAILURE;
    }

    // auto isetting = checkedAcquireInterface<carb::settings::ISettings>();
    auto iinput = carb::getFramework()->acquireInterface<carb::input::IInput>();
    auto iwin = carb::getFramework()->acquireInterface<carb::windowing::IWindowing>();
    auto igui = carb::getFramework()->acquireInterface<carb::simplegui::ISimpleGui>();
    auto ifs = carb::getFramework()->acquireInterface<carb::filesystem::IFileSystem>();

    auto window = iwin->createWindow({ 1024, 768, "Test Window", false, carb::windowing::kWindowHintNone });
    iwin->showWindow(window);

    auto actions = {
        ScopedAction {[iwin, window]() { iwin->destroyWindow(window); }},
        ScopedAction {[igui]() { igui->destroyContext(igui->getCurrentContext()); }},
    };

    iwin->setWindowTitle(window, "Test Application");
    igui->createContext( carb::simplegui::ContextDesc {
        { 800.0f, 600.0f },
        window,
        iwin->getKeyboard(window),
        iwin->getMouse(window),
        nullptr,
    });

    // Load INI settings from file if it exists
    auto exePath = std::string { ifs->getExecutableDirectoryPath() };
    auto settingsFilePath = exePath + "/gui_setting.ini";
    if (ifs->exists(settingsFilePath.c_str()))
    {
        auto file = ifs->openFileToRead(settingsFilePath.c_str());
        auto fileSize = ifs->getFileSize(file);
        auto content = std::vector<char>(fileSize);
        if (ifs->readFileChunk(file, content.data(), fileSize) < fileSize)
        {
            CARB_LOG_WARN("Failed to read all data from settings file. Some settings may not be loaded correctly.");
        }
        ifs->closeFile(file);
        igui->loadIniSettingsFromMemory(content.data(), fileSize);
    }

    auto lastTime = std::chrono::steady_clock::now();
    auto first = true;
    while (!iwin->shouldWindowClose(window))
    {
        igui->newFrame();
        igui->showDemoWindow(nullptr);

        auto curTime = std::chrono::steady_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::duration<float>>(curTime - lastTime).count();
        lastTime = curTime;

        if (first) {
            dt = 1.0f / 60.0f;
            first = false;
        }

        igui->render(dt);

        iwin->updateInputDevices();
        iwin->pollEvents();
        iinput->distributeBufferedEvents();
    }

    auto file_size = size_t{0};
    auto settings = igui->saveIniSettingsToMemory(&file_size);

    auto file = ifs->openFileToWrite(settingsFilePath.c_str());
    if (ifs->writeFileChunk(file, settings, file_size) < file_size)
    {
        CARB_LOG_WARN("failed to write all data to gui setting file");
    }
    ifs->closeFile(file);

    return EXIT_SUCCESS;
}