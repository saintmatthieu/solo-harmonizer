#include "Factory/IntervalGetterFactory.h"
#include "SoloHarmonizer.h"
#include "SoloHarmonizerEditor.h"

#include <juce_gui_basics/juce_gui_basics.h>

class MainWindowTutorialApplication : public juce::JUCEApplication {
public:
  class MainWindow : public juce::DocumentWindow {
  public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Colours::lightgrey,
                         DocumentWindow::allButtons),
          _openEditorButton("Open Editor"),
          _intervallerFactory(std::make_shared<saint::IntervalGetterFactory>()),
          _harmonizer(std::nullopt, _intervallerFactory, _intervallerFactory) {
      centreWithSize(400, 300);
      setVisible(true);
      constexpr auto resizeToFitWhenContentChangesSize = true;
      _openEditorButton.setSize(400, 100);
      _openEditorButton.onClick = [this]() {
        if (!_sut) {
          _sut = std::make_unique<saint::SoloHarmonizerEditor>(
              _harmonizer, *_intervallerFactory);
          _sut->setTopLeftPosition(0, 100);
          _openEditorButton.setButtonText("Close Editor");
          _rootComponent.addAndMakeVisible(_sut.get());
        } else {
          _rootComponent.removeChildComponent(_sut.get());
          _sut.reset();
          _openEditorButton.setButtonText("Open Editor");
        }
      };
      _rootComponent.setSize(400, 400);
      _rootComponent.addAndMakeVisible(&_openEditorButton);
      setContentNonOwned(&_rootComponent, resizeToFitWhenContentChangesSize);
    }

    void closeButtonPressed() override {
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

  private:
    juce::Component _rootComponent;
    juce::TextButton _openEditorButton;
    const std::shared_ptr<saint::IntervalGetterFactory> _intervallerFactory;
    saint::SoloHarmonizer _harmonizer;
    std::unique_ptr<saint::SoloHarmonizerEditor> _sut;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

  const juce::String getApplicationName() override {
    return "SoloHarmonizerEditorTestApp";
  }

  const juce::String getApplicationVersion() override { return "0.0.0"; }

  void initialise(const juce::String &commandLineParameters) override {
    mainWindow.reset(new MainWindow("SaintHarmonizer"));
  }

  void shutdown() override { mainWindow.reset(); }

private:
  std::unique_ptr<MainWindow> mainWindow;
};

static juce::JUCEApplicationBase *juce_CreateApplication() {
  return new MainWindowTutorialApplication();
}

int main() {
  juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;
  return juce::JUCEApplicationBase::main();
}
