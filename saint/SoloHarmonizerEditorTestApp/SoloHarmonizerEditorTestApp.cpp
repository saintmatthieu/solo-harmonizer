#include "TestAppMainWindow.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace saint {

class MainWindowTutorialApplication : public juce::JUCEApplication {
public:
  const juce::String getApplicationName() override {
    return "SoloHarmonizerEditorTestApp";
  }

  const juce::String getApplicationVersion() override { return "0.0.0"; }

  void initialise(const juce::String &) override {
    mainWindow.reset(new TestAppMainWindow("SaintHarmonizer"));
  }

  void shutdown() override { mainWindow.reset(); }

private:
  std::unique_ptr<TestAppMainWindow> mainWindow;
};
} // namespace saint

static juce::JUCEApplicationBase *juce_CreateApplication() {
  return new saint::MainWindowTutorialApplication();
}

int main() {
  juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;
  return juce::JUCEApplicationBase::main();
}
