// SPDX-License-Identifier: Apache-2.0
#include "../GrooveCore.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <cstring>

using namespace groove;
constexpr int rate = 32000;
constexpr int bar = 2 * rate;

void render(Engine& engine, int frames, FILE* pcm = nullptr) {
  int16_t block[512];
  while (frames > 0) {
    const int count = frames > 256 ? 256 : frames;
    engine.render(block, count);
    if (pcm) assert(fwrite(block, sizeof(int16_t) * 2, count, pcm) == size_t(count));
    frames -= count;
  }
}

void checkHarmony() {
  const auto& add9 = jamChord(AMinAdd9);
  assert(add9.contains(69) && add9.contains(72) && add9.contains(76) && add9.contains(83));
  assert(!add9.contains(79));
  const auto& sus4 = jamChord(GSus4);
  assert(sus4.contains(67) && sus4.contains(72) && sus4.contains(74));
  assert(!sus4.contains(71) && !sus4.contains(70));
  const auto& dominant = jamChord(GDom7);
  const auto& replacement = jamChord(DbDom7);
  assert(dominant.contains(71) && dominant.contains(77));
  assert(replacement.contains(71) && replacement.contains(77));
  assert(replacement.contains(61) && !replacement.contains(67));

  for (int style = 0; style < jamStyleCount; ++style) {
    Jam jam;
    jam.setStyle(style);
    jam.setMode(false);
    jam.start();
    for (int direction = 0; direction < jamDirectionCount; ++direction) {
      const int selected = jamChordMap(jam.status()).chord[direction];
      jam.selectChord(selected);
      jam.beginBar();
      const auto& chord = jamChord(selected);
      const auto downbeat = jam.beat(0);
      assert(downbeat.bass.velocity && downbeat.bass.note % 12 == chord.root);
      int voices = 0;
      for (const auto hit : downbeat.harmony) {
        if (!hit.velocity) continue;
        assert(chord.contains(hit.note));
        ++voices;
      }
      assert(voices == (style == ChipTune ? 1 : style == Rock ? 2 : chord.count - 1));
      for (int step = 0; step < 16; ++step) {
        const auto beat = jam.beat(step);
        if (beat.bass.velocity) assert(chord.contains(beat.bass.note));
        if (beat.lead.velocity && step % 4 == 0) assert(chord.contains(beat.lead.note));

      }
    }
  }
}

void checkCapturedMotif() {
  Jam jam;
  jam.setMode(false);
  jam.selectChord(CMaj7);
  jam.start();
  jam.toggleCapture();
  jam.capture(64, 91, 0, 4000);
  jam.capture(67, 78, 12000, 4000);
  jam.capture(71, 97, 80000, 4000);
  jam.toggleCapture();
  jam.beginBar();
  assert(jam.beat(0).lead.note == 64);
  assert(jam.beat(3).lead.note == 67);
  assert(!jam.beat(2).lead.velocity);
  jam.beginBar();
  assert(jam.beat(4).lead.velocity);
}

void checkPlayedLengths() {
  Jam jam;
  jam.setMode(false);
  jam.selectChord(CMaj);
  jam.start();
  jam.toggleCapture();
  constexpr int cell=4000;
  const int starts[]={0,2,5,10}, lengths[]={1,2,4,6};
  for(int i=0;i<4;++i) {
    jam.capture(64+i,90,starts[i]*cell+100,cell,10+i);
    jam.releaseCapture(10+i,(starts[i]+lengths[i])*cell-200);
  }
  jam.toggleCapture();
  jam.beginBar();
  for(int i=0;i<4;++i) assert(jam.beat(starts[i]).lead.gate==4*lengths[i]);

  // Measure the actual audio late in a short note and a held note on the same string.
  // Repeat the measurement during the first generated answer.
  Engine engine;
  Song song{};
  defaults(song);
  song.bpm=120; song.echo=0;
  for(int t=0;t<Lead;++t) song.mute[t]=true;
  engine.init(rate,song);
  engine.command({Action::JamMode,0,0,0,0});
  engine.command({Action::JamChord,0,0,0,CMaj});
  engine.command({Action::JamPreset,0,0,0,5}); // Plucked preset with zero native sustain.
  engine.command({Action::JamTransport});
  render(engine,1);
  auto energy=[&]() {
    int16_t audio[512];
    engine.render(audio,256);
    uint64_t sum=0;
    for(int sample:audio) sum+=sample*sample;
    return sum;
  };
  engine.command({Action::JamLoopOn,Lead,108,10,1});
  render(engine,cell-1);
  engine.command({Action::JamLoopOff,Lead,0,10});
  render(engine,5*cell);
  const auto shortLive=energy();
  render(engine,2*cell-256);
  engine.command({Action::JamLoopOn,Lead,108,10,1});
  render(engine,6*cell);
  assert(energy()>shortLive*4);
  render(engine,cell-256);
  engine.command({Action::JamLoopOff,Lead,0,10});
  render(engine,7*cell);
  const auto shortReply=energy();
  render(engine,8*cell-256);
  assert(energy()>shortReply*4);
  assert(engine.meter().jam.replySustain==0x7e00);
}

void checkAcrossBar() {
  Jam jam;
  jam.setMode(false);
  jam.selectChord(CMaj);
  jam.start(); jam.beginBar();
  jam.captureLoop(64,100,12,10);
  jam.beginBar();
  assert(jam.status().loopInput);
  jam.releaseLoop(10,4);
  jam.beginBar();
  assert(jam.beat(12).lead.gate==32);
  jam.selectChord(GMaj);
  jam.beginBar();
  assert(jam.status().chord==GMaj);
  assert((jam.status().replySustain&15)==15);
  for(int i=0;i<4;++i) assert(!jam.beat(i).lead.velocity);

  Song song{};
  defaults(song); song.bpm=120;
  Engine engine;
  engine.init(rate,song);
  engine.command({Action::JamMode,0,0,0,0});
  engine.command({Action::JamChord,0,0,0,CMaj});
  engine.command({Action::JamTransport});
  constexpr int cell=bar/16;
  render(engine,12*cell+1);
  engine.command({Action::JamLoopOn,Lead,110,10,1}); // E above C
  render(engine,8*cell);
  engine.command({Action::JamLoopOff,Lead,0,10});
  render(engine,24*cell); // first reply starts on beat four
  engine.command({Action::JamChord,0,0,0,GMaj});
  render(engine,4*cell); // G chord now sounds under the tied E
  Engine cut=engine;
  cut.command({Action::JamCapture}); // releases the generated lead
  render(engine,3*cell); render(cut,3*cell);
  int16_t tied[512], released[512];
  engine.render(tied,256); cut.render(released,256);
  int difference=0;
  for(int i=0;i<512;++i) difference+=std::abs(int(tied[i])-released[i]);
  assert(difference>512*100);

}

void checkGeneratedLengths() {
  unsigned allLengths=0;
  int ties=0;
  for(int style=0;style<jamStyleCount;++style) {
    Jam jam;
    jam.setStyle(style);
    jam.start();
    int rests=0, carry=0;
    for(int barIndex=0;barIndex<64;++barIndex) {
      jam.beginBar();
      for(int i=0;i<carry;++i) {
        assert(!jam.beat(i).lead.velocity);
        assert(jam.status().replySustain&(1u<<i));
      }
      carry=0;
      int next=16;
      for(int step=15;step>=0;--step) {
        const auto hit=jam.beat(step).lead;
        if(!hit.velocity) continue;
        assert(hit.gate>=4 && hit.gate%4==0);
        const int cells=hit.gate/4;
        assert(step+cells<=next || (next==16 && step+cells==20));
        if(step+cells>16) { carry=step+cells-16; ++ties; }
        else rests+=next-step-cells;
        allLengths|=1u<<cells;
        next=step;
      }
    }
    assert(rests>0);
  }
  assert(ties>0);
  for(int cells:{1,2,3,4,6,8,12,16}) assert(allLengths&(1u<<cells));
}

void checkStyles() {
  constexpr int expected[jamStyleCount][4] = {
    {AMin7, FMaj7, CMaj7, GDom7},
    {DMin7, GDom7, CMaj7, CMaj7},
    {CMaj7, CMaj7, DMaj, DMaj},
    {GMaj, CMaj, GMaj, DDom7},
    {CMaj7, ADom7, DMin7, GDom7},
    {CMaj, FMaj, GMaj, CMaj},
    {AMin, FMaj, CMaj, GMaj},
    {AMin7, DDom7, EMin7, AMin7},
    {GMaj, FMaj, CMaj, GMaj},
    {CMaj, GMaj, AMin, FMaj},
    {CMaj, AMin, GMaj, CMaj},
    {ESus4, FOpenMaj7, AMin, ESus4},
    {GMaj, CMaj, DMaj, GMaj},
    {ADom7, ADom7, ADom7, ADom7},
  };
  int attacks[jamStyleCount]{};
  unsigned openingMasks[jamStyleCount]{};
  for (int style = 0; style < jamStyleCount; ++style) {
    Jam jam;
    assert(jam.status().automatic);
    jam.setStyle(style);
    jam.start();
    unsigned masks[4]{};
    for (int bar = 0; bar < 4; ++bar) {
      jam.beginBar();
      assert(jam.status().chord == expected[style][bar]);
      for (int step = 0; step < 16; ++step) {
        const auto lead = jam.beat(step).lead;
        if (!lead.velocity) continue;
        masks[bar] |= 1u << step;
        ++attacks[style];
        if (style == Ambient) assert(lead.gate >= 14);
        if (style == JazzSwing && step % 2 != 0) {
          assert(step % 4 == 1 && step < 13);
          const int target = jam.beat(step + 3).lead.note;
          const int approach = jam.beat(step + 1).lead.note;
          assert(lead.note + approach == 2 * target);
        }
      }
    }
    assert(masks[0] != masks[1] && masks[1] != masks[2]);
    openingMasks[style] = masks[0];
    for (int previous = 0; previous < style; ++previous)
      assert(openingMasks[previous] != openingMasks[style]);
  }
  assert(attacks[1] > attacks[0] && attacks[0] > attacks[2]);

  Jam jam;
  jam.start();
  jam.beginBar();
  jam.setStyle(1);
  for (int bar = 1; bar < 4; ++bar) {
    jam.beginBar();
    assert(jam.status().style == 0 && jam.status().chord == expected[0][bar]);
  }
  jam.beginBar();
  assert(jam.status().style == JazzSwing && jam.status().chord == DMin7);
  const int pickup = jam.beat(12).bass.note;
  assert((pickup + 1) % 12 == 7 || (pickup - 1) % 12 == 7);
  jam.selectChord(DbDom7);
  jam.beginBar();
  assert(jam.status().chord == DbDom7);
  const auto target = jam.beat(8).lead;
  assert(jamChord(DbDom7).contains(target.note));
  assert(jam.beat(5).lead.velocity && jam.beat(6).lead.velocity);
  assert(jam.beat(5).lead.note + jam.beat(6).lead.note == target.note * 2);
  jam.beginBar();
  assert(jam.status().chord == CMaj7);
}

void checkPulseAndModes() {
  assert(jamScaleTone(Ambient, CMaj7, 66) && !jamScaleTone(Ambient, CMaj7, 65)); // C Lydian, F#
  assert(jamScaleTone(RnB, AMin7, 66) && !jamScaleTone(RnB, AMin7, 65));        // A Dorian, F#
  assert(jamScaleTone(Rock, GMaj, 65) && !jamScaleTone(Rock, GMaj, 66));         // G Mixolydian, F
  assert(jamScaleTone(LoFi, AMin7, 65) && !jamScaleTone(LoFi, AMin7, 66));       // A natural minor
  assert(jamScaleTone(BossaNova, ADom7, 61) && !jamScaleTone(BossaNova, ADom7, 60)); // V of D minor

  for (int style = 0; style < jamStyleCount; ++style) {
    Jam jam;
    jam.setStyle(style);
    jam.start();
    int leadOn = 0, leadOff = 0, bassOn = 0, bassOff = 0;
    int barsWithAnchor = 0;
    for (int bar = 0; bar < 24; ++bar) {
      jam.beginBar();
      const int current = jam.status().chord;
      const auto& chord = jamChord(current);
      bool anchor = false;
      for (int step = 0; step < 16; ++step) {
        const auto beat = jam.beat(step);
        if (beat.bass.velocity) ++(step % 4 == 0 ? bassOn : bassOff);
        if (!beat.lead.velocity) continue;
        ++(step % 4 == 0 ? leadOn : leadOff);
        if (step % 8 == 0) {
          assert(chord.contains(beat.lead.note));
          anchor = true;
        }
        if (style != JazzSwing || step % 4 == 0)
          assert(chord.contains(beat.lead.note) || jamScaleTone(style, current, beat.lead.note));
      }
      barsWithAnchor += anchor;
    }
    assert(barsWithAnchor == 24);
    assert(leadOn >= 2 * leadOff);
    assert(bassOn > 0 && bassOn >= bassOff);
  }
}

void checkChordMapAndModes() {
  Jam jam;
  jam.setStyle(Pop);
  jam.setMode(false);
  jam.start();
  jam.beginBar();
  auto map = jamChordMap(jam.status());
  assert(map.chord[0]==CMaj && map.chord[1]==GMaj && map.chord[2]==AMin && map.chord[3]==FMaj);
  jam.setProgression(1);
  map = jamChordMap(jam.status());
  assert(map.chord[0]==AMin && map.chord[1]==FMaj && map.chord[2]==CMaj);
  jam.selectChord(map.chord[1]);
  assert(jam.status().waitingChord);
  jam.beginBar();
  assert(jam.status().chord==FMaj && !jam.status().waitingChord);

  jam.setScale(ScaleHarmonicMinor);
  jam.beginBar();
  assert(jam.status().scale==ScaleHarmonicMinor && jam.status().chord==FMaj);
  map = jamChordMap(jam.status());
  assert(map.chord[0]==AMin && map.chord[1]==DMin && map.chord[2]==EDom7);
  assert(jamScaleTone(Pop, AMin, 68, ScaleHarmonicMinor));
  assert(!jamScaleTone(Pop, AMin, 67, ScaleHarmonicMinor));
  assert(jamScaleTone(Pop, ESus4, 65, ScaleMiyako));
  assert(!jamScaleTone(Pop, ESus4, 67, ScaleMiyako));

  for (int style=0; style<jamStyleCount; ++style) {
    for (int scale=0; scale<jamScaleCount; ++scale) {
      for (int choice=0; choice<jamProgressionCount; ++choice) {
        JamStatus info{};
        info.style=style; info.scale=scale; info.progression=choice;
        const auto choices=jamChordMap(info);
        for (int d=0; d<jamDirectionCount; ++d) {
          assert(jamChord(choices.chord[d]).name[0]);
          for (int other=0; other<d; ++other) {
            if (choices.chord[d]==choices.chord[other])
              fprintf(stderr,"Duplicate chord at style %d scale %d preset %d direction %d\n",style,scale,choice,d);
            assert(choices.chord[d]!=choices.chord[other]);
          }
        }
      }
    }
  }
  assert(jamChordColor(CMaj)==ChordMajor && jamChordColor(AMin)==ChordMinor);
  assert(jamChordColor(ADom7)==ChordDominant && jamChordColor(ESus4)==ChordOpen);
  assert(jam.status().trackCells[3]==0x1111);
  Jam blues;
  blues.setStyle(Blues);
  blues.start();
  const int expected[] = {ADom7,ADom7,ADom7,ADom7,DDom7,DDom7,ADom7,ADom7,EDom7,DDom7,ADom7,EDom7,ADom7};
  for (int chord : expected) { blues.beginBar(); assert(blues.status().chord==chord); }
}

void checkTransposition() {
  Jam original, moved;
  original.seed(7426); moved.seed(7426);
  original.setStyle(Pop); moved.setStyle(Pop);
  original.setScale(ScaleMajor); moved.setScale(ScaleMajor);
  moved.setKey(7);
  char text[48];
  jamFormatScale(moved.status(), text, sizeof(text));
  assert(strcmp(text, "G MAJOR") == 0);
  jamFormatProgression(moved.status(), text, sizeof(text));
  assert(strcmp(text, "G D Em C") == 0);
  moved.setProgression(1);
  jamFormatProgression(moved.status(), text, sizeof(text));
  assert(strcmp(text, "Em C G D") == 0);
  moved.setProgression(0);
  const auto map = jamChordMap(moved.status());
  jamFormatChord(moved.status(), map.chord[1], text, sizeof(text));
  assert(strcmp(text, "D") == 0);
  jamFormatChord(moved.status(), map.chord[2], text, sizeof(text));
  assert(strcmp(text, "Em") == 0);
  assert(jamPitch(moved.status(), jamStringNote(CMaj, 0)) % 12 == 7);

  original.start(); moved.start();
  for (int bar=0; bar<8; ++bar) {
    original.beginBar(); moved.beginBar();
    for (int step=0; step<16; ++step) {
      const auto a=original.beat(step), b=moved.beat(step);
      auto compare=[](JamHit first, JamHit second) {
        assert(first.velocity==second.velocity && first.gate==second.gate);
        if (first.velocity) assert((second.note + 12 - first.note) % 12 == 7);
      };
      compare(a.bass,b.bass); compare(a.lead,b.lead);
      for(int t=0;t<3;++t) {
        assert(a.drums[t].note==b.drums[t].note && a.drums[t].velocity==b.drums[t].velocity);
        compare(a.harmony[t],b.harmony[t]);
      }
    }
  }
  moved.setKey(4);
  assert(jamKey(moved.status())==7 && moved.status().pendingKey==4);
  moved.beginBar();
  assert(jamKey(moved.status())==4 && moved.status().chord==CMaj);
  moved.beginBar();
  assert(moved.status().chord==GMaj); // Key changes preserve progression position.

  JamStatus spelling=moved.status();
  spelling.key=6;
  jamFormatChord(spelling,BHalfDim,text,sizeof(text));
  assert(strcmp(text,"E#m7b5")==0);
  for(int key=0;key<12;++key) {
    spelling.key=key;
    assert(jamKey(spelling)==key);
    assert((jamPitch(spelling,60)+12)%12==key);
  }
}

void checkBassAndDrums() {
  Jam synth, bossa, swing;
  synth.setStyle(Synthwave); bossa.setStyle(BossaNova); swing.setStyle(JazzSwing);
  synth.start(); synth.beginBar();
  bossa.start(); bossa.beginBar();
  swing.start(); swing.beginBar();
  assert(synth.beat(2).bass.note == synth.beat(0).bass.note + 12);
  assert(synth.beat(4).bass.note == synth.beat(0).bass.note);
  assert(!bossa.beat(4).bass.velocity && bossa.beat(8).bass.note % 12 == 7);
  for(int step=0;step<16;step+=4) assert(swing.beat(step).bass.velocity);
  assert(swing.beat(0).bass.note != swing.beat(4).bass.note);
  assert(synth.beat(12).drums[0].velocity && !bossa.beat(12).drums[0].velocity);

  // Separately exercise the same kick/snare/hat trigger with two styles.
  Song song{};
  defaults(song);
  song.echo=0;
  song.mute[Lead]=song.mute[Bass]=song.mute[Snare]=song.mute[Hat]=true;
  Engine rock, rim;
  rock.init(rate,song); rim.init(rate,song);
  rock.command({Action::JamStyle,0,0,0,Rock}); rim.command({Action::JamStyle,0,0,0,BossaNova});
  rock.command({Action::JamTransport}); rim.command({Action::JamTransport});
  int16_t first[512], second[512];
  rock.render(first,1); rim.render(second,1);
  rock.command({Action::Note,Kick,105,20,36,20});
  rim.command({Action::Note,Kick,105,20,36,20});
  rock.render(first,256); rim.render(second,256);
  assert(std::memcmp(first,second,sizeof first)!=0);
}

void checkLeadLevel() {
  Song song{};
  defaults(song);
  for(int track=0;track<Lead;++track) song.mute[track]=true;
  song.echo=0;
  Engine muted, clear;
  muted.init(rate,song); clear.init(rate,song);
  muted.command({Action::JamMode,0,0,0,0}); clear.command({Action::JamMode,0,0,0,0});
  muted.command({Action::JamLeadLevel,0,0,0,0});
  muted.command({Action::JamTransport}); clear.command({Action::JamTransport});
  muted.command({Action::JamLoopOn,Lead,110,10,2});
  clear.command({Action::JamLoopOn,Lead,110,10,2});
  int16_t quiet[1024], heard[1024];
  muted.render(quiet,512); clear.render(heard,512);
  int peak=0;
  for(int i=0;i<1024;++i) {
    assert(quiet[i]==0);
    if(heard[i]>peak) peak=heard[i];
  }
  assert(peak>300);
}

void checkGrooveVariations() {
  for (int style = 0; style < jamStyleCount; ++style) {
    Jam jam;
    jam.setStyle(style);
    jam.start();
    uint16_t rhythm[4][5]{};
    for (int bar = 0; bar < 4; ++bar) {
      jam.selectChord(CMaj7);  // Changes in rhythm must be audible on a stationary chord.
      jam.beginBar();
      for (int step = 0; step < 16; ++step) {
        const auto beat = jam.beat(step);
        for (int drum = 0; drum < 3; ++drum)
          if (beat.drums[drum].velocity) rhythm[bar][drum] |= 1u << step;
        if (beat.bass.velocity) rhythm[bar][3] |= 1u << step;
        if (beat.harmony[0].velocity) rhythm[bar][4] |= 1u << step;
      }
      assert(rhythm[bar][3] & 1); // Preserve the bass downbeat.
    }
    assert(std::memcmp(rhythm[0], rhythm[1], sizeof rhythm[0]) != 0);
  }
}

void checkDrumAnswers() {
  Jam jam;
  jam.setStyle(Pop);
  jam.setMode(false);
  jam.selectChord(CMaj);
  jam.start(); jam.beginBar();
  jam.tapDrum(0,3); jam.tapDrum(2,7); jam.tapDrum(1,16);
  jam.beginBar();
  assert(jam.beat(3).drums[0].velocity==98 && jam.beat(7).drums[2].velocity==76);
  assert(jam.status().replyCells);  // A drum phrase alone can invite a melodic answer.
  jam.beginBar();
  assert(jam.beat(0).drums[1].velocity==98);
  jam.beginBar();
  assert(jam.beat(7).drums[0].velocity==68 && jam.beat(11).drums[2].velocity==52);
  jam.stop(); jam.start(); jam.beginBar();
  assert(!jam.beat(3).drums[0].velocity && !jam.status().replyCells);

  for(int style:{Pop,BossaNova}) {
    for(bool automatic:{false,true}) {
      Jam player;
      player.setStyle(style); player.setMode(automatic); player.selectChord(CMaj);
      player.start(); player.beginBar();
      player.tapDrum(0,0); player.tapDrum(1,8); player.tapDrum(2,10);
      player.beginBar();
      assert(player.status().replyCells==0x0501);
      assert(player.beat(0).lead.gate==16 && player.beat(8).lead.gate==4);
      assert(player.beat(0).lead.note!=player.beat(8).lead.note);
      assert(player.beat(8).lead.note!=player.beat(10).lead.note);
      player.beginBar();
      assert(player.status().replyCells==(style==Pop?0x0501:0x0141));
      player.tapDrum(2,5); player.tapDrum(2,12); player.beginBar();
      assert(player.status().replyCells==0x1020);
    }
  }

  Song song{}; defaults(song);
  Engine e; e.init(rate,song);
  e.command({Action::Note,Kick,110,drumPadTag,36,100});
  e.command({Action::Note,Kick,90,0,36,100});
  assert(e.meter().triggers[Kick]==1);
  render(e,rate/20);
  e.command({Action::Note,Kick,90,0,36,100});
  assert(e.meter().triggers[Kick]==2);
}

void checkLoopPitchDevelopment() {
  for(bool taps:{false,true}) {
    Jam jam;
    jam.setMode(false); jam.setStyle(Pop); jam.selectChord(CMaj);
    jam.start(); jam.beginBar();
    const int steps[]={0,8,10}, notes[]={64,67,72};
    for(int i=0;i<3;++i) {
      if(taps) jam.tapDrum(0,steps[i]);
      else {
        jam.captureLoop(notes[i],90,steps[i],10);
        jam.releaseLoop(10,steps[i]+1);
      }
    }
    uint8_t lines[16][16]{};
    int distinct=0;
    for(int b=0;b<16;++b) {
      jam.beginBar();
      int count=0;
      for(int i=0;i<16;++i) {
        const auto note=jam.beat(i).lead;
        if(note.velocity) lines[b][count++]=note.note;
      }
      assert(count>0);
      if(b) assert(memcmp(lines[b],lines[b-1],16)!=0);
      bool fresh=true;
      for(int prior=0;prior<b;++prior) if(!memcmp(lines[b],lines[prior],16)) fresh=false;
      distinct+=fresh;
    }
    assert(distinct>=8);
  }
}

void checkThreeLoopStyles() {
  Jam lines[3];
  const int styles[]={RnB,ChipTune,Synthwave};
  for(int s=0;s<3;++s) {
    auto& jam=lines[s];
    jam.setStyle(styles[s]); jam.setScale(ScaleMajor); jam.setMode(false); jam.selectChord(CMaj);
    jam.start(); jam.beginBar();
    const int steps[]={0,8,10}, notes[]={64,67,72};
    for(int i=0;i<3;++i) {
      jam.captureLoop(notes[i],90,steps[i],10); jam.releaseLoop(10,steps[i]+2);
    }
    jam.beginBar();
    assert(jam.status().replyCells==0x0501 && jam.beat(0).lead.gate==8);
    jam.beginBar();
  }
  // The same opening fragment now enters late, answers an octave away, or holds.
  assert(lines[0].beat(5).lead.velocity && !lines[0].beat(4).lead.velocity);
  assert(lines[1].beat(4).lead.velocity && lines[1].beat(6).lead.velocity);
  assert(std::abs(lines[1].beat(4).lead.note-lines[1].beat(6).lead.note)==12);
  assert(lines[2].beat(4).lead.gate>lines[1].beat(4).lead.gate);
}

void checkAutoRecordedDevelopment() {
  const int steps[]={0,7,12}, pitches[]={64,67,65}, lengths[]={3,2,4};
  for(int style=0;style<jamStyleCount;++style) for(bool changeChords:{false,true}) {
    Jam jam;
    jam.setStyle(style); jam.setScale(ScaleMajor);
    jam.start(); jam.beginBar();
    jam.toggleCapture();
    for(int i=0;i<3;++i) {
      jam.capture(pitches[i],90,steps[i]*4000,4000,10);
      jam.releaseCapture(10,(steps[i]+lengths[i])*4000);
    }
    jam.toggleCapture();
    uint8_t lines[16][16]{};
    for(int b=0;b<16;++b) {
      if(!changeChords || b==0) jam.selectChord(CMaj);
      jam.beginBar();
      for(int step=0;step<16;++step) {
        const auto hit=jam.beat(step).lead;
        if(hit.velocity) lines[b][step]=hit.note;
      }
      if(!b) {
        assert(jam.status().replyCells==((1<<0)|(1<<7)|(1<<12)));
        for(int i=0;i<3;++i) {
          const auto hit=jam.beat(steps[i]).lead;
          assert(hit.note==pitches[i] && hit.gate==4*lengths[i]);
        }
      }
    }
    assert(memcmp(lines,lines+4,4*sizeof(lines[0]))!=0);
    assert(memcmp(lines+4,lines+8,4*sizeof(lines[0]))!=0);
    // Replace the motif. Its entire two-bar statement precedes any variation.
    jam.toggleCapture();
    jam.capture(64,90,0,4000,10); jam.releaseCapture(10,4000);
    jam.capture(67,90,17*4000,4000,10); jam.releaseCapture(10,23*4000);
    jam.toggleCapture();
    jam.selectChord(CMaj); jam.beginBar();
    assert(jam.status().replyCells==1 && jam.beat(0).lead.gate==4);
    jam.selectChord(GMaj); jam.beginBar();
    assert(jam.status().replyCells==2 && jam.beat(1).lead.gate==24);
    assert(jamChord(GMaj).contains(jam.beat(1).lead.note));
  }
}

void checkManualLoop() {
  Jam jam;
  jam.setMode(false);
  jam.start();
  jam.beginBar();
  assert(jam.status().waitingChord);
  assert(jam.beat(0).drums[0].velocity);
  assert(!jam.beat(0).bass.velocity && !jam.beat(0).lead.velocity);
  jam.selectChord(CMaj7);
  jam.beginBar();
  assert(!jam.status().waitingChord && !jam.status().userMotif);
  for (int step = 0; step < 16; ++step) {
    assert(bool(jam.beat(step).bass.velocity) == (step % 4 == 0));
    assert(!jam.beat(step).lead.velocity);
  }
  jam.captureLoop(64, 95, 4, 10);
  jam.releaseLoop(10, 5);
  jam.captureLoop(67, 82, 7, 11);
  jam.releaseLoop(11, 9);
  jam.captureLoop(71, 91, 12, 12);
  jam.releaseLoop(12, 14);
  assert(jam.status().loopInput);
  assert(jam.status().loopCells == ((1<<4)|(1<<7)|(1<<12)));
  jam.beginBar();
  assert(!jam.status().loopInput && jam.status().userMotif);
  assert(!jam.beat(0).lead.velocity);
  assert(jam.beat(4).lead.note == 64 && jam.beat(4).lead.gate == 4);
  assert(jam.beat(7).lead.note == 67 && jam.beat(7).lead.gate == 8);
  assert(jam.beat(12).lead.velocity && jam.beat(12).lead.note != 71);
  assert(jam.status().replyCells == ((1<<4)|(1<<7)|(1<<12)));

  unsigned masks[3]{};
  for (int bar = 0; bar < 3; ++bar) {
    jam.beginBar();
    for (int step = 0; step < 16; ++step)
      if (jam.beat(step).lead.velocity) masks[bar] |= 1u << step;
  }
  assert(masks[0] && masks[1] && masks[2] && masks[0] != masks[1] && masks[1] != masks[2]);
  jam.selectChord(GDom7);
  jam.beginBar();
  assert(jam.beat(0).bass.note % 12 == 7);
  jam.captureLoop(65, 92, 16, 10);
  assert(!jam.beat(4).lead.velocity);
  jam.beginBar();                         // The late pickup belongs to the new bar.
  assert(jam.status().loopInput);
  jam.releaseLoop(10, 2);
  jam.beginBar();
  assert(jam.status().motifNotes == 1);
  assert(jam.beat(0).lead.gate == 8);
}

// Same last question, two different endings taught in an earlier bar.
void checkLearnedReply() {
  for (int falling = 0; falling < 2; ++falling) {
    Jam jam;
    jam.setMode(false);
    jam.selectChord(CMaj);
    jam.start();
    jam.beginBar();
    auto tap = [&](int step, int note) {
      jam.captureLoop(note, 90, step, 1);
      jam.releaseLoop(1, step + 1);
    };
    tap(0, 64); tap(4, 67);
    const int gap = falling ? 6 : 3;
    tap(4 + gap, falling ? 60 : 74);
    jam.beginBar();
    tap(0, 60); tap(4, 63);  // Same upward third at a different pitch.
    jam.selectChord(GMaj);
    jam.beginBar();
    const auto opening = jam.beat(0).lead, ending = jam.beat(gap).lead;
    assert(opening.velocity && ending.velocity && ending.gate == 4);
    assert(jam.status().replyCells == ((1 << gap) | 1));
    assert(jamChord(GMaj).contains(opening.note) && jamChord(GMaj).contains(ending.note));
    assert(falling ? ending.note < opening.note : ending.note > opening.note);
  }
}

void checkAndRenderManualLoop(const char* filename = nullptr) {
  Song song{};
  defaults(song);
  song.bpm = 120;
  Engine engine;
  engine.init(rate, song);
  FILE* pcm = filename ? fopen(filename, "wb") : nullptr;
  assert(!filename || pcm);

  engine.command({Action::JamMode, 0, 0, 0, 0});
  engine.command({Action::JamTransport});
  render(engine, 1, pcm);
  assert(engine.meter().jam.waitingChord);
  assert(engine.meter().triggers[Kick] && !engine.meter().triggers[Bass]);
  engine.command({Action::JamChord, 0, 0, 0, CMaj7});
  render(engine, bar - 1 + 14200, pcm); // Slightly after the grid midpoint
  assert(!engine.meter().jam.waitingChord);
  engine.command({Action::JamLoopOn, Lead, 105, 10, 1}, 512);
  assert(engine.meter().jam.loopInput);
  render(engine, 5600, pcm);
  engine.command({Action::JamLoopOff, Lead, 0, 10});
  render(engine, 11000, pcm);
  engine.command({Action::JamLoopOn, Lead, 92, 11, 2});
  render(engine, 8100, pcm);
  engine.command({Action::JamLoopOff, Lead, 0, 11});
  render(engine, 9100, pcm);
  engine.command({Action::JamLoopOn, Lead, 91, 12, 3});
  render(engine, 10500, pcm);
  engine.command({Action::JamLoopOff, Lead, 0, 12});
  render(engine, bar - 58500 + 1, pcm);
  assert(engine.meter().jam.userMotif && engine.meter().jam.motifNotes == 3);
  const auto harmonyTriggers = engine.meter().triggers[Lead];
  render(engine, 11999, pcm); // The audible attack snaps to cell 4 after queue compensation.
  assert(engine.meter().triggers[Lead] == harmonyTriggers);
  render(engine, 1, pcm);
  assert(engine.meter().triggers[Lead] == harmonyTriggers + 1);
  engine.command({Action::JamChord, 0, 0, 0, GDom7});
  render(engine, bar * 4 - 12001, pcm);
  assert(engine.meter().jam.chord == GDom7);
  if (pcm) assert(fclose(pcm) == 0);
}

void renderStyles(const char* filename) {
  constexpr int tempo = 110;
  constexpr int framesPerStyle = rate * 16 * 60 / tempo;
  Song song{};
  defaults(song);
  song.bpm = tempo;
  FILE* pcm = fopen(filename, "wb");
  assert(pcm);
  for (int style = 0; style < jamStyleCount; ++style) {
    Engine engine;
    engine.init(rate, song);
    engine.command({Action::JamStyle, 0, 0, 0, int16_t(style)});
    engine.command({Action::JamTransport});
    render(engine, framesPerStyle, pcm);
    printf("%05.1fs  %s\n", double(style * framesPerStyle) / rate, jamStyleName(style));
  }
  assert(fclose(pcm) == 0);
}

void renderComparison(const char* filename, bool rhythmOnly) {
  constexpr int tempo=110;
  constexpr int framesPerStyle=rate*16*60/tempo;
  const int styles[]={Pop,JazzSwing,BossaNova,Synthwave,Rock};
  FILE* pcm=fopen(filename,"wb");
  assert(pcm);
  for(int style:styles) {
    Song song{};
    defaults(song);
    song.bpm=tempo;
    song.mute[Lead]=rhythmOnly;
    Engine engine;
    engine.init(rate,song);
    engine.seedJam(7426);
    engine.command({Action::JamStyle,0,0,0,int16_t(style)});
    if(style==Pop||style==JazzSwing) {
      engine.command({Action::JamScaleMode,0,0,0,ScaleMajor});
      engine.command({Action::JamKey,0,0,0,7});
    }
    engine.command({Action::JamTransport});
    render(engine,framesPerStyle,pcm);
  }
  assert(fclose(pcm)==0);
}

int main(int argc, char** argv) {
  checkHarmony();
  checkCapturedMotif();
  checkPlayedLengths();
  checkGeneratedLengths();
  checkAcrossBar();
  checkStyles();
  checkPulseAndModes();
  checkManualLoop();
  checkLoopPitchDevelopment();
  checkAutoRecordedDevelopment();
  checkThreeLoopStyles();
  checkDrumAnswers();
  checkLearnedReply();
  checkGrooveVariations();
  checkChordMapAndModes();
  checkTransposition();
  checkBassAndDrums();
  checkLeadLevel();
  if (argc == 3 && strcmp(argv[1], "--loop") == 0) {
    checkAndRenderManualLoop(argv[2]);
    return 0;
  }
  if(argc==3 && (strcmp(argv[1],"--compare")==0||strcmp(argv[1],"--grooves")==0)) {
    renderComparison(argv[2],strcmp(argv[1],"--grooves")==0);
    return 0;
  }
  checkAndRenderManualLoop();
  if (argc == 3 && strcmp(argv[1], "--styles") == 0) {
    renderStyles(argv[2]);
    return 0;
  }
  Song song{};
  defaults(song);
  song.bpm = 120;
  Engine engine;
  engine.init(rate, song);
  FILE* pcm = argc > 1 ? fopen(argv[1], "wb") : nullptr;
  assert(argc == 1 || pcm);

  engine.command({Action::JamMode, 0, 0, 0, 0});
  engine.command({Action::JamChord, 0, 0, 0, CMaj7});
  engine.command({Action::JamTransport});
  render(engine, 1, pcm);
  assert(engine.meter().jam.chord == 0);
  assert(engine.meter().jam.motifNotes == 0);
  render(engine, 12000, pcm);
  assert(engine.meter().triggers[Bass] > 0);
  assert(engine.meter().triggers[Lead] > 0);

  engine.command({Action::JamChord, 0, 0, 0, 4});
  assert(engine.meter().jam.pendingChord == 4);
  render(engine, bar - 12001, pcm);
  assert(engine.meter().jam.chord == 0);
  render(engine, 1, pcm);
  assert(engine.meter().jam.chord == 4 && engine.meter().jam.pendingChord == -1);

  engine.command({Action::JamMode, 0, 0, 0, 1});
  engine.command({Action::JamChord, 0, 0, 0, 7});
  render(engine, bar, pcm);
  assert(engine.meter().jam.chord == 7);
  render(engine, bar, pcm);
  assert(engine.meter().jam.chord == FMaj7);

  engine.command({Action::JamCapture});
  render(engine, 2000, pcm);
  assert(engine.meter().jam.capturing);
  engine.command({Action::Note, Lead, 100, 10, 64, 180});
  render(engine, 8000, pcm);
  engine.command({Action::Note, Lead, 100, 11, 67, 180});
  render(engine, 8000, pcm);
  engine.command({Action::Note, Lead, 100, 12, 69, 300});
  engine.command({Action::JamCapture});
  render(engine, bar, pcm);
  assert(engine.meter().jam.motifNotes == 3);
  assert(engine.meter().jam.userMotif && !engine.meter().jam.capturing);

  engine.command({Action::JamStyle, 0, 0, 0, 1});
  engine.command({Action::JamPreset, Lead, 0, 0, 3});
  assert(engine.meter().jam.style == 0 && engine.meter().jam.pendingStyle == 1);
  render(engine, bar, pcm);
  assert(engine.meter().jam.instrument == 3);
  render(engine, bar * 7, pcm);
  engine.command({Action::JamStyle, 0, 0, 0, 2});
  engine.command({Action::JamProgression, 0, 0, 0, 0});
  render(engine, bar * 8, pcm);
  assert(engine.meter().jam.style == 2);
  assert(engine.meter().jam.userMotif);

  engine.command({Action::JamTransport});
  assert(!engine.meter().playing);
  render(engine, rate, pcm);
  if (pcm) assert(fclose(pcm) == 0);
  puts("Jam harmony, manual Loop quantization and motif development, transposition, bass profiles, drum kits and 14 grooves passed");
}
