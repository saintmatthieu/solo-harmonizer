#include "DummyAudioProcessor.h"
#include "SoloHarmonizerEditor.h"

#include <juce_gui_basics/juce_gui_basics.h>

class MainWindowTutorialApplication : public juce::JUCEApplication {
public:
  class MainWindow : public juce::DocumentWindow {
  public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Colours::lightgrey,
                         DocumentWindow::allButtons),
          sut(processor, [](const std::filesystem::path &) {}) {
      centreWithSize(300, 200);
      setVisible(true);
      constexpr auto resizeToFitWhenContentChangesSize = true;
      setContentNonOwned(&sut, resizeToFitWhenContentChangesSize);
    }

    void closeButtonPressed() override {
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

  private:
    DummyAudioProcessor processor;
    SoloHarmonizerEditor sut;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

  const juce::String getApplicationName() override {
    return "SoloHarmonizerEditorTestApp";
  }

  const juce::String getApplicationVersion() override { return "0.0.0"; }

  void initialise(const juce::String &commandLineParameters) override {
    mainWindow.reset(new MainWindow("HelloWorldDemo"));
  }

  void shutdown() override { mainWindow.reset(); }

private:
  std::unique_ptr<MainWindow> mainWindow;
};

juce::JUCEApplicationBase *juce_CreateApplication() {
  return new MainWindowTutorialApplication();
}

int main() {
  juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;
  return juce::JUCEApplicationBase::main();
}
