#pragma once

#include "Playhead.h"
#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerVst.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <gmock/gmock.h>

namespace saint {

class MockPlayhead : public Playhead {
public:
  using OptFloat = std::optional<float>;
  MOCK_METHOD(OptFloat, getTimeInCrotchets, (), (const));
};

class TestAppMainWindow : public juce::DocumentWindow,
                          public juce::TextEditor::Listener {
public:
  TestAppMainWindow(juce::String name);
  void closeButtonPressed() override;
  void textEditorReturnKeyPressed(juce::TextEditor &) override;

private:
  void _initTimeInCrotchetsInputEditor();
  juce::Component _rootComponent;
  juce::TextButton _openEditorButton;
  juce::TextEditor _timeInCrotchetsInputEditor;
  saint::SoloHarmonizerVst _harmonizerVst;
  std::unique_ptr<SoloHarmonizerEditor> _sut;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestAppMainWindow)
};
} // namespace saint
