#include "Playheads/ProcessCallbackDrivenPlayhead.h"
#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerVst.h"

#include "gmock/gmock.h"
#include <juce_gui_basics/juce_gui_basics.h>

#include <gmock/gmock.h>

namespace saint {

class MockPlayhead : public Playhead {
public:
  using OptFloat = std::optional<float>;
  MOCK_METHOD(OptFloat, getTimeInCrotchets, (), (const));
};

class MainWindowTutorialApplication : public juce::JUCEApplication {
public:
  class MainWindow : public juce::DocumentWindow,
                     public juce::TextEditor::Listener {
  public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Colours::lightgrey,
                         DocumentWindow::allButtons),
          _openEditorButton("Open Editor"),
          _harmonizerVst(
              [](bool, const JuceAudioPlayHeadProvider &,
                 const std::optional<float>,
                 const std::optional<int>) -> std::shared_ptr<Playhead> {
                return std::shared_ptr<Playhead>{new MockPlayhead()};
              }) {
      centreWithSize(400, 600);
      setVisible(true);
      constexpr auto resizeToFitWhenContentChangesSize = true;
      _openEditorButton.setSize(400, 100);
      _openEditorButton.onClick = [this]() {
        if (!_sut) {
          _sut.reset(_harmonizerVst.createSoloHarmonizerEditor());
          _sut->setTopLeftPosition(0, 200);
          _openEditorButton.setButtonText("Close Editor");
          _rootComponent.addAndMakeVisible(_sut.get());
        } else {
          _rootComponent.removeChildComponent(_sut.get());
          _sut.reset();
          _openEditorButton.setButtonText("Open Editor");
        }
      };
      _timeInCrotchetsInputEditor.setTopLeftPosition(0, 100);
      _timeInCrotchetsInputEditor.setSize(400, 100);
      _timeInCrotchetsInputEditor.setTextToShowWhenEmpty(
          "Set time (crotchets)", juce::Colours::whitesmoke.withAlpha(0.5f));
      _timeInCrotchetsInputEditor.setInputFilter(
          new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789."),
          true);
      _timeInCrotchetsInputEditor.addListener(this);
      _rootComponent.setSize(400, 500);
      _rootComponent.addAndMakeVisible(&_openEditorButton);
      _rootComponent.addAndMakeVisible(&_timeInCrotchetsInputEditor);
      setContentNonOwned(&_rootComponent, resizeToFitWhenContentChangesSize);
    }

    void closeButtonPressed() override {
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

    void textEditorReturnKeyPressed(juce::TextEditor &) override {
      if (!_sut) {
        return;
      }
      const auto time =
          std::stof(_timeInCrotchetsInputEditor.getText().toStdString());
      _sut->updateTimeInCrotchets(time);
    }

  private:
    juce::Component _rootComponent;
    juce::TextButton _openEditorButton;
    juce::TextEditor _timeInCrotchetsInputEditor;
    saint::SoloHarmonizerVst _harmonizerVst;
    std::unique_ptr<SoloHarmonizerEditor> _sut;
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
