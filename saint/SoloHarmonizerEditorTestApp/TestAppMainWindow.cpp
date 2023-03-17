#include "TestAppMainWindow.h"
#include "Playhead.h"
#include "Playheads/ProcessCallbackDrivenPlayhead.h"
#include "SoloHarmonizerEditor.h"
#include "Utils.h"

#include <gmock/gmock.h>

#include <cassert>

namespace saint {

using namespace std::placeholders;

namespace {
constexpr auto openEditorButtonYOffset = 0;
constexpr auto openEditorButtonHeight = 100;
constexpr auto timeInCrotchetsInputEditorYOffset =
    openEditorButtonYOffset + openEditorButtonHeight;
constexpr auto timeInCrotchetsInputEditorHeight = 100;
constexpr auto filePlaybackComponentYOffset =
    timeInCrotchetsInputEditorYOffset + timeInCrotchetsInputEditorHeight;
constexpr auto soloHarmonizerEditorYOffset = openEditorButtonHeight +
                                             timeInCrotchetsInputEditorHeight +
                                             FilePlaybackComponent::height;
constexpr auto totalHeight =
    openEditorButtonHeight + timeInCrotchetsInputEditorHeight +
    FilePlaybackComponent::height + SoloHarmonizerEditor::height;
} // namespace

TestAppMainWindow::TestAppMainWindow(juce::String name)
    : DocumentWindow(name, juce::Colours::lightgrey,
                     DocumentWindow::allButtons),
      _openEditorButton("Open Editor"),
      _harmonizerVst([](bool mustSetPpqPosition,
                        const JuceAudioPlayHeadProvider &,
                        const std::optional<float> &crotchetsPerSecond,
                        const std::optional<int> &samplesPerSecond)
                         -> std::shared_ptr<Playhead> {
        assert(mustSetPpqPosition);
        if (!samplesPerSecond.has_value() || !crotchetsPerSecond.has_value()) {
          return nullptr;
        }
        return std::shared_ptr<Playhead>{new ProcessCallbackDrivenPlayhead(
            *samplesPerSecond, utils::getCrotchetsPerSample(
                                   *crotchetsPerSecond, *samplesPerSecond))};
      }),
      _filePlaybackComponent(
          std::bind(&TestAppMainWindow::_prepareToPlay, this, _1, _2),
          std::bind(&TestAppMainWindow::_process, this, _1)) {
  centreWithSize(SoloHarmonizerEditor::width, totalHeight);
  setVisible(true);
  constexpr auto resizeToFitWhenContentChangesSize = true;
  _initOpenEditorButton();
  _initTimeInCrotchetsInputEditor();
  _filePlaybackComponent.setTopLeftPosition(0, filePlaybackComponentYOffset);
  _rootComponent.setSize(SoloHarmonizerEditor::width, totalHeight);
  _rootComponent.addAndMakeVisible(&_openEditorButton);
  _rootComponent.addAndMakeVisible(&_timeInCrotchetsInputEditor);
  _rootComponent.addAndMakeVisible(&_filePlaybackComponent);
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
  _timeInCrotchetsInputEditor.setTopLeftPosition(
      0, timeInCrotchetsInputEditorYOffset);
  _timeInCrotchetsInputEditor.setSize(SoloHarmonizerEditor::width,
                                      timeInCrotchetsInputEditorHeight);
  _timeInCrotchetsInputEditor.setTextToShowWhenEmpty(
      "Set time (crotchets)", juce::Colours::whitesmoke.withAlpha(0.5f));
  _timeInCrotchetsInputEditor.setInputFilter(
      new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789."),
      true);
  _timeInCrotchetsInputEditor.addListener(this);
}

void TestAppMainWindow::_initOpenEditorButton() {
  _openEditorButton.setSize(SoloHarmonizerEditor::width,
                            openEditorButtonHeight);
  _openEditorButton.onClick = [this]() {
    if (!_sut) {
      _sut.reset(_harmonizerVst.createSoloHarmonizerEditor());
      _sut->setTopLeftPosition(0, soloHarmonizerEditorYOffset);
      _openEditorButton.setButtonText("Close Editor");
      _rootComponent.addAndMakeVisible(_sut.get());
    } else {
      _rootComponent.removeChildComponent(_sut.get());
      _sut.reset();
      _openEditorButton.setButtonText("Open Editor");
    }
  };
}

void TestAppMainWindow::_prepareToPlay(int sampleRate, int blockSize) {
  _harmonizerVst.prepareToPlay(static_cast<double>(sampleRate), blockSize);
}

void TestAppMainWindow::_process(std::vector<float> &audio) {
  std::vector<float *> channels(1);
  channels[0] = audio.data();
  juce::AudioBuffer<float> buffer{channels.data(), 1,
                                  static_cast<int>(audio.size())};
  juce::MidiBuffer midiBuf;
  _harmonizerVst.processBlock(buffer, midiBuf);
}
} // namespace saint
