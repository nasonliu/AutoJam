// SPDX-License-Identifier: Apache-2.0
#include "Jam.h"
#include <string.h>

namespace groove {
namespace {
constexpr JamChord chords[] = {
  {"Cmaj7", 0, {0, 4, 7, 11}, 4}, {"Dm7", 2, {0, 3, 7, 10}, 4},
  {"Em7", 4, {0, 3, 7, 10}, 4}, {"Fmaj7", 5, {0, 4, 7, 11}, 4},
  {"G7", 7, {0, 4, 7, 10}, 4}, {"Am7", 9, {0, 3, 7, 10}, 4},
  {"Bm7b5", 11, {0, 3, 6, 10}, 4}, {"Db7", 1, {0, 4, 7, 10}, 4},
  {"Am(add9)", 9, {0, 3, 7, 14}, 4}, {"Gsus4", 7, {0, 5, 7, 0}, 3},
  {"C", 0, {0, 4, 7, 0}, 3}, {"G", 7, {0, 4, 7, 0}, 3},
  {"Am", 9, {0, 3, 7, 0}, 3}, {"F", 5, {0, 4, 7, 0}, 3},
  {"D", 2, {0, 4, 7, 0}, 3}, {"D7", 2, {0, 4, 7, 10}, 4},
  {"A7", 9, {0, 4, 7, 10}, 4}, {"Dm", 2, {0, 3, 7, 0}, 3},
  {"E7", 4, {0, 4, 7, 10}, 4}, {"G#dim", 8, {0, 3, 6, 0}, 3},
  {"Esus4", 4, {0, 5, 7, 0}, 3}, {"Fmaj7*", 5, {0, 7, 11, 0}, 3},
  {"C6*", 0, {0, 4, 9, 0}, 3}, {"Asus2", 9, {0, 2, 7, 0}, 3},
  {"B7b5*", 11, {0, 6, 10, 0}, 3}, {"Eaddb9*", 4, {0, 1, 7, 0}, 3},
  {"Baddb2*", 11, {0, 1, 5, 0}, 3},
};
constexpr uint8_t palettes[jamStyleCount][jamDirectionCount] = {
  {CMaj7, DMin7, EMin7, FMaj7, GDom7, AMin7, BHalfDim, AMinAdd9},
  {CMaj7, DMin7, EMin7, FMaj7, GDom7, AMin7, BHalfDim, DbDom7},
  {CMaj7, DMaj, EMin7, FMaj7, GSus4, AMinAdd9, GDom7, AMin7},
  {GMaj, CMaj, DDom7, DMaj, EMin7, AMin, FMaj, ADom7},
  {CMaj7, DMin7, EMin7, FMaj7, GDom7, AMin7, ADom7, DbDom7},
  {CMaj, DMin7, EMin7, FMaj, GMaj, AMin, DDom7, ADom7},
  {AMin, CMaj, FMaj, GMaj, DMin7, EMin7, FMaj7, GDom7},
  {CMaj7, DDom7, EMin7, FMaj7, GDom7, AMin7, ADom7, AMinAdd9},
  {GMaj, CMaj, DMaj, FMaj, AMin, DDom7, EMin7, GDom7},
  {CMaj, DMin7, EMin7, FMaj, GMaj, AMin, CMaj7, DDom7},
  {CMaj, AMin, GMaj, FMaj, DMin7, EMin7, AMin7, CMaj7},
  {ESus4, FOpenMaj7, AMin, COpen6, Asus2, BOpen7, EOpenFlatNine, BOpenFlatTwo},
  {GMaj, CMaj, DMaj, AMin, EMin7, DDom7, GSus4, CMaj7},
  {ADom7, DDom7, EDom7, AMin7, GMaj, CMaj, DMin7, AMin},
};
struct Progression { const char* name; uint8_t length, barsPerChord, chords[12]; };
constexpr Progression progressions[jamStyleCount][jamProgressionCount] = {
  {{"Am7 Fmaj7 Cmaj7 G7", 4, 1, {AMin7, FMaj7, CMaj7, GDom7}},
   {"C G Am F", 4, 1, {CMaj, GMaj, AMin, FMaj}}},
  {{"Dm7 G7 Cmaj7", 4, 1, {DMin7, GDom7, CMaj7, CMaj7}},
   {"Cmaj7 Am7 Dm7 G7", 4, 1, {CMaj7, AMin7, DMin7, GDom7}}},
  {{"Cmaj7 D Cmaj7 Em7", 4, 2, {CMaj7, DMaj, CMaj7, EMin7}},
   {"Cmaj7 D", 2, 2, {CMaj7, DMaj}}},
  {{"G C G D7", 4, 1, {GMaj, CMaj, GMaj, DDom7}},
   {"G D C G", 4, 1, {GMaj, DMaj, CMaj, GMaj}}},
  {{"Cmaj7 A7 Dm7 G7", 4, 1, {CMaj7, ADom7, DMin7, GDom7}},
   {"Cmaj7 Fmaj7 Dm7 G7", 4, 1, {CMaj7, FMaj7, DMin7, GDom7}}},
  {{"C F G C", 4, 1, {CMaj, FMaj, GMaj, CMaj}},
   {"Am C F G", 4, 1, {AMin, CMaj, FMaj, GMaj}}},
  {{"Am F C G", 4, 1, {AMin, FMaj, CMaj, GMaj}},
   {"Am G F G", 4, 1, {AMin, GMaj, FMaj, GMaj}}},
  {{"Am7 D7 Em7 Am7", 4, 1, {AMin7, DDom7, EMin7, AMin7}},
   {"Cmaj7 Am7 Dm7 G7", 4, 1, {CMaj7, AMin7, DMin7, GDom7}}},
  {{"G F C G", 4, 1, {GMaj, FMaj, CMaj, GMaj}},
   {"G C D G", 4, 1, {GMaj, CMaj, DMaj, GMaj}}},
  {{"C G Am F", 4, 1, {CMaj, GMaj, AMin, FMaj}},
   {"Am F C G", 4, 1, {AMin, FMaj, CMaj, GMaj}}},
  {{"C Am G C", 4, 1, {CMaj, AMin, GMaj, CMaj}},
   {"C F Am C", 4, 1, {CMaj, FMaj, AMin, CMaj}}},
  {{"Esus4 Fmaj7* Am E", 4, 1, {ESus4, FOpenMaj7, AMin, ESus4}},
   {"Esus4 C6* Fmaj7* E", 4, 1, {ESus4, COpen6, FOpenMaj7, ESus4}}},
  {{"G C D G", 4, 1, {GMaj, CMaj, DMaj, GMaj}},
   {"G G C D", 4, 1, {GMaj, GMaj, CMaj, DMaj}}},
  {{"12 BAR A7 D7 E7", 12, 1, {ADom7,ADom7,ADom7,ADom7,DDom7,DDom7,ADom7,ADom7,EDom7,DDom7,ADom7,EDom7}},
   {"QUICK IV A7 D7 E7", 12, 1, {ADom7,DDom7,ADom7,ADom7,DDom7,DDom7,ADom7,ADom7,EDom7,DDom7,ADom7,EDom7}}},
};
constexpr const char* styles[] = {"LO-FI", "SWING", "AMBIENT", "BLUEGRASS", "BOSSA NOVA",
                                   "8-BIT", "SYNTHWAVE", "R&B", "ROCK", "POP",
                                   "CHINESE PENT", "MIYAKO", "AMERICANA", "BLUES"};
constexpr uint16_t ionian = 0xAB5, aeolian = 0x5AD, lydian = 0xAD5;
constexpr uint16_t dorian = 0x6AD, mixolydian = 0x6B5, majorPentatonic = 0x295;
struct Scale { uint8_t tonic; uint16_t intervals; };
constexpr Scale scales[jamStyleCount] = {
  {9, aeolian}, {0, ionian}, {0, lydian}, {7, majorPentatonic}, {0, ionian},
  {0, ionian}, {9, aeolian}, {9, dorian}, {7, mixolydian}, {0, ionian},
  {0, majorPentatonic}, {4, 0x1A3}, {7, majorPentatonic}, {9, 0x4E9},
};
constexpr uint16_t minorPentatonic = 0x4A9, harmonicMinor = 0x9AD, miyakoScale = 0x1A3, bluesScale = 0x4E9;
constexpr Scale modalScales[jamScaleCount] = {
  {0, 0}, {0, ionian}, {9, aeolian}, {9, harmonicMinor}, {0, majorPentatonic},
  {9, minorPentatonic}, {9, dorian}, {7, mixolydian}, {0, lydian}, {4, miyakoScale}, {9, bluesScale},
};
constexpr const char* modeNames[jamScaleCount] = {
  "STYLE", "C MAJOR", "A NAT MIN", "A HARM MIN", "C MAJ PENT", "A MIN PENT",
  "A DORIAN", "G MIXOLYD", "C LYDIAN", "E MIYAKO", "A BLUES",
};
constexpr const char* styleScaleNames[jamStyleCount] = {
  "A NAT MIN", "C MAJOR", "C LYDIAN", "G MAJ PENT", "C MAJOR",
  "C MAJOR", "A NAT MIN", "A DORIAN", "G MIXOLYD", "C MAJOR",
  "C MAJ PENT", "E MIYAKO", "G MAJ PENT", "A BLUES",
};
constexpr Progression harmonicDeck[jamProgressionCount] = {
  {"Am Dm E7 Am", 4, 1, {AMin,DMin,EDom7,AMin}},
  {"Am F Dm E7", 4, 1, {AMin,FMaj,DMin,EDom7}},
};
constexpr Progression mixolydianDeck[jamProgressionCount] = {
  {"G F C G", 4, 1, {GMaj,FMaj,CMaj,GMaj}},
  {"G C F G", 4, 1, {GMaj,CMaj,FMaj,GMaj}},
};
constexpr int deckStyle[jamScaleCount] = {Pop, Pop, Synthwave, Pop, ChinesePent, Synthwave, RnB, Rock, Ambient, Miyako, Blues};
const Progression& progressionFor(int style, int mode, int choice) {
  if (mode == ScaleStyle || (mode == ScaleMajor && (style == JazzSwing || style == BossaNova)))
    return progressions[style][choice];
  if (mode == ScaleHarmonicMinor) return harmonicDeck[choice];
  if (mode == ScaleMixolydian) return mixolydianDeck[choice];
  if (mode == ScaleDorian && choice == 1) {
    static constexpr Progression alternative = {"Am7 G D7 Am7", 4, 1, {AMin7, GMaj, DDom7, AMin7}};
    return alternative;
  }
  return progressions[deckStyle[mode]][choice];
}
Scale melodyScale(int style, int chord, int mode) {
  if (mode != ScaleStyle) {
    if (mode == ScaleMajor && chord == ADom7 && style == BossaNova) return {9, 0x5B3};
    return modalScales[mode];
  }
  if (style == Blues) return {9, bluesScale};
  if (style == Miyako) return {4, miyakoScale};
  if (chord == DbDom7) return {1, 0x6D5}; // Db Lydian dominant
  if (chord == ADom7) return {9, 0x5B3};  // A Phrygian dominant, D harmonic minor
  if (chord == DDom7 && style != Americana && style != Bluegrass) return {2, mixolydian};
  return scales[style];
}
// The four quarter-note positions are 0, 4, 8 and 12. A phrase stays legible
// with them alone; the other positions are occasional pickups.
constexpr uint16_t phrasing[jamStyleCount][4] = {
  {0x0111, 0x1110, 0x1141, 0x0101},
  {0x1115, 0x1114, 0x1511, 0x0111},
  {0x0101, 0x0001, 0x0111, 0x0100},
  {0x1151, 0x1111, 0x1115, 0x0101},
  {0x1105, 0x1110, 0x1141, 0x0101},
  {0x1551, 0x1111, 0x1115, 0x0101},
  {0x1101, 0x0101, 0x1111, 0x0001},
  {0x0115, 0x1110, 0x1151, 0x0101},
  {0x1011, 0x1111, 0x1511, 0x0101},
  {0x1111, 0x0111, 0x1111, 0x0101},
  {0x0104, 0x0111, 0x1101, 0x0001},
  {0x0001, 0x0101, 0x1110, 0x0100},
  {0x0511, 0x1111, 0x0115, 0x0101},
  {0x0110, 0x1110, 0x1115, 0x0111},
};
struct MelodyProfile { uint8_t center, radius, motion, repeatCost, gatePercent; };
constexpr MelodyProfile melodyProfiles[jamStyleCount] = {
  {64,7,2,5,71}, {69,9,2,8,62}, {69,7,1,4,94}, {66,9,4,6,49},
  {68,7,2,7,76}, {77,6,4,3,46}, {65,9,3,4,83}, {67,7,2,6,77},
  {64,8,4,2,70}, {67,6,2,2,76}, {72,7,3,5,75}, {70,7,1,5,89},
  {66,8,3,6,67}, {65,7,3,4,66},
};
bool on(uint16_t mask, int step) { return (mask >> step) & 1; }
int magnitude(int value) { return value < 0 ? -value : value; }

int closestPitch(int target, int pitchClass, int low, int high) {
  int best = low + (pitchClass - low % 12 + 12) % 12;
  for (int note = best + 12; note <= high; note += 12)
    if (magnitude(note - target) < magnitude(best - target)) best = note;
  return best;
}
int targetTone(const JamChord& chord, int target, bool guideOnly, const Scale* scale = nullptr) {
  int best = 60, distance = 100;
  for (int i = 0; i < chord.count; ++i) {
    const bool guides = chord.count == 4 && chord.intervals[3] <= 11;
    if (guideOnly && guides && i != 1 && i != 3) continue;
    const int note = closestPitch(target, (chord.root + chord.intervals[i]) % 12, 59, 84);
    // Favor the selected collection while retaining a valid chord when it has no common tone.
    const int cost = magnitude(note - target) + (scale && !on(scale->intervals,(note-scale->tonic+12)%12) ? 48 : 0);
    if (cost < distance) {
      best = note;
      distance = cost;
    }
  }
  return best;
}
}
bool JamChord::contains(int note) const {
  for (int i = 0; i < count; ++i)
    if (note % 12 == (root + intervals[i]) % 12) return true;
  return false;
}
const JamChord& jamChord(int id) { return chords[id]; }
bool jamScaleTone(int style, int chord, int note, int mode) {
  const auto scale = melodyScale(style, chord, mode);
  return on(scale.intervals, (note - scale.tonic + 120) % 12);
}
JamChordMap jamChordMap(const JamStatus& jam) {
  JamChordMap map{};
  int used = 0;
  auto add = [&](int chord) {
    for (int i = 0; i < used; ++i)
      if (map.chord[i] == chord) return;
    if (used < jamDirectionCount) map.chord[used++] = chord;
  };
  const auto& progression = progressionFor(jam.style, jam.scale, jam.progression);
  for (int i = 0; i < progression.length; ++i) add(progression.chords[i]);
  if (jam.scale == ScaleHarmonicMinor) {
    constexpr int alternatives[] = {AMin, DMin, EDom7, FMaj, FMaj7, DMin7, GSharpDim, AMinAdd9};
    for (int chord : alternatives) add(chord);
  } else {
    const int style = jam.scale == ScaleStyle ? jam.style : deckStyle[jam.scale];
    for (int chord : palettes[style]) {
      if (jam.scale != ScaleStyle && jam.scale != ScaleBlues && jam.scale != ScaleMajorPent && jam.scale != ScaleMinorPent) {
        const auto& entry = jamChord(chord);
        bool fits = true;
        for (int i = 0; i < entry.count; ++i)
          if (!jamScaleTone(jam.style, chord, entry.root + entry.intervals[i], jam.scale)) fits = false;
        if (!fits) continue;
      }
      add(chord);
    }
  }
  // Use other voicings of the same mode before falling back to the tonic.
  for (int chord = 0; used < jamDirectionCount && chord < int(sizeof(chords)/sizeof(*chords)); ++chord) {
    const auto& entry = jamChord(chord);
    bool fits = true;
    for (int i = 0; i < entry.count; ++i)
      if (!jamScaleTone(jam.style, chord, entry.root + entry.intervals[i], jam.scale)) fits = false;
    if (fits) add(chord);
  }
  for (int i = used; i < jamDirectionCount; ++i) map.chord[i] = map.chord[0];
  return map;
}
ChordColor jamChordColor(int id) {
  const auto& chord = jamChord(id);
  if (chord.intervals[1] == 3 && chord.intervals[2] == 7 && chord.intervals[3] < 12) return ChordMinor;
  if (chord.intervals[1] == 4) {
    if (chord.count == 4 && chord.intervals[3] == 10) return ChordDominant;
    return ChordMajor;
  }
  return ChordOpen;
}
const char* jamStyleName(int style) { return styles[style]; }
namespace {
int referenceKey(const JamStatus& jam) {
  return jam.scale==ScaleStyle?scales[jam.style].tonic:modalScales[jam.scale].tonic;
}
int keyShift(const JamStatus& jam) {
  return jam.key==autoJamKey?0:(int(jam.key)-referenceKey(jam)+18)%12-6;
}
const char* pitchName(int key,bool flats) {
  constexpr const char* sharpNames[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  constexpr const char* flatNames[]={"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};
  return flats?flatNames[key]:sharpNames[key];
}
bool flatKey(int key) { return key==1 || key==3 || key==5 || key==8 || key==10; }
void append(char* buffer, size_t size, const char* part) {
  if (!size) return;
  size_t used=strlen(buffer);
  while(*part && used+1<size) buffer[used++]=*part++;
  buffer[used]=0;
}
}
int jamKey(const JamStatus& jam) { return jam.key==autoJamKey?referenceKey(jam):jam.key; }
const char* jamKeyName(int key) { return pitchName(key,flatKey(key)); }
int jamPitch(const JamStatus& jam,int note) { return note+keyShift(jam); }
void jamFormatScale(const JamStatus& jam,char* buffer,size_t size) {
  if (!size) return;
  buffer[0]=0;
  const char* label=jam.scale==ScaleStyle?styleScaleNames[jam.style]:modeNames[jam.scale];
  while(*label&&*label!=' ') ++label;
  const char* suffix=label;
  append(buffer,size,jamKeyName(jamKey(jam)));
  if(suffix) append(buffer,size,suffix);
}
void jamFormatChord(const JamStatus& jam,int id,char* buffer,size_t size) {
  if (!size) return;
  buffer[0]=0;
  const auto& chord=jamChord(id);
  if(!keyShift(jam)) { append(buffer,size,chord.name); return; }
  const char* suffix=chord.name+1;
  if(*suffix=='b'||*suffix=='#') ++suffix;
  const int pitch=(chord.root+keyShift(jam)+12)%12;
  constexpr char letters[]="CDEFGAB";
  constexpr uint8_t naturals[]={0,2,4,5,7,9,11};
  auto letter=[](char c) {
    switch(c) {
      case 'D': return 1; case 'E': return 2; case 'F': return 3;
      case 'G': return 4; case 'A': return 5; case 'B': return 6;
      default: return 0;
    }
  };
  const int index=(letter(chord.name[0])-letter(jamKeyName(referenceKey(jam))[0])
                   +letter(jamKeyName(jamKey(jam))[0])+7)%7;
  const int accidental=(pitch-naturals[index]+18)%12-6;
  if (accidental < -2 || accidental > 2) append(buffer,size,pitchName(pitch,flatKey(jamKey(jam))));
  else {
    char spelling[4]={letters[index],0,0,0};
    for(int i=1;i<=magnitude(accidental);++i) spelling[i]=accidental<0?'b':'#';
    append(buffer,size,spelling);
  }
  append(buffer,size,suffix);
}
void jamFormatProgression(const JamStatus& jam,char* buffer,size_t size) {
  if (!size) return;
  buffer[0]=0;
  const auto& progression=progressionFor(jam.style,jam.scale,jam.progression);
  if(progression.length==12) append(buffer,size,jam.progression?"QUICK IV":"12 BAR");
  int previous=-1;
  for(int i=0;i<progression.length;++i) {
    const int chord=progression.chords[i];
    if(progression.length==12) {
      bool displayed=false;
      for(int p=0;p<i;++p) if(progression.chords[p]==chord) displayed=true;
      if(displayed) continue;
    } else if(chord==previous) continue;
    char label[12];
    jamFormatChord(jam,chord,label,sizeof(label));
    if(strlen(buffer)+strlen(label)+1>=size) break;
    if(buffer[0]) append(buffer,size," ");
    append(buffer,size,label);
    previous=chord;
  }
}
int jamStringNote(int id, int string) {
  const JamChord& chord = jamChord(id);
  const int root = 48 + chord.root;
  if (string < chord.count) return root + chord.intervals[string];
  return root + 12 + chord.intervals[string - chord.count];
}

int Jam::random(int range) {
  random_ ^= random_ << 13;
  random_ ^= random_ >> 17;
  random_ ^= random_ << 5;
  return random_ % range;
}
void Jam::start() {
  status_.active = true;
  memset(drumInput_,0,sizeof drumInput_);
  memset(drumTheme_,0,sizeof drumTheme_);
  memset(drumReply_,0,sizeof drumReply_);
  nextDrums_=drumPhrase_=0;
  tappedRhythm_=0;
  status_.waitingChord = !status_.automatic && !manualChordSet_;
  status_.loopInput = false;
  loopCount_ = loopOffset_ = replyCarry_ = 0;
  loopAnswerBar_=0;
  nextLoopReady_ = false;
  status_.loopCells = status_.replyCells = status_.loopSustain = status_.replySustain = 0;
  firstBar_ = true;
  resetProgression_ = true;
  newMotif_ = true;
  phrasesSinceTheme_ = 0;
  if (!status_.userMotif && !status_.capturing) status_.motifNotes = 0;
}
void Jam::stop(uint64_t frame) {
  status_.active = false;
  loopCount_ = loopOffset_ = replyCarry_ = 0;
  loopAnswerBar_=0;
  nextLoopReady_ = false;
  status_.loopCells = status_.replyCells = status_.loopSustain = status_.replySustain = 0;
  status_.loopInput = false;
  if (status_.capturing) toggleCapture(frame);
  if (status_.pendingChord >= 0) selectChord(status_.pendingChord);
  if (status_.pendingStyle >= 0) setStyle(status_.pendingStyle);
  if (status_.pendingScale >= 0) setScale(status_.pendingScale);
  if (status_.pendingKey >= 0) setKey(status_.pendingKey);
  if (status_.pendingInstrument >= 0) setInstrument(status_.pendingInstrument);
  if (status_.pendingProgression >= 0) {
    setProgression(status_.pendingProgression);
    status_.pendingProgression = -1;
  }
}
void Jam::selectChord(int chord) {
  if (chord < 0 || chord >= int(sizeof(chords) / sizeof(*chords))) return;
  if (status_.active) status_.pendingChord = chord;
  else {
    status_.chord = chord;
    status_.pendingChord = -1;
    if (!status_.automatic) {
      manualChordSet_ = true;
      status_.waitingChord = false;
    }
  }
}
void Jam::setMode(bool automatic) {
  if (automatic == status_.automatic) return;
  if (automatic) {
    resetProgression_ = true;
    if (status_.userMotif) newMotif_=true;
  }
  else manualChordSet_ = false;
  loopCount_ = loopOffset_ = replyCarry_ = 0;
  loopAnswerBar_=0;
  nextLoopReady_ = false;
  status_.loopCells = status_.replyCells = status_.loopSustain = status_.replySustain = 0;
  status_.loopInput = false;
  status_.automatic = automatic;
  status_.waitingChord = !automatic;
}
void Jam::setProgression(int progression) {
  if (progression < 0 || progression >= jamProgressionCount) return;
  if (status_.active && status_.automatic) status_.pendingProgression = progression;
  else {
    status_.progression = progression;
    status_.progressionStyle = status_.style;
    resetProgression_ = true;
  }
}
void Jam::setStyle(int style) {
  if (style < 0 || style >= jamStyleCount) return;
  if (status_.active) status_.pendingStyle = style == status_.style ? -1 : style;
  else {
    if (status_.style != style) {
      status_.progressionStyle = style;
      status_.progression = 0;
      resetProgression_ = true;
    }
    status_.style = style;
    status_.pendingStyle = -1;
  }
}
void Jam::setScale(int scale) {
  if (scale < 0 || scale >= jamScaleCount) return;
  if (status_.active) status_.pendingScale = scale == status_.scale ? -1 : scale;
  else {
    if (status_.scale != scale) status_.progression = 0;
    status_.scale = scale;
    status_.pendingScale = -1;
    resetProgression_ = true;
  }
}
void Jam::setKey(int key) {
  if (key < 0 || key > autoJamKey) return;
  if (status_.active) status_.pendingKey = key==status_.key?-1:key;
  else {
    status_.key = key;
    status_.pendingKey = -1;
  }
}
void Jam::setInstrument(int instrument) {
  if (status_.active) status_.pendingInstrument = instrument;
  else {
    status_.instrument = instrument;
    status_.pendingInstrument = -1;
  }
}
void Jam::toggleCapture(uint64_t frame) {
  if (status_.capturing && frame) {
    for (int i = 0; i < recorded_; ++i) releaseCapture(captureTags_[i], frame);
  }
  status_.capturing = !status_.capturing;
  if (status_.capturing) {
    recorded_ = 0;
    replyCarry_ = 0;
  } else if (recorded_) {
    for (int i = 0; i < recorded_; ++i) motif_[i] = recording_[i];
    status_.motifNotes = recorded_;
    motifFromLoop_ = false;
    if (status_.automatic) tappedRhythm_=0;
    status_.userMotif = true;
    newMotif_ = true;
  }
}
void Jam::capture(int note, int velocity, uint64_t frame, uint32_t sixteenthFrames, int tag) {
  if (!status_.capturing || recorded_ == maxMotifNotes || sixteenthFrames == 0) return;
  if (recorded_ == 0) { firstCaptureFrame_ = frame; captureCellFrames_ = sixteenthFrames; }
  const uint64_t offset = (frame - firstCaptureFrame_ + captureCellFrames_ / 2) / captureCellFrames_;
  if (offset >= 32) return;
  if (recorded_ && recording_[recorded_ - 1].step == offset) return;
  captureTags_[recorded_] = tag;
  recording_[recorded_++] = {uint8_t(offset), uint8_t(note), uint8_t(velocity)};
}

void Jam::releaseCapture(int tag, uint64_t frame) {
  if (!status_.capturing || !tag || !recorded_ || frame < firstCaptureFrame_) return;
  const uint64_t end = (frame - firstCaptureFrame_ + captureCellFrames_ / 2) / captureCellFrames_;
  for (int i = recorded_-1; i >= 0; --i) {
    if (captureTags_[i] != tag) continue;
    auto& event = recording_[i];
    const int limit = 32 - event.step;
    const int cells = end >= 32 ? limit : int(end) - event.step;
    event.gate = 4 * (cells > 0 ? cells : 1);
    captureTags_[i] = 0;
    return;
  }
}

bool Jam::captureLoop(int note, int velocity, int step, int tag) {
  if (!status_.active || status_.automatic || status_.waitingChord || status_.capturing) return false;
  status_.loopInput = true;
  replyCarry_ = 0;
  if (step == 16) {
    if (!nextLoopReady_) {
      nextLoop_ = {{0, uint8_t(note), uint8_t(velocity), 4}, uint8_t(tag)};
      nextLoopReady_ = true;
    }
  } else if (loopCount_ < maxMotifNotes && (!loopCount_ || loop_[loopCount_ - 1].event.step != step + loopOffset_)) {
    loop_[loopCount_++] = {{uint8_t(step + loopOffset_), uint8_t(note), uint8_t(velocity), 4}, uint8_t(tag)};
    status_.loopCells |= 1u << step;
  }
  return true;
}

void Jam::releaseLoop(int tag, int step) {
  if (nextLoopReady_ && nextLoop_.heldTag == tag) {
    nextLoop_.heldTag = 0;   // A pickup that starts and ends at the line still occupies a cell.
    return;
  }
  for (int i = loopCount_ - 1; i >= 0; --i) {
    auto& note = loop_[i];
    if (note.heldTag != tag) continue;
    const int cells = step + loopOffset_ - note.event.step;
    note.event.gate = uint8_t((cells > 0 ? cells : 1) * 4);
    for (int cell = 0; cell < step && cell < 16; ++cell)
      if (cell + loopOffset_ > note.event.step)
        status_.loopSustain |= 1u << cell;
    note.heldTag = 0;
    return;
  }
}

int Jam::melodyTarget(int target, bool guides) const {
  const auto scale = melodyScale(status_.style,status_.chord,status_.scale);
  return targetTone(jamChord(status_.chord), target, guides, &scale);
}
void Jam::generateMotif() {
  const int style = status_.style;
  const int chordId = status_.chord;
  const auto scale = melodyScale(style, chordId, status_.scale);
  const auto& profile=melodyProfiles[style];
  const int tonic=closestPitch(profile.center,scale.tonic,59,84);
  const int first=melodyTarget(tonic+random(5)-2);
  const int direction=random(2)?1:-1;
  const int leap=profile.motion>=3?3+random(3):1+random(2);
  int neighbour=first+direction*leap;
  while(!jamScaleTone(style,chordId,neighbour,status_.scale)) neighbour+=direction;
  const int ending=melodyTarget(first+(random(3)?direction*(profile.motion+2):0));
  motif_[0] = {0, uint8_t(first), 86};
  motif_[1] = {4, uint8_t(neighbour), 76};
  motif_[2] = {8, uint8_t(ending), 85};
  status_.motifNotes = 3;
}

void Jam::developMelody(int phraseBar) {
  for (auto& event : lead_) event = {};
  const bool ambient = status_.style == Ambient;
  const bool swing = status_.style == JazzSwing;
  const bool twoBars = motif_[status_.motifNotes - 1].step >= 16;
  const int motifBar = twoBars ? phraseBar % 2 : 0;
  const int variation = phraseBar < 2 ? 0 : random(5) - 2;
  const int origin = motif_[0].note;
  const int registerTarget = status_.userMotif && phraseBar == 0 ? (origin < 60 ? origin + 12 : origin) : lastLead_;
  const int anchor = melodyTarget(registerTarget + variation, swing && phraseBar % 2);
  for (int i = 0; i < status_.motifNotes; ++i) {
    const MotifNote& event = motif_[i];
    if (event.step / 16 != motifBar) continue;
    if (ambient && phraseBar != 0 && (i % 2 == 1)) continue;
    if (phraseBar == 2 && i > 1 && random(3) == 0) continue;
    const int originalStep = event.step % 16;
    const int shift = phraseBar >= 2 && (status_.style == BossaNova || status_.style == RnB) ? 2 : 0;
    const int step = originalStep + shift < 16 ? originalStep + shift : originalStep;
    const bool resolution = i == status_.motifNotes - 1;
    const int interval = int(event.note) - origin;
    const int target = anchor + (phraseBar == 3 ? interval / 2 : interval);
    const int note = melodyTarget(target, swing && phraseBar != 0 && (step == 0 || step == 8));
    const int gate = event.gate ? event.gate : (ambient ? 24 : (resolution ? 9 : 5));
    lead_[step] = {uint8_t(note), event.velocity, uint8_t(gate)};
  }
  if (swing && status_.scale==ScaleStyle && phraseBar >= 2) decorateSwing(phraseBar, false);
  for (const auto& event : lead_) if (event.velocity) lastLead_ = event.note;
}

// Keep the recorded grid and intervals as the material for every new answer.
void Jam::voiceRecordedBar(int bar) {
  for(auto& hit:lead_) hit={};
  int offset=0, previous=-1, sourcePrevious=-1;
  for(int i=0;i<status_.motifNotes;++i) {
    const auto& event=motif_[i];
    if(event.step/16!=bar) continue;
    const int step=event.step%16;
    if(previous<0) offset=melodyTarget(event.note<59?event.note+12:event.note)-event.note;
    const int target=event.note+offset;
    int best=10000, chosen=melodyTarget(target);
    for(int n=59;n<=84;++n) {
      const bool chordTone=jamChord(status_.chord).contains(n);
      if(!chordTone && (previous<0 || !jamScaleTone(status_.style,status_.chord,n,status_.scale))) continue;
      const bool losesContour=previous>=0 && ((event.note>sourcePrevious && n<=previous) ||
                                              (event.note<sourcePrevious && n>=previous));
      const int cost=3*magnitude(n-target)+(losesContour?7:0)+(!chordTone && step%4==0?2:0);
      if(cost<best) { best=cost; chosen=n; }
    }
    lead_[step]={uint8_t(chosen),event.velocity,event.gate?event.gate:uint8_t(4)};
    previous=chosen; sourcePrevious=event.note;
  }
}

void Jam::rememberLoop() {
  auto& bar = history_[historyWrite_];
  bar.count = status_.motifNotes;
  memcpy(bar.notes, motif_, bar.count * sizeof(MotifNote));
  historyWrite_ = (historyWrite_ + 1) % 4;
  if (historySize_ < 4) ++historySize_;
}

bool Jam::continueLoop() {
  const int count = status_.motifNotes;
  if (historySize_ < 2 || count < 2) return false;
  const PlayedBar* source = nullptr;
  int pivot = 0, best = -1;
  const int start = motif_[0].step;
  // Match the last one to three gestures against earlier player input.
  // Each gesture carries its onset spacing and signed pitch interval.
  for (int age = 2; age <= historySize_; ++age) {
    const auto& bar = history_[(historyWrite_ + 4 - age) % 4];
    for (int i = 1; i + 1 < bar.count; ++i) {
      if (start + bar.notes[i+1].step - bar.notes[i].step >= 16) continue;
      int score = 0;
      for (int k = 0; k < 3 && k < i && k + 1 < count; ++k) {
        const auto& a = bar.notes[i-k];
        const auto& b = motif_[count-1-k];
        const auto& prevA = bar.notes[i-k-1];
        const auto& prevB = motif_[count-2-k];
        const int da = a.note - prevA.note, db = b.note - prevB.note;
        if ((da > 0) != (db > 0) || (da < 0) != (db < 0)) break;
        const int rhythmError = magnitude((a.step-prevA.step) - (b.step-prevB.step));
        if (rhythmError > 2) break;
        score += 16 - rhythmError * 3 - (magnitude(da-db) > 6 ? 6 : magnitude(da-db));
      }
      if (!score) continue;
      score = score * 4 + random(4);
      if (score > best) { best = score; source = &bar; pivot = i; }
    }
  }
  if (!source) return false;
  for (auto& hit : lead_) hit = {};
  const auto& tail = motif_[count-1];
  int pitch = melodyTarget(tail.note < 59 ? tail.note + 12 : tail.note);
  int previousStep = start;
  lead_[start] = {uint8_t(pitch), tail.velocity, tail.gate};
  for (int i = pivot+1; i < source->count; ++i) {
    const auto& event = source->notes[i];
    const int step = start + event.step - source->notes[pivot].step;
    if (step >= 16) break;
    const int delta = event.note - source->notes[i-1].note;
    const int target = pitch + delta;
    const bool landing = i+1 == source->count ||
        start + source->notes[i+1].step - source->notes[pivot].step >= 16;
    int bestCost = 10000, next = pitch;
    for (int note = 59; note <= 84; ++note) {
      if (landing || step % 4 == 0) {
        if (!jamChord(status_.chord).contains(note)) continue;
      } else if (!jamScaleTone(status_.style,status_.chord,note,status_.scale)) continue;
      const bool losesDirection = (delta > 0 && note <= pitch) || (delta < 0 && note >= pitch);
      const int cost = magnitude(note-target) + (losesDirection ? 4 : 0);
      if (cost < bestCost) { bestCost = cost; next = note; }
    }
    auto& last = lead_[previousStep];
    const int available = (step-previousStep)*4;
    if (last.gate > available) last.gate = available;
    lead_[step] = {uint8_t(next), event.velocity, event.gate};
    pitch = next;
    previousStep = step;
  }
  return true;
}

void Jam::developLoop(int phraseBar) {
  for (auto& event : lead_) event = {};
  const auto& chord = jamChord(status_.chord);
  const int count = status_.motifNotes;
  const auto& first = motif_[0];
  const int near = first.note < 59 ? first.note + 12 : first.note;
  const int anchor = melodyTarget(near, false);
  auto play = [&](int step, int pitch, int velocity, int gate) {
    if (step < 0 || step >= 16 || lead_[step].velocity) return;
    const int limit = (32 - step) * 4;
    lead_[step] = {uint8_t(melodyTarget(pitch, false)), uint8_t(velocity),
                   uint8_t(gate < limit ? gate : limit)};
  };
  if (phraseBar == 0) {
    // Keep the part the player can recognize immediately. Answer with a new ending.
    for (int i = 0; i < count; ++i) {
      const auto& event = motif_[i];
      int pitch = anchor + int(event.note) - first.note;
      if (i > 0 && i == count - 1) {
        const int old = melodyTarget(pitch, false);
        pitch = closestPitch(anchor, chord.root, 59, 84);
        if (old == pitch) pitch = melodyTarget(pitch + (pitch >= 80 ? -5 : 5), false);
      }
      play(event.step, pitch, event.velocity, event.gate);
    }
    if (count == 1 && first.step + first.gate / 4 <= 12 && first.step < 12) play(12, anchor + 5, 69, 8);
  } else if (phraseBar == 1) {
    // Move a two-note fragment and leave the other half for the answer.
    const int start = first.step < 4 ? 4 : first.step < 8 ? 8 : 0;
    play(start, anchor, first.velocity, first.gate);
    if (count > 1) {
      const auto& second = motif_[1];
      const int spacing = second.step - first.step;
      play(start + (spacing < 4 ? spacing : 4), anchor + second.note - first.note, second.velocity, second.gate);
    }
    play(start == 8 ? 4 : 12, anchor - 4, 74, 8);
  } else if (phraseBar == 2) {
    // Compress the first few notes into a pickup, and turn the melodic contour around.
    const int end = count < 3 ? count : 3;
    for (int i = 0; i < end; ++i) {
      const auto& event = motif_[i];
      const int spacing = (event.step - first.step + 1) / 2;
      play(spacing, anchor - (int(event.note) - first.note), event.velocity, 4 * ((event.gate + 7) / 8));
    }
    play(8, anchor + 7, 77, 24);
  } else {
    // Enlarge the opening and resolve after a rest.
    play(0, anchor, first.velocity, first.gate * 2 > 16 ? 16 : first.gate * 2);
    if (count > 1) play(4, anchor + motif_[1].note - first.note, motif_[1].velocity, 8);
    play(12, closestPitch(anchor, chord.root, 59, 84), 81, 16);
  }
  // Keep the end and the following beginning on whole grid cells.
  int next = 32;
  for (int step = 15; step >= 0; --step) {
    auto& event = lead_[step];
    if (!event.velocity) continue;
    const int available = (next - step) * 4;
    if (event.gate > available) event.gate = available;
    next = step;
  }
}

void Jam::decorateSwing(int phraseBar, bool enclosure) {
  for (int step = 4; step < 16; step += 4) {
    if (!lead_[step].velocity || lead_[step - 2].velocity) continue;
    const int target = lead_[step].note;
    const int direction = phraseBar == 1 ? -1 : 1;
    lead_[step - 2] = {uint8_t(target + direction), 66, 4};
    if (enclosure && !lead_[step - 3].velocity)
      lead_[step - 3] = {uint8_t(target - direction), 59, 3};
    return;
  }
}

// Select note lengths in sixteenth-note cells after the onsets (including fills)
// are known. Unused cells after a release become rests.
void Jam::phraseDurations(int phraseBar, bool fromPlayer) {
  constexpr int lengths[]={1,2,3,4,6,8,12,16};
  int next=16;
  const int articulation=melodyProfiles[status_.style].gatePercent;
  for(int step=15;step>=0;--step) {
    auto& hit=lead_[step];
    if(!hit.velocity) continue;
    const int room=next-step;
    const bool cadence=next==16;
    int target=1+(room-1)*articulation/100;
    if(fromPlayer) target=(target+(hit.gate+2)/4+1)/2;
    if(cadence) target=phraseBar%2 ? room : (room+1)/2;
    else if(step%4) target=(target+1)/2;
    int weights[8]{}, total=0;
    for(int i=0;i<8 && lengths[i]<=room;++i) {
      const int n=lengths[i];
      if(status_.style==Ambient && room>=4 && n<4) continue;
      const int distance=magnitude(n-target);
      weights[i]=1+48/(1+distance*distance);
      if(cadence && phraseBar==3 && n==room) weights[i]*=4;
      total+=weights[i];
    }
    int draw=random(total);
    for(int i=0;i<8;++i) {
      if(draw<weights[i]) { hit.gate=4*lengths[i]; break; }
      draw-=weights[i];
    }
    // Occasionally carry the closing tone over the next downbeat.
    if(cadence && step>=8 && phraseBar%2 && random(4)==0)
      hit.gate=4*(room+4);
    next=step;
  }
}

void Jam::developAutomaticMelody(int phraseBar) {
  for(auto& event:lead_) event={};
  const int style=status_.style, chordId=status_.chord;
  const auto& chord=jamChord(chordId);
  const auto& profile=melodyProfiles[style];
  const auto scale=melodyScale(style,chordId,status_.scale);
  const bool jazz=style==JazzSwing;
  const uint16_t rhythm=phrasing[style][phraseBar];

  uint8_t steps[8]{};
  int length=0;
  for(int step=0;step<16;++step) if(on(rhythm,step)) steps[length++]=step;
  const int first=motif_[0].note;
  const int anchor=melodyTarget(first+(phraseBar==2?4:0),jazz);
  const int home=closestPitch(anchor,scale.tonic,59,84);
  const int following=nextChord();
  const auto followingScale=melodyScale(style,following,status_.scale);
  const int link=targetTone(jamChord(following),anchor,jazz,&followingScale);
  const int lower=profile.center-profile.radius<57?57:profile.center-profile.radius;
  const int upper=profile.center+profile.radius>84?84:profile.center+profile.radius;

  struct Path { uint8_t note[8]{}; int cost=100000; };
  constexpr int beamSize=4;
  Path beam[beamSize];
  beam[0].cost=0;
  const int salt=random(127);
  for(int pos=0;pos<length;++pos) {
    Path next[beamSize];
    const int step=steps[pos];
    const bool last=pos==length-1;
    const bool accented=step%8==0||pos==0||last;
    const int relative=motif_[pos%status_.motifNotes].note-first;
    int wanted=anchor+(phraseBar==1?-relative:relative);
    if(pos>=status_.motifNotes) wanted+=phraseBar==2?3:-2;
    if(last&&phraseBar==3) wanted=home;
    if((status_.scale==ScaleLydian||(status_.scale==ScaleStyle&&style==Ambient))&&phraseBar==2&&!accented)
      wanted=closestPitch(wanted,6,59,84);

    for(int note=lower;note<=upper;++note) {
      const bool harmonic=chord.contains(note);
      const bool inScale=on(scale.intervals,(note-scale.tonic+12)%12);
      if(accented?!harmonic:!inScale) continue;
      const bool guide=chord.count==4&&chord.intervals[3]<=11&&
        ((note-chord.root+12)%12==chord.intervals[1]||(note-chord.root+12)%12==chord.intervals[3]);
      for(const auto& path:beam) {
        if(path.cost==100000) break;
        const int previous=pos?path.note[pos-1]:lastLead_;
        const int delta=note-previous, jump=magnitude(delta);
        const int prior=pos>1?previous-path.note[pos-2]:0;
        int cost=path.cost+2*magnitude(note-wanted)+magnitude(jump-profile.motion);
        if(!inScale) cost+=7;
        if(jump>7) cost+=(jump-7)*4;
        if(jump==6) cost+=3;
        if(!jump) cost+=profile.repeatCost+(pos>1&&prior==0?9:0);
        if(magnitude(prior)>=5&&delta*prior>0) cost+=6;
        if(last&&phraseBar==3) cost+=2*magnitude(note-home);
        else if(last&&following!=chordId&&magnitude(note-link)>2) cost+=magnitude(note-link)-2;
        if(jazz&&accented&&guide) cost-=4;
        if(last&&pos&&jump<=2) cost-=3;
        cost+=(note*13+pos*11+salt)%4;
        for(int i=0;i<beamSize;++i) if(cost<next[i].cost) {
          for(int j=beamSize-1;j>i;--j) next[j]=next[j-1];
          next[i]=path;
          next[i].cost=cost;
          next[i].note[pos]=note;
          break;
        }
      }
    }
    // Rare narrow palettes can miss a chord tone in the preferred register.
    if(next[0].cost==100000) {
      next[0]=beam[0]; next[0].cost+=24;
      next[0].note[pos]=melodyTarget(wanted,jazz);
    }
    for(int i=0;i<beamSize;++i) beam[i]=next[i];
  }
  for(int pos=0;pos<length;++pos) {
    const int step=steps[pos];
    const int velocity=(step%8==0?93:step%4==0?79:64)+random(7);
    lead_[step]={beam[0].note[pos],uint8_t(velocity),0};
  }
  if(jazz&&status_.scale==ScaleStyle&&(phraseBar==1||phraseBar==3)) decorateSwing(phraseBar,true);
  phraseDurations(phraseBar,false);
  lastLead_=beam[0].note[length-1];
}

int Jam::nextChord() const {
  if (status_.pendingChord >= 0) return status_.pendingChord;
  if (!status_.automatic) return status_.chord;
  const int style = status_.pendingStyle>=0?status_.pendingStyle:status_.style;
  const int scale = status_.pendingScale>=0?status_.pendingScale:status_.scale;
  const int choice = status_.pendingProgression>=0?status_.pendingProgression:(status_.pendingStyle>=0 || status_.pendingScale>=0)?0:status_.progression;
  if (barInPhrase_==3 && (status_.pendingStyle>=0 || status_.pendingScale>=0 || status_.pendingProgression>=0))
    return progressionFor(style,scale,choice).chords[0];
  const auto& progression = progressionFor(status_.style,status_.scale,status_.progression);
  return progression.chords[position_ / progression.barsPerChord];
}

void Jam::tapDrum(int track,int step) {
  if(!status_.active || track<0 || track>=3 || step<0 || step>16) return;
  if(step==16) nextDrums_|=1u<<track;
  else drumInput_[track]|=1u<<step;
}

void Jam::developDrums() {
  const uint16_t tapped=drumInput_[0]|drumInput_[1]|drumInput_[2];
  if(tapped) { tappedRhythm_=tapped; loopAnswerBar_=0; }
  bool fresh=false;
  for(int t=0;t<3;++t) {
    if(drumInput_[t]) { drumTheme_[t]=drumInput_[t]; fresh=true; }
    drumInput_[t]=(nextDrums_>>t)&1;
    drumReply_[t]=drumTheme_[t];
  }
  nextDrums_=0;
  drumPhrase_=fresh?0:(drumPhrase_+1)%4;
  if(!drumPhrase_) return;
  // Answer one played accent at a style-specific distance. Leave space around the rest.
  constexpr uint8_t spacing[jamStyleCount]={3,2,8,4,3,2,2,3,4,4,6,8,4,2};
  for(int t=0;t<3;++t) {
    if(!drumTheme_[t]) continue;
    for(int step=15;step>=0;--step) if(on(drumTheme_[t],step)) {
      const int offset=spacing[status_.style]*(drumPhrase_==2?2:1);
      const int answer=(step+offset)%16;
      drumReply_[t]|=1u<<answer;
      if(drumPhrase_==3 && step%4 && answer!=step) drumReply_[t]&=~(1u<<step);
      break;
    }
  }
}

void Jam::answerDrumRhythm(int carry) {
  if(!tappedRhythm_ || status_.loopInput || status_.waitingChord || status_.capturing) return;
  // Reuse the harmonic contour already written for this bar, on the player's clock.
  JamHit contour[16]{};
  int pitches=0, tieCells=0;
  for(int step=0;step<16;++step) if(lead_[step].velocity) {
    contour[pitches++]=lead_[step];
    const int overflow=step+(lead_[step].gate+3)/4-16;
    tieCells=overflow>0?overflow:0;
  }
  if(!pitches) return;
  uint8_t steps[16];
  int count=0;
  for(int step=0;step<16;++step) if(on(tappedRhythm_,step)) steps[count++]=step;

  // Phrase two develops the final group as a unit, preserving its internal spacing.
  constexpr int8_t shifts[jamStyleCount]={1,2,0,0,-2,0,2,-1,0,0,2,0,0,2};
  if(drumPhrase_ && count>1) {
    int group=count-1;
    while(group>1 && steps[group]-steps[group-1]<=2) --group;
    const int shift=status_.style==BossaNova && drumPhrase_>1 && random(3)!=0 ? 0 : shifts[status_.style];
    if(steps[group]+shift>steps[group-1] && steps[count-1]+shift<16)
      for(int i=group;i<count;++i) steps[i]+=shift;
  }
  for(auto& hit:lead_) hit={};
  int previous=-1;
  for(int i=0;i<count;++i) {
    const int step=steps[i];
    if(step<carry) continue;
    const auto& source=contour[i%pitches];
    int note=source.note;
    if(step%4==0 || i==count-1) note=melodyTarget(note);
    if(note==previous) note=melodyTarget(note+(note>=75?-5:5));
    const int gap=(i+1<count?steps[i+1]:16)-step;
    // A tap gives us an onset. Infer a release, leaving silence before the next group.
    int cells=gap>=4?(gap+1)/2:1;
    if(drumPhrase_) {
      const int articulation=melodyProfiles[status_.style].gatePercent;
      cells=(gap*articulation+50)/100;
      if(cells<1) cells=1;
      if(gap>=3 && cells>=gap) cells=gap-1;
    }
    int gate=cells*4;
    if(i==count-1 && tieCells) gate=(16-step+tieCells)*4;
    lead_[step]={uint8_t(note),source.velocity,uint8_t(gate)};
    previous=lastLead_=note;
  }
}

void Jam::articulateLoopStyle(bool closing) {
  const int style=status_.style;
  if(style!=RnB && style!=ChipTune && style!=Synthwave) return;
  int steps[16], count=0;
  for(int i=0;i<16;++i) if(lead_[i].velocity) steps[count++]=i;
  if(!count) return;
  for(int i=0;i<count;++i) {
    const int step=steps[i], next=i+1<count?steps[i+1]:16, room=next-step;
    auto& hit=lead_[step];
    if(step*4+hit.gate>64) continue; // Keep a sounding tie intact.
    if(style==ChipTune) {
      if(closing && i==count-1) { hit.gate=room*4; continue; }
      hit.gate=room>=2?5:3;
      // Break one spacious note into a separated call and a high answer.
      if(room>=4 && i==0) {
        const int up=hit.note<=72?12:-12;
        lead_[step+2]={uint8_t(hit.note+up),uint8_t(hit.velocity*4/5),4};
      }
    } else if(style==RnB) {
      if(closing && i==count-1) {
        hit.gate=room>=3?(room-1)*4:room*4;
      } else if(i+1==count-1 && room>=4) {
        // An unstressed scale neighbour leads into the last sung syllable.
        const int landing=lead_[next].note;
        int neighbour=landing;
        for(int d=1;d<=3;++d) {
          const int n=landing+(landing<72?d:-d);
          if(jamScaleTone(style,status_.chord,n,status_.scale)) { neighbour=n; break; }
        }
        if(neighbour!=landing) {
          if(hit.gate>(room-2)*4) hit.gate=(room-2)*4;
          lead_[next-1]={uint8_t(neighbour),uint8_t(hit.velocity*2/3),4};
        }
      } else if(i==0 && room>=4 && !closing) {
        // A breath before an answer. Leave the initial player response exact.
        auto late=hit;
        late.gate=late.gate>(room-1)*4?(room-1)*4:late.gate;
        lead_[step+1]=late; hit={};
      }
    } else {
      hit.gate=(i==count-1 || room<3)?room*4:(room-1)*4;
    }
  }
}

void Jam::developPhrasePitches(bool statement) {
  if((status_.automatic && !status_.userMotif) || status_.waitingChord || status_.loopInput || status_.capturing) return;
  int last=-1, count=0;
  for(int step=0;step<16;++step) if(lead_[step].velocity) { last=step; ++count; }
  if(last<0) return;
  if(!loopAnswerBar_) { loopRegister_=0; phraseAge_=0; phraseSpan_=2+random(3); }
  if(loopAnswerBar_!=0 && !statement) {
    const bool closing=++phraseAge_>=phraseSpan_;
    const int phase=closing?3:phraseAge_;
    const int style=status_.style;
    const bool hook=style==Rock || style==Pop;
    const bool folk=style==Americana || style==Bluegrass;
    const bool recall=previousLineSize_>=2 && (hook || style==JazzSwing) && random(4)!=0;
    const int center=melodyProfiles[status_.style].center;
    const int home=motif_[0].note<59?motif_[0].note+12:motif_[0].note;
    if(phraseAge_==1) {
      constexpr int levels[]={-4,0,4,7};
      const int pick=random(4);
      loopRegister_=levels[pick]!=loopRegister_?levels[pick]:levels[(pick+1)%4];
    }
    const int registerCenter=(center+home)/2+loopRegister_;
    bool sameLine=true;
    const int direction=loopEnding_>registerCenter ? -1 : 1;
    int previous=loopEnding_, sourcePrevious=-1, index=0;
    for(int step=0;step<=last;++step) {
      auto& hit=lead_[step];
      if(!hit.velocity) continue;
      // Start near the previous ending, then sequence, invert or answer the motif.
      int motion=direction*(2+random(3));
      if(sourcePrevious>=0) {
        motion=int(hit.note)-sourcePrevious;
        if(phase==2) motion=-motion;
        if(!motion) motion=(phase==1?-direction:direction)*2;
        if(phraseAge_==1 && index==1) motion=direction*(3+random(3));
      }
      if(motion>6) motion=6;
      if(motion< -6) motion=-6;
      sourcePrevious=hit.note;
      int target=previous+motion;
      if(target>registerCenter+9 || target<registerCenter-9) target=previous-motion;
      if(index==0) target=(target*2+registerCenter)/3;
      if(phase==3 && step==last) target=(previous+home+loopRegister_)/2;
      // Quote a short opening, then let the ending respond to the present chord.
      if(recall && index<2 && step!=last)
        target=index==0?previousLine_[0]:previous+previousLine_[1]-previousLine_[0];
      if(folk && count>=3)
        target=previous+(index==0?direction*2:index<=((count-1)/2)&&!closing?3:-2);
      if(style==BossaNova && index && step!=last)
        target=previous+(motion>0?2:motion<0?-2:0);
      if(style==JazzSwing && closing && step==last)
        target=melodyTarget(previous,true);
      if(style==RnB) {
        if(index==0) target=loopEnding_;
        else if(step!=last) target=previous+(motion>=0?2:-2);
        else if(closing) target=melodyTarget(previous);
      }
      if(style==ChipTune) {
        if(index==0) target=melodyTarget(home+7);
        else if(step==last && closing) target=closestPitch(previous,jamChord(status_.chord).root,59,84);
        else target=previous+(index%2?-5:7);
      }
      if(style==Synthwave) {
        const int anchor=melodyTarget(home<66?home+12:home);
        if(index<2) target=anchor+int(motif_[index%status_.motifNotes].note)-motif_[0].note;
        else target=closing && step==last?melodyTarget(anchor-5):anchor+(phase==1?7:5);
      }
      // Shape these answers around the current scale; the final target still
      // goes through the same chord-aware pitch selection below.
      if(style==LoFi) target=index==0?loopEnding_:previous+(closing?-2:motion/2);
      if(style==Ambient) target=index==0?loopEnding_:previous+(index==1?2:-1);
      if(style==ChinesePent) target=index==0?home:previous+(step==last?-3:direction*3);
      if(style==Miyako) target=index==0?loopEnding_:previous+(closing?-1:(index%2?1:4));
      if(style==Blues) target=step==last?melodyTarget(home):index==0?loopEnding_:previous+(index%2?3:-2);
      if(status_.automatic && index==0)
        target=(target+registerCenter+loopRegister_)/2;
      if(status_.automatic && closing && step==last)
        target=melodyTarget((target+home+loopRegister_)/2,style==JazzSwing);
      const bool looseBeat=(hook || style==ChinesePent || style==Miyako || style==Blues) && !closing && step!=last;
      int bestCost=10000, chosen=previous;
      for(int n=59;n<=84;++n) {
        const bool chordTone=jamChord(status_.chord).contains(n);
        if(!jamScaleTone(status_.style,status_.chord,n,status_.scale) && !chordTone) continue;
        if((step==last || (step%4==0 && !looseBeat)) && !chordTone) continue;
        if(sameLine && step==last && previousLineSize_==index+1 && n==previousLine_[index]) continue;
        const int jump=magnitude(n-previous);
        const int cost=3*magnitude(n-target)+(jump>6?(jump-6)*5:0)+(jump==0 && !hook && !recall && style!=Synthwave && style!=RnB?6:0)
                       + (!jamScaleTone(status_.style,status_.chord,n,status_.scale)?16:0);
        if(cost<bestCost) { bestCost=cost; chosen=n; }
      }
      sameLine=sameLine && index<previousLineSize_ && chosen==previousLine_[index];
      hit.note=chosen;
      const int accent=index==0?4:(step==last && closing?-8:0);
      hit.velocity=uint8_t(hit.velocity+accent);
      // Leave room to breathe after a cadence; preserve deliberate bar-line ties.
      if(step==last && closing && hit.gate<=(16-step)*4 && hit.gate>=12)
        hit.gate-=4;
      if(style==Ambient || style==Bluegrass || style==Miyako || style==LoFi) {
        int next=step+1;
        while(next<16 && !lead_[next].velocity) ++next;
        if(hit.gate<=(16-step)*4) {
          const int room=(next-step)*4;
          if(style==Ambient) hit.gate=room;
          if(style==Bluegrass && step!=last && hit.gate>6) hit.gate=6;
          if(style==Miyako && closing && step==last) hit.gate=room>8?room-8:4;
          if(style==LoFi && step==last && room>8) hit.gate=room/2;
        }
      }
      previous=chosen;
      ++index;
    }
    articulateLoopStyle(closing);
    if(closing) {
      phraseAge_=0;
      phraseSpan_=2+random(last<12?2:3);
    }
  }
  previousLineSize_=0;
  for(const auto& hit:lead_) if(hit.velocity) previousLine_[previousLineSize_++]=hit.note;
  loopEnding_=lastLead_=previousLine_[previousLineSize_-1];
  ++loopAnswerBar_;
}

void Jam::beginBar() {
  const int previousChord = status_.chord;
  if (firstBar_) barInPhrase_ = 0;
  else barInPhrase_ = (barInPhrase_ + 1) % 4;
  firstBar_ = false;
  if (barInPhrase_ == 0 && status_.pendingStyle >= 0) {
    status_.style = status_.pendingStyle;
    status_.progressionStyle = status_.style;
    status_.progression = 0;
    status_.pendingStyle = -1;
    resetProgression_ = true;
    if (!status_.userMotif) status_.motifNotes = 0;
  }
  if (status_.pendingScale >= 0 && (!status_.automatic || barInPhrase_ == 0)) {
    status_.scale = status_.pendingScale;
    status_.progression = 0;
    status_.pendingScale = -1;
    resetProgression_ = true;
    if (!status_.userMotif) status_.motifNotes = 0;
  }
  if (status_.pendingKey >= 0) {
    status_.key = status_.pendingKey;
    status_.pendingKey = -1;
  }
  if (status_.pendingInstrument >= 0) {
    status_.instrument = status_.pendingInstrument;
    status_.pendingInstrument = -1;
  }
  if (barInPhrase_ == 0 && status_.pendingProgression >= 0) {
    status_.progression = status_.pendingProgression;
    status_.progressionStyle = status_.style;
    status_.pendingProgression = -1;
    resetProgression_ = true;
  }
  if (status_.automatic) {
    const auto& progression = progressionFor(status_.style,status_.scale,status_.progression);
    if (resetProgression_) position_ = 0;
    status_.chord = progression.chords[position_ / progression.barsPerChord];
    position_ = (position_ + 1) % (progression.length * progression.barsPerChord);
    resetProgression_ = false;
  }
  if (status_.pendingChord >= 0) {
    status_.chord = status_.pendingChord;
    status_.pendingChord = -1;
    if (!status_.automatic) manualChordSet_ = true;
  }
  status_.waitingChord = !status_.automatic && !manualChordSet_;
  bool extendInput = false;
  for (int i=0; i<loopCount_; ++i)
    if (loop_[i].heldTag && loopOffset_==0) extendInput=true;
  if (extendInput) {
    loopOffset_=16;
  } else {
    if (loopCount_) {
      for (int i=0; i<loopCount_; ++i) {
        auto note=loop_[i].event;
        if (loop_[i].heldTag) note.gate=(loopOffset_+16-note.step)*4;
        motif_[i]=note;
      }
      status_.motifNotes=loopCount_;
      rememberLoop();
      motifFromLoop_=true;
      status_.userMotif=true;
      newMotif_=true;
    }
    loopCount_=loopOffset_=0;
  }
  status_.loopInput=extendInput || nextLoopReady_;
  status_.loopSustain=0;
  status_.loopCells=nextLoopReady_ ? 1 : 0;
  if (nextLoopReady_) {
    nextLoop_.event.step=loopOffset_;
    if (loopCount_<maxMotifNotes) loop_[loopCount_++]=nextLoop_;
    nextLoopReady_=false;
  }
  developDrums();
  const bool hasDrumTheme=drumTheme_[0]||drumTheme_[1]||drumTheme_[2];
  melodyBar_ = newMotif_ ? 0 : (melodyBar_ + 1) % 4;
  const bool recordedAuto=status_.automatic && status_.userMotif;
  const auto& tail=motif_[status_.motifNotes?status_.motifNotes-1:0];
  const bool secondBar=tail.step>=16 || tail.step+(tail.gate+3)/4>16;
  if(newMotif_) statementBarsLeft_=secondBar?2:1;
  const bool statement=recordedAuto && statementBarsLeft_>0;
  if (!status_.automatic && (status_.waitingChord || (!status_.userMotif && !hasDrumTheme))) {
    for (auto& event : lead_) event = {};
  } else if (!status_.userMotif) {
    if (!status_.motifNotes || (melodyBar_ == 0 && ++phrasesSinceTheme_ >= 2)) {
      generateMotif();
      phrasesSinceTheme_ = 0;
    }
    developAutomaticMelody(melodyBar_);
  } else if (recordedAuto) {
    voiceRecordedBar(secondBar?melodyBar_%2:0);
    if(statementBarsLeft_ && !status_.capturing) --statementBarsLeft_;
  } else if (!status_.automatic && motifFromLoop_ && motif_[status_.motifNotes-1].step<16) {
    if ((melodyBar_ != 0 && previousChord == status_.chord) || !continueLoop()) {
      developLoop(melodyBar_);
      if (melodyBar_ >= 2) phraseDurations(melodyBar_,true);
    }
  } else {
    developMelody(melodyBar_);
    if (melodyBar_ >= 2) phraseDurations(melodyBar_,true);
  }
  status_.replyCells = 0;
  status_.replySustain = 0;
  int end = status_.loopInput || status_.capturing ? 0 : replyCarry_;
  // Resume the new phrase on release if its first attack fell inside the tie.
  if (end>0 && end<16 && !lead_[end].velocity) {
    for (int i=end-1; i>=0; --i) {
      const int remaining=int(lead_[i].gate)-(end-i)*4;
      if (lead_[i].velocity && remaining>0) {
        lead_[end]=lead_[i];
        lead_[end].gate=uint8_t(remaining);
        break;
      }
    }
  }
  for (int step = 0; step < end && step < 16; ++step) lead_[step]={};
  if(!statement) answerDrumRhythm(end);
  if(newMotif_) loopAnswerBar_=0;
  developPhrasePitches(statement);
  for (int step = 0; step < 16; ++step) {
    if (lead_[step].velocity) {
      status_.replyCells |= 1u << step;
      end = step + (lead_[step].gate + 2) / 4;
    } else if (step < end) status_.replySustain |= 1u << step;
  }
  replyCarry_=end>16 && !status_.loopInput && !status_.capturing ? end-16 : 0;
  voiceChord();
  updateTracks();
  newMotif_ = false;
}

void Jam::updateTracks() {
  for (auto& track : status_.trackCells) track=0;
  const bool playingInput = status_.loopInput;
  status_.loopInput = false; // Keep the planned lead visible if input later yields the bar.
  for (int step=0;step<16;++step) {
    const auto event=beat(step);
    for (int t=0;t<3;++t) if(event.drums[t].velocity) status_.trackCells[t]|=1u<<step;
    if(event.bass.velocity) status_.trackCells[3]|=1u<<step;
    for(const auto& note:event.harmony) if(note.velocity) status_.trackCells[4]|=1u<<step;
    if(event.lead.velocity) status_.trackCells[5]|=1u<<step;
  }
  status_.loopInput = playingInput;
}

void Jam::voiceChord() {
  const auto& chord = jamChord(status_.chord);
  const int voices = chord.count - 1;
  constexpr uint8_t orders[][3] = {{1, 2, 3}, {2, 1, 3}, {1, 3, 2},
                                  {2, 3, 1}, {3, 1, 2}, {3, 2, 1}};
  int bestDistance = 1000;
  uint8_t best[3]{};
  const int combinations = voices == 3 ? 6 : 2;
  for (int permutation = 0; permutation < combinations; ++permutation) {
    uint8_t notes[3]{};
    int distance = 0;
    for (int i = 0; i < voices; ++i) {
      const int target = voicing_[i] ? voicing_[i] : 50 + i * 5;
      const int interval = chord.intervals[orders[permutation][i]];
      notes[i] = closestPitch(target, (chord.root + interval) % 12, 47, 69);
      distance += magnitude(notes[i] - target);
    }
    if (distance < bestDistance) {
      bestDistance = distance;
      for (int i = 0; i < 3; ++i) best[i] = notes[i];
    }
  }
  for (int i = 0; i < 3; ++i) voicing_[i] = best[i];
}

JamBeat Jam::beat(int step) const {
  JamBeat result{};
  const auto& chord = jamChord(status_.chord);
  const int bassRoot = closestPitch(42, chord.root, 33, 50);
  int bassPitch = bassRoot;
  uint16_t bassMask = 0x0101, harmonyMask = 0x0441;
  const bool response = barInPhrase_ % 2 != 0;
  if (status_.style == LoFi) {
    if (on(response ? 0x0501 : 0x0141, step)) result.drums[0] = {36, 90, 1};
    if (on(0x1010, step)) result.drums[1] = {38, 72, 1};
    if (on(response ? 0x5044 : 0x4444, step)) result.drums[2] = {42, 49, 1};
    if (step == 8) bassPitch = closestPitch(bassRoot, (chord.root + chord.intervals[2]) % 12, 33, 52);
    if (barInPhrase_ % 2) harmonyMask = 0x1041;
  } else if (status_.style == JazzSwing) {
    bassMask = 0x1111;
    harmonyMask = barInPhrase_ % 2 ? 0x1041 : 0x0405;
    if (on(0x0101, step)) result.drums[0] = {36, 61, 1};
    if (on(0x1010, step)) result.drums[1] = {38, 65, 1};
    if (on(response ? 0x4551 : 0x5151, step)) result.drums[2] = {42, uint8_t(step % 4 ? 42 : 68), 1};
    bassPitch = closestPitch(bassRoot + (step == 8 ? 7 : 0),
                            (chord.root + chord.intervals[(step / 4) % chord.count]) % 12, 33, 55);
    if (step == 12 && nextChord() != status_.chord) {
      const int root = closestPitch(42, jamChord(nextChord()).root, 33, 50);
      bassPitch = root + (root >= bassRoot ? -1 : 1);
    }
  } else if (status_.style == Bluegrass) {
    // A quiet downbeat, two backbeat chops, then an occasional pickup.
    harmonyMask = response ? 0x5011 : 0x1011;
    if (step == 8) bassPitch = closestPitch(bassRoot, (chord.root + chord.intervals[2]) % 12, 33, 52);
  } else if (status_.style == BossaNova) {
    harmonyMask = barInPhrase_ % 2 ? 0x1141 : 0x1041;
    if (on(0x4141,step)) result.drums[0]={36,uint8_t(step%8==0?81:53),1};
    if (on(response?0x1041:0x0481,step)) result.drums[1]={38,68,1};
    if (step%2==0) result.drums[2]={42,uint8_t(step%4==0?41:54),1};
    if (step == 8) bassPitch = closestPitch(bassRoot, (chord.root + chord.intervals[2]) % 12, 33, 52);
  } else if (status_.style == Ambient || status_.style == Miyako || status_.style == ChinesePent) {
    bassMask = 1;
    harmonyMask = response ? 0x0101 : 1;
    if (status_.style == ChinesePent) harmonyMask = response ? 0x1011 : 0x0101;
    if (status_.style == Miyako && response && step == 12) result.drums[2] = {42, 23, 1};
  } else if (status_.style == Americana || status_.style == Blues) {
    bassMask = status_.style == Blues ? 0x1111 : 0x0101;
    harmonyMask = response ? 0x1411 : 0x1011;
    if (on(response ? 0x0501 : 0x0101, step)) result.drums[0] = {36, 63, 1};
    if (on(0x1010, step)) result.drums[1] = {38, 54, 1};
    if(status_.style==Blues && on(response?0x5551:0x5555,step))
      result.drums[2]={42,uint8_t(step%4==0?58:35),1};
    if (step == 8 || (status_.style == Blues && step == 12))
      bassPitch = closestPitch(bassRoot, (chord.root + chord.intervals[2]) % 12, 33, 52);
  } else {
    // Masks use one bit per sixteenth, from the first beat to the last offbeat.
    struct Groove {
      uint16_t kick, snare, hat, bass, harmony, replyKick, replyHat;
      uint8_t bassGate, harmonyGate;
    };
    constexpr Groove grooves[] = {
      {0x0101, 0x1010, 0x5151, 0x1111, 0x5555, 0x0401, 0x4551, 8, 3},   // Chiptune arpeggio
      {0x1111, 0x1010, 0x4444, 0x5555, 0x0101, 0x1111, 0x5444, 6, 26},  // Synthwave offbeat pulse and pads
      {0x0941, 0x1010, 0x5454, 0x1251, 0x2441, 0x0505, 0x4454, 9, 13},  // R&B syncopation
      {0x0505, 0x1010, 0x5555, 0x5555, 0x1111, 0x0141, 0x5155, 7, 9},  // Rock driving eighths
      {0x0101, 0x1010, 0x5555, 0x1111, 0x1041, 0x0901, 0x5155, 12, 14}, // Pop steady four
    };
    const auto& groove = grooves[status_.style - ChipTune];
    if (on(response ? groove.replyKick : groove.kick, step)) result.drums[0] = {36, uint8_t(status_.style == Rock ? 112 : 79), 1};
    if (on(groove.snare, step)) result.drums[1] = {38, uint8_t(status_.style == Rock ? 106 : 72), 1};
    if (on(response ? groove.replyHat : groove.hat, step)) result.drums[2] = {42, 42, 1};
    if (barInPhrase_ == 3 && (step == 14 || (status_.style == Rock && step == 15)))
      result.drums[1] = {38, uint8_t(status_.style == Rock ? 78 : 52), 1};
    if (on(groove.bass, step)) {
      const bool fifth = step == 8 || (status_.style == RnB && step == 6);
      int pitch = fifth ? closestPitch(bassRoot, (chord.root + chord.intervals[2]) % 12, 33, 52) : bassRoot;
      if(status_.style==Synthwave) pitch=bassRoot+(step%4==2?12:0);
      if(status_.style==ChipTune&&step==12) pitch=bassRoot+12;
      if(status_.style==Rock&&step==14) pitch=closestPitch(bassRoot,(chord.root+chord.intervals[2])%12,33,52);
      result.bass = {uint8_t(pitch), uint8_t(step%4==0?101:83), groove.bassGate};
    }
    if (on(groove.harmony, step)) {
      if (status_.style == ChipTune) {
        const int interval = chord.intervals[(step / 2) % chord.count];
        result.harmony[0] = {uint8_t(60 + chord.root + interval), 58, groove.harmonyGate};
      } else if (status_.style == Rock) {
        result.harmony[0] = {uint8_t(bassRoot + 12), 69, groove.harmonyGate};
        result.harmony[1] = {uint8_t(bassRoot + 12 + chord.intervals[2]), 58, groove.harmonyGate};
      } else {
        for (int i = 0; i < chord.count - 1; ++i)
          result.harmony[i] = {voicing_[i], 52, groove.harmonyGate};
      }
    }
    return finishBeat(step, result);
  }
  if (on(bassMask, step))
    result.bass = {uint8_t(bassPitch), 83, uint8_t(status_.style == Ambient || status_.style == Miyako ? 51 : 12)};
  if (on(harmonyMask, step)) {
    for (int i = 0; i < chord.count - 1; ++i)
      result.harmony[i] = {voicing_[i], uint8_t(status_.style == Bluegrass && step == 0 ? 32 : 55),
                           uint8_t(status_.style == Ambient || status_.style == Miyako ? 55 : status_.style == Bluegrass ? 3 : 10)};
  }
  return finishBeat(step, result);
}

JamBeat Jam::finishBeat(int step, JamBeat result) const {
  if (!status_.capturing && !status_.loopInput) result.lead = lead_[step];
  if (barInPhrase_ == 3 && step == 14 && (status_.style == LoFi || status_.style == JazzSwing))
    result.drums[1] = {38, 48, 1};
  if (!status_.automatic) {
    if (status_.style==Bluegrass) {
      if(step%8==0) result.drums[0]={36,67,1};
      if(step%8==4) result.drums[1]={38,77,1};
    } else if(status_.style==ChinesePent) {
      if(on(0x0101,step)) result.drums[0]={36,68,1};
      if(on(barInPhrase_%2?0x5040:0x1040,step)) result.drums[1]={38,58,1};
    } else if(status_.style==Ambient || status_.style==Miyako) {
      if(step==0) result.drums[0]={36,55,1};
      if(step==(status_.style==Ambient?12:8)) result.drums[2]={42,38,1};
    }
    if (step != 0 || status_.waitingChord) {
      for (auto& note : result.harmony) note = {};
    }
    if (status_.waitingChord) {
      result.bass = {};
      result.lead = {};
    } else if (step % 4 == 0) {
      const int root = closestPitch(42, jamChord(status_.chord).root, 33, 50);
      result.bass = {uint8_t(root), uint8_t(step % 8 == 0 ? 93 : 74), 10};
    } else result.bass = {};
  }
  constexpr uint8_t drumNotes[]={36,38,42};
  for(int t=0;t<3;++t) if(on(drumReply_[t],step)) {
    const uint8_t velocity=on(drumTheme_[t],step)?(t==2?76:98):(t==2?52:68);
    if(result.drums[t].velocity<velocity) result.drums[t]={drumNotes[t],velocity,1};
  }
  if (result.bass.velocity) {
    result.bass.note=jamPitch(status_,result.bass.note+12);
  }
  if (result.lead.velocity) result.lead.note=jamPitch(status_,result.lead.note);
  for (auto& note:result.harmony)
    if (note.velocity) note.note=jamPitch(status_,note.note);
  return result;
}
}
