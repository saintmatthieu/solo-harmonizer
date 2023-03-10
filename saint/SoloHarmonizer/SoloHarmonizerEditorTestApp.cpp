#include "Playheads/ProcessCallbackDrivenPlayhead.h"
#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerVst.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace saint {
class MainWindowTutorialApplication : public juce::JUCEApplication {
public:
  class MainWindow : public juce::DocumentWindow {
  public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Colours::lightgrey,
                         DocumentWindow::allButtons),
          _openEditorButton("Open Editor"),
          _harmonizerVst(
              [](bool mustSetPpqPosition, const JuceAudioPlayHeadProvider &,
                 float crotchetsPerSample) -> std::unique_ptr<Playhead> {
                assert(mustSetPpqPosition);
                return std::make_unique<ProcessCallbackDrivenPlayhead>(
                    crotchetsPerSample);
              }) {
      centreWithSize(400, 300);
      setVisible(true);
      constexpr auto resizeToFitWhenContentChangesSize = true;
      _openEditorButton.setSize(400, 100);
      _openEditorButton.onClick = [this]() {
        if (!_sut) {
          _sut.reset(_harmonizerVst.createEditor());
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
    saint::SoloHarmonizerVst _harmonizerVst;
    std::unique_ptr<juce::AudioProcessorEditor> _sut;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

  const juce::String getApplicationName() override {
    return "SoloHarmonizerEditorTestApp";
  }

  const juce::String getApplicationVersion() override { return "0.0.0"; }

  void initialise(const juce::String &) override {
    mainWindow.reset(new MainWindow("SaintHarmonizer"));
  }

  void shutdown() override { mainWindow.reset(); }

private:
  std::unique_ptr<MainWindow> mainWindow;
};
} // namespace saint

static juce::JUCEApplicationBase *juce_CreateApplication() {
  return new saint::MainWindowTutorialApplication();
}

int main() {
  juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;
  return juce::JUCEApplicationBase::main();
}
