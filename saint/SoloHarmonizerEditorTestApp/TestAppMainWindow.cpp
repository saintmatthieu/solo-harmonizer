#include "TestAppMainWindow.h"

namespace saint {

TestAppMainWindow::TestAppMainWindow(juce::String name)
    : DocumentWindow(name, juce::Colours::lightgrey,
                     DocumentWindow::allButtons),
      _openEditorButton("Open Editor"),
      _harmonizerVst([](bool, const JuceAudioPlayHeadProvider &,
                        const std::optional<float>,
                        const std::optional<int>) -> std::shared_ptr<Playhead> {
        return std::shared_ptr<Playhead>{new MockPlayhead()};
      }) {
  centreWithSize(400, 700);
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
  _initTimeInCrotchetsInputEditor(), _rootComponent.setSize(400, 500);
  _rootComponent.addAndMakeVisible(&_openEditorButton);
  _rootComponent.addAndMakeVisible(&_timeInCrotchetsInputEditor);
  setContentNonOwned(&_rootComponent, resizeToFitWhenContentChangesSize);
}

void TestAppMainWindow::closeButtonPressed() {
  juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void TestAppMainWindow::textEditorReturnKeyPressed(juce::TextEditor &) {
  if (!_sut || _timeInCrotchetsInputEditor.getText().isEmpty()) {
    return;
  }
  const auto time =
      std::stof(_timeInCrotchetsInputEditor.getText().toStdString());
  _sut->updateTimeInCrotchets(time);
}

void TestAppMainWindow::_initTimeInCrotchetsInputEditor() {
  _timeInCrotchetsInputEditor.setTopLeftPosition(0, 100);
  _timeInCrotchetsInputEditor.setSize(400, 100);
  _timeInCrotchetsInputEditor.setTextToShowWhenEmpty(
      "Set time (crotchets)", juce::Colours::whitesmoke.withAlpha(0.5f));
  _timeInCrotchetsInputEditor.setInputFilter(
      new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789."),
      true);
  _timeInCrotchetsInputEditor.addListener(this);
}
} // namespace saint
