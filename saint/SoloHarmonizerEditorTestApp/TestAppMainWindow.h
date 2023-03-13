#pragma once

#include "FilePlaybackComponent.h"
#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerVst.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace saint {
class TestAppMainWindow : public juce::DocumentWindow,
                          public juce::TextEditor::Listener {
public:
  TestAppMainWindow(juce::String name);
  void closeButtonPressed() override;
  void textEditorReturnKeyPressed(juce::TextEditor &) override;

private:
  void _initTimeInCrotchetsInputEditor();
  void _initOpenEditorButton();
  void _prepareToPlay(int, int);
  void _process(std::vector<float> &);
  juce::Component _rootComponent;
  juce::TextButton _openEditorButton;
  juce::TextEditor _timeInCrotchetsInputEditor;
  SoloHarmonizerVst _harmonizerVst;
  std::unique_ptr<SoloHarmonizerEditor> _sut;
  FilePlaybackComponent _filePlaybackComponent;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestAppMainWindow)
};
} // namespace saint
