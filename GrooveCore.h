// Modified for AutoJam, 2026. See NOTICE.md.
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdint.h>
#include "Jam.h"
#include "src/DaisySP/Synthesis/fm2.h"
#include "src/DaisySP/Synthesis/oscillator.h"
#include "src/DaisySP/Control/adsr.h"
namespace groove {
constexpr int trackCount=5, stepCount=16, bankCount=4, presetCount=9, voiceCount=8;
constexpr int drumPadTag=20;
constexpr int chipPulsePreset = 8;
enum Track : uint8_t { Kick, Snare, Hat, Bass, Lead };
enum Parameter : uint8_t { Ratio, Index, Attack, Decay, Sustain, Release, Tone, Level, parameterCount };
enum class Action : uint8_t {
  Transport, Record, Tempo, Swing, Master, Echo, Led, Bank, Preset, Param,
  Mute, TrackLevel, ToggleStep, StepNote, StepVelocity, Note, Off, Bend, Panic, ClearBank,
  JamTransport, JamChord, JamMode, JamProgression, JamStyle, JamScaleMode, JamKey, JamLiveNote, JamLeadLevel, JamPreset, JamCapture, JamLoopOn, JamLoopOff
};
struct Command { Action action; uint8_t track=0, item=0, tag=0; int16_t value=0; uint16_t gateMs=0; uint32_t revision=0, inputMicros=0; };
struct Patch { uint8_t preset; uint16_t values[parameterCount]; };
struct Step { uint8_t note, velocity, enabled; };
struct Song {
  uint16_t bpm;
  uint8_t swing, master, echo, led, bank;
  uint8_t mute[trackCount], levels[trackCount];
  Patch patches[2]; // Bass and Lead have independent FM patches.
  Step steps[bankCount][trackCount][stepCount];
};
struct Meter {
  JamStatus jam;
  bool playing=false, recording=false;
  uint8_t step=0, pendingBank=255, voices=0, jamLeadLevel=130;
  uint32_t triggers[trackCount]={};
  float peak=0;
  int8_t wave[32]={};
};
extern const char* const trackNames[trackCount];
extern const char* const presetNames[presetCount];
extern const char* const parameterNames[parameterCount];
extern const char* const chordNames[8];
extern const char* const optionNames[12];
void defaults(Song& song);
void sanitize(Song& song);
Patch preset(int index);
int parameterLimit(int index);
float midiFrequency(int note);
// The audio task owns mutable synthesis state; input sends queued commands.
class Engine {
 public:
  void init(uint32_t sampleRate, const Song& song);
  void command(const Command& command, uint32_t inputDelayFrames = 0);
  void render(int16_t* stereo, int frames);
  const Song& song() const { return song_; }
  const Meter& meter() const { return meter_; }
  uint64_t frames() const { return clock_; }
  void restore(const Song& song);
  void seedJam(uint32_t seed) { jam_.seed(seed); }
 private:
  struct Voice {
    daisysp::Fm2 fm;
    daisysp::Adsr envelope;
    bool active=false;
    uint8_t track=Lead, note=60, tag=0;
    uint32_t gate=0;
    float gain=0, depth=0, tone=0, filtered=0, amplitude=0, tail=0;
    float pulsePhase=0, pulseStep=0;
  };
  struct Drum {
    float env=0, decay=0, phase=0, filter=0, gain=0;
    float frequency=0, sweep=0, body=0, noise=0, cutoff=0;
  };
  void trigger(int track, int note, int velocity, uint32_t gateFrames, int tag, const Patch* patch = nullptr);
  int loopGridStep(uint32_t inputDelayFrames) const;
  void advanceStep();
  void playJamStep(uint32_t frames,double swing);
  Jam jam_;
  Patch harmonyPatch_, jamBassPatch_;
  float drums();
  float random();
  Song song_{};
  Meter meter_{};
  Voice voices_[voiceCount];
  Drum drum_[3];
  uint64_t kickPadPriorityUntil_=0;
  float kickAttack_=0, kickAttackDecay_=0;
  uint32_t sampleRate_=32000, lastStepFrames_=0, noise_=0x853c49e7;
  uint64_t clock_=0, nextStep_=0;
  double stepRemainder_=0;
  float bend_=0, volume_=0, dcX_=0, dcY_=0, harmonyGain_=1;
  // Fixed delay storage avoids allocation in the realtime audio path.
  float delay_[8192]={};
  uint32_t delayPos_=0;
};
}
