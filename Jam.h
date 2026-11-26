// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace groove {
enum JamStyle : uint8_t { LoFi, JazzSwing, Ambient, Bluegrass, BossaNova, ChipTune, Synthwave, RnB, Rock, Pop, ChinesePent, Miyako, Americana, Blues, jamStyleCount };
constexpr int jamDirectionCount = 8;
constexpr int jamProgressionCount = 2;
constexpr int autoJamInstrument = 127, autoJamKey = 12;
enum JamScale : uint8_t {
  ScaleStyle, ScaleMajor, ScaleNaturalMinor, ScaleHarmonicMinor, ScaleMajorPent,
  ScaleMinorPent, ScaleDorian, ScaleMixolydian, ScaleLydian, ScaleMiyako, ScaleBlues, jamScaleCount
};
enum ChordColor : uint8_t { ChordMajor, ChordMinor, ChordDominant, ChordOpen };

struct JamChord {
  const char* name;
  uint8_t root;
  uint8_t intervals[4];
  uint8_t count;
  bool contains(int note) const;
};
enum JamChordId : uint8_t {
  CMaj7, DMin7, EMin7, FMaj7, GDom7, AMin7, BHalfDim, DbDom7, AMinAdd9, GSus4, CMaj, GMaj, AMin, FMaj, DMaj, DDom7, ADom7, DMin, EDom7, GSharpDim, ESus4, FOpenMaj7, COpen6, Asus2, BOpen7, EOpenFlatNine, BOpenFlatTwo
};
struct JamStatus {
  bool active = false, automatic = true, capturing = false, userMotif = false;
  bool waitingChord = false, loopInput = false;
  uint16_t loopCells = 0, replyCells = 0, loopSustain = 0, replySustain = 0, trackCells[6]{};
  uint8_t scale = ScaleStyle, key = autoJamKey;
  uint8_t style = 0, instrument = autoJamInstrument, progression = 0, progressionStyle = 0, chord = CMaj7, motifNotes = 0;
  int8_t pendingChord = -1, pendingStyle = -1, pendingInstrument = -1, pendingProgression = -1, pendingScale = -1, pendingKey = -1;
};
struct JamChordMap { uint8_t chord[jamDirectionCount]; };
struct JamHit {
  uint8_t note = 0, velocity = 0, gate = 0; // gate in quarters of a sixteenth note
};
struct JamBeat {
  JamHit drums[3], bass, harmony[3], lead;
};

const JamChord& jamChord(int id);
bool jamScaleTone(int style, int chord, int note, int scale = ScaleStyle);
JamChordMap jamChordMap(const JamStatus& jam);
ChordColor jamChordColor(int chord);
int jamKey(const JamStatus& jam);
const char* jamKeyName(int key);
int jamPitch(const JamStatus& jam, int note);
void jamFormatScale(const JamStatus& jam, char* buffer, size_t size);
void jamFormatChord(const JamStatus& jam, int chord, char* buffer, size_t size);
void jamFormatProgression(const JamStatus& jam, char* buffer, size_t size);
const char* jamStyleName(int style);
int jamStringNote(int chord, int string);

class Jam {
 public:
  void seed(uint32_t value) { if (value) random_ = value; }
  void start();
  void stop(uint64_t frame = 0);
  void selectChord(int chord);
  void setMode(bool automatic);
  void setProgression(int progression);
  void setStyle(int style);
  void setScale(int scale);
  void setKey(int key);
  void setInstrument(int instrument);
  void toggleCapture(uint64_t frame = 0);
  void capture(int note, int velocity, uint64_t frame, uint32_t sixteenthFrames, int tag = 0);
  void releaseCapture(int tag, uint64_t frame);
  bool captureLoop(int note, int velocity, int step, int tag);
  void releaseLoop(int tag, int step);
  void tapDrum(int track,int step);
  void beginBar();
  JamBeat beat(int step) const;
  const JamStatus& status() const { return status_; }

 private:
  struct MotifNote { uint8_t step, note, velocity, gate = 0; };
  static constexpr int maxMotifNotes = 8;
  struct PlayedBar { MotifNote notes[maxMotifNotes]{}; uint8_t count = 0; };
  PlayedBar history_[4]{};
  uint8_t historyWrite_ = 0, historySize_ = 0;
  bool continueLoop();
  void rememberLoop();
  uint32_t random_ = 0x36a23e49;
  JamStatus status_{};
  struct LoopNote { MotifNote event; uint8_t heldTag = 0; };
  MotifNote motif_[maxMotifNotes]{}, recording_[maxMotifNotes]{};
  LoopNote loop_[maxMotifNotes]{}, nextLoop_{};
  uint16_t drumInput_[3]{}, drumTheme_[3]{}, drumReply_[3]{};
  uint8_t nextDrums_=0, drumPhrase_=0;
  void developDrums();
  uint16_t tappedRhythm_=0;
  void answerDrumRhythm(int carry);
  void developPhrasePitches(bool statement);
  void voiceRecordedBar(int bar);
  uint8_t statementBarsLeft_=0;
  void articulateLoopStyle(bool closing);
  uint16_t loopAnswerBar_=0;
  uint8_t phraseAge_=0, phraseSpan_=3;
  int loopEnding_=67, loopRegister_=0;
  uint8_t previousLine_[16]{}, previousLineSize_=0;
  uint8_t loopOffset_ = 0, replyCarry_ = 0;
  bool nextLoopReady_ = false, motifFromLoop_ = false;
  uint8_t recorded_ = 0, loopCount_ = 0, position_ = 0, barInPhrase_ = 0, melodyBar_ = 0, phrasesSinceTheme_ = 0;
  bool manualChordSet_ = false;
  bool resetProgression_ = true, firstBar_ = true, newMotif_ = false;
  uint64_t firstCaptureFrame_ = 0;
  uint32_t captureCellFrames_ = 0;
  uint8_t captureTags_[maxMotifNotes]{};
  int lastLead_ = 67;
  uint8_t voicing_[3]{};
  JamHit lead_[16]{};
  int random(int range);
  void generateMotif();
  int melodyTarget(int target, bool guides = false) const;
  void updateTracks();
  JamBeat finishBeat(int step, JamBeat result) const;
  void developMelody(int phraseBar);
  void developLoop(int phraseBar);
  void phraseDurations(int phraseBar, bool fromPlayer);
  void developAutomaticMelody(int phraseBar);
  void decorateSwing(int phraseBar, bool enclosure);
  int nextChord() const;
  void voiceChord();
};
}
