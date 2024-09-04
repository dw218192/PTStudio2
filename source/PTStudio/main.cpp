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

template <typename InterfaceType>
auto checkedAcquireInterface()
{
    auto interface = carb::getFramework()->acquireInterface<InterfaceType>();
    if (!interface)
    {
        CARB_LOG_FATAL("Failed to acquire the interface %s", typeid(InterfaceType).name());
        exit(EXIT_FAILURE);
    }
    return interface;
}

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
    auto iinput = checkedAcquireInterface<carb::input::IInput>();
    auto iwin = checkedAcquireInterface<carb::windowing::IWindowing>();
    auto igui = checkedAcquireInterface<carb::simplegui::ISimpleGui>();
    auto ifs = checkedAcquireInterface<carb::filesystem::IFileSystem>();

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
    auto exe_path = std::string { ifs->getExecutableDirectoryPath() };
    auto settingsFilePath = exe_path + "/gui_setting.ini";
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

    auto last_time = std::chrono::steady_clock::now();
    auto first = true;
    while (!iwin->shouldWindowClose(window))
    {
        igui->newFrame();
        igui->showDemoWindow(nullptr);

        auto current_time = std::chrono::steady_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::duration<float>>(current_time - last_time).count();
        last_time = current_time;

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