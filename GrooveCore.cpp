// Modified for AutoJam, 2026. See NOTICE.md.
// SPDX-License-Identifier: Apache-2.0
#include "GrooveCore.h"
#include <math.h>
#include <string.h>
namespace groove {
const char* const chordNames[]={"C","Dm","Em","F","G","Am","Bdim","G7"};
const char* const optionNames[]={"TEMPO","SWING","MASTER","ECHO","PATTERN","RECORD","LED SHOW","SAVE","LOAD","CLEAR","DEFAULTS","UPLOADER"};
const char* const trackNames[]={"KICK","SNARE","HAT","BASS","LEAD"};
const char* const presetNames[]={"GLASS EP","ROUND BASS","PLUCK","BELL","SOFT ORGAN","MARIMBA","FM BRASS","SPACE","CHIP PULSE"};
const char* const parameterNames[]={"RATIO","FM DEPTH","ATTACK","DECAY","SUSTAIN","RELEASE","TONE","LEVEL"};
namespace {
struct DrumKit {
  float duration[3], frequency[3], body[3], noise[3], filter, kickSweep;
};
constexpr DrumKit kits[]={
  {{.18f,.12f,.04f},{45,180,0},{.70f,.35f,0},{0,.38f,.27f},.12f,110},     // Original
  {{.15f,.10f,.05f},{58,152,0},{.73f,.43f,0},{0,.21f,.19f},.28f,74},     // Dusty
  {{.095f,.16f,.23f},{66,226,1420},{.62f,.13f,.11f},{0,.41f,.24f},.09f,60}, // Brush and ride
  {{.12f,.037f,.032f},{60,438,1040},{.64f,.55f,.11f},{0,.10f,.15f},.19f,48}, // Rim and shaker
  {{.21f,.19f,.070f},{48,160,0},{.81f,.43f,0},{0,.50f,.33f},.10f,169},    // Punchy kit
  {{.075f,.055f,.03f},{81,242,1850},{.69f,.12f,.07f},{0,.48f,.30f},.08f,58}, // Short electronic
  {{.27f,.24f,.095f},{52,148,970},{.80f,.29f,.06f},{0,.60f,.29f},.13f,132}, // Drum machine
  {{.11f,.055f,.061f},{72,288,720},{.64f,.38f,.08f},{0,.20f,.17f},.18f,55},  // Light acoustic
};
constexpr uint8_t jamKits[jamStyleCount]={1,2,7,7,3,5,6,1,4,0,7,7,7,2};
}
static int jamLeadPreset(const JamStatus& jam) {
  constexpr uint8_t styleLeads[jamStyleCount]={0, 0, 3, 2, 5, chipPulsePreset, 6, 4, 6, 0, 5, 3, 2, 6};
  return jam.instrument==autoJamInstrument?styleLeads[jam.style]:jam.instrument;
}
static int clamp(int v,int lo,int hi) { return v<lo?lo:(v>hi?hi:v); }
int parameterLimit(int i) {
  const int limits[]={32,500,2000,2500,100,3000,100,100};
  return i>=0&&i<parameterCount?limits[i]:0;
}
Patch preset(int i) {
  static const uint16_t values[presetCount][parameterCount]={
    {4,160,4,620,18,500,78,85}, {2,90,3,300,55,140,38,100},
    {4,220,2,260,5,180,68,90}, {14,280,3,1400,0,1100,85,78},
    {4,50,25,250,85,220,60,80}, {8,230,2,190,0,180,65,90},
    {4,300,60,280,70,250,65,78}, {6,180,300,900,60,1600,55,75},
    {4,0,2,110,70,45,90,75}
  };
  Patch p{}; p.preset=clamp(i,0,presetCount-1);
  memcpy(p.values,values[p.preset],sizeof(p.values)); return p;
}
void defaults(Song& s) {
  memset(&s,0,sizeof(s)); s.bpm=110; s.master=75; s.echo=18; s.led=1;
  s.patches[0]=preset(1); s.patches[1]=preset(0);
  for(int t=0;t<trackCount;++t) s.levels[t]=t==Hat?55:80;
  const uint16_t masks[bankCount][trackCount]={
    {0x1111,0x1010,0x5555,0x1111,0x0404}, {0x1141,0x1010,0xffff,0x4511,0x2204},
    {0x4921,0x1040,0x5555,0x4921,0x0422}, {0x1111,0x1010,0xaaaa,0x4444,0x1249}
  };
  const uint8_t bass[]={36,36,43,36,36,36,46,43,36,36,43,36,34,34,43,46};
  const uint8_t lead[]={60,67,70,72,67,63,60,67,72,70,67,63,60,63,67,70};
  for(int b=0;b<bankCount;++b) for(int t=0;t<trackCount;++t) for(int i=0;i<stepCount;++i)
    s.steps[b][t][i]={uint8_t(t==Bass?bass[i]:(t==Lead?lead[i]:36+t*2)),uint8_t(i%4==0?112:88),uint8_t((masks[b][t]>>i)&1)};
}
void sanitize(Song& s) {
  s.bpm=clamp(s.bpm,30,300); s.swing=clamp(s.swing,0,60); s.master=clamp(s.master,0,100);
  s.echo=clamp(s.echo,0,60); s.led=!!s.led; s.bank=clamp(s.bank,0,bankCount-1);
  for(int t=0;t<trackCount;++t) { s.mute[t]=!!s.mute[t]; s.levels[t]=clamp(s.levels[t],0,100); }
  for(auto& p:s.patches) {
    p.preset=clamp(p.preset,0,presetCount-1);
    for(int i=0;i<parameterCount;++i) p.values[i]=clamp(p.values[i],i==Ratio?1:0,parameterLimit(i));
  }
  for(auto& bank:s.steps) for(auto& track:bank) for(auto& step:track) {
    step.note=clamp(step.note,24,96); step.velocity=clamp(step.velocity,1,127); step.enabled=!!step.enabled;
  }
}
float midiFrequency(int note) { return 440.f*powf(2.f,(note-69)/12.f); }
void Engine::init(uint32_t sampleRate,const Song& s) {
  sampleRate_=sampleRate; daisysp::Oscillator::InitSineTable();
  for(auto& v:voices_) { v.fm.Init(sampleRate); v.envelope.Init(sampleRate); }
  restore(s);
}
void Engine::restore(const Song& s) {
  jam_.stop(clock_); harmonyPatch_=preset(4); jamBassPatch_=preset(1);
  song_=s; sanitize(song_); meter_=Meter{}; clock_=0; nextStep_=0; lastStepFrames_=0; stepRemainder_=0; bend_=0; harmonyGain_=1;
  meter_.jam=jam_.status();
  for(auto& v:voices_) { v.active=false; v.tail=0; }
  for(auto& d:drum_) d.env=0;
  kickPadPriorityUntil_=0; kickAttack_=0; kickAttackDecay_=expf(-1.f/(sampleRate_*.052f));
  memset(delay_,0,sizeof(delay_)); delayPos_=0; dcX_=dcY_=0;
}
void Engine::trigger(int track,int note,int velocity,uint32_t gateFrames,int tag,const Patch* patch) {
  if(track<0||track>=trackCount||song_.mute[track]) return;
  if(track==Kick && tag!=drumPadTag && clock_<kickPadPriorityUntil_) return;
  velocity=clamp(velocity,1,127); note=clamp(note,24,96); ++meter_.triggers[track];
  if(meter_.recording&&meter_.playing&&tag>=10) {
    const int step=clock_>=nextStep_?(meter_.step+1)%stepCount:meter_.step;
    song_.steps[song_.bank][track][step]={uint8_t(note),uint8_t(velocity),1};
  }
  if(track<3) {
    Drum& d=drum_[track]; d.env=1; d.phase=0; d.filter=0;
    const auto& kit=kits[jam_.status().active?jamKits[jam_.status().style]:0];
    d.decay=expf(-1.f/(sampleRate_*kit.duration[track]));
    d.frequency=kit.frequency[track];
    d.sweep=track==Kick?kit.kickSweep:0;
    d.body=kit.body[track];
    d.noise=kit.noise[track];
    d.cutoff=kit.filter;
    d.gain=(velocity/127.f)*(song_.levels[track]/100.f);
    if(track==Kick) {
      kickAttack_=tag==drumPadTag ? 1.f : 0.f;
      if(kickAttack_) { d.gain*=1.65f; kickPadPriorityUntil_=clock_+sampleRate_/25; }
    }
    return;
  }
  Voice* chosen=nullptr;
  float best=100;
  for(auto& v:voices_) {
    if(!v.active) { chosen=&v; break; }
    const bool heldLead=jam_.status().active&&v.track==Lead&&v.gate&&(v.tag==4||v.tag>=10);
    const float cost=v.amplitude+(v.gate?2.f:0.f)+(heldLead?4.f:0.f);
    if(cost<best) { chosen=&v; best=cost; }
  }
  Voice& v=*chosen;
  // Fade the stolen voice from its last sample to avoid a discontinuity.
  const float tail=v.active?v.filtered*v.amplitude*v.gain:0;
  const Patch& p=patch?*patch:song_.patches[track-Bass];
  const float frequency=midiFrequency(note)*powf(2.f,bend_/12.f);
  const float ratio=fminf(p.values[Ratio]/4.f,sampleRate_*.45f/frequency);
  v.fm.Reset(); v.fm.SetFrequency(frequency); v.fm.SetRatio(ratio);
  v.pulsePhase=0; v.pulseStep=p.preset==chipPulsePreset?frequency/sampleRate_:0;
  v.envelope.Init(sampleRate_);
  v.envelope.SetTime(daisysp::ADSR_SEG_ATTACK,fmaxf(.002f,p.values[Attack]/1000.f));
  v.envelope.SetTime(daisysp::ADSR_SEG_DECAY,fmaxf(.01f,p.values[Decay]/1000.f));
  v.envelope.SetTime(daisysp::ADSR_SEG_RELEASE,fmaxf(.01f,p.values[Release]/1000.f));
  v.envelope.SetSustainLevel(p.values[Sustain]/100.f);
  v.track=track; v.note=note; v.tag=tag; v.gate=gateFrames; v.active=true;
  v.gain=(velocity/127.f)*(p.values[Level]/100.f)*.20f;
  // Convert DaisySP FM2 phase increment to a pitch-independent FM index.
  // This keeps modulation depth and preset brightness consistent across pitches.
  v.depth=(p.values[Index]/100.f)*5.f*frequency*ratio/sampleRate_;
  v.tone=.015f+p.values[Tone]*.006f;
  v.filtered=0; v.amplitude=0; v.tail=tail;
}
int Engine::loopGridStep(uint32_t inputDelayFrames) const {
  if (!lastStepFrames_) return 0;
  const auto offset = int64_t(clock_) - int64_t(nextStep_) + lastStepFrames_;
  // Compensate for the audio queued ahead of what the player heard, bounded to half a cell.
  const auto delay = inputDelayFrames < lastStepFrames_ / 2 ? inputDelayFrames : lastStepFrames_ / 2;
  return meter_.step + ((offset - delay) * 2 >= lastStepFrames_ ? 1 : 0);
}
void Engine::command(const Command& c,uint32_t inputDelayFrames) {
  const int t=clamp(c.track,0,trackCount-1),i=clamp(c.item,0,stepCount-1);
  switch(c.action) {
    case Action::JamTransport:
      if(jam_.status().active) {
        jam_.stop(clock_); meter_.playing=false;
        for(auto& v:voices_) v.gate=0;
      } else {
        jam_.start(); meter_.playing=true; meter_.recording=false;
        meter_.step=15; nextStep_=clock_; stepRemainder_=0;
      }
      break;
    case Action::JamChord: jam_.selectChord(c.value); break;
    case Action::JamMode: jam_.setMode(c.value!=0); break;
    case Action::JamProgression: jam_.setProgression(c.value); break;
    case Action::JamStyle: jam_.setStyle(c.value); break;
    case Action::JamScaleMode: jam_.setScale(c.value); break;
    case Action::JamKey: jam_.setKey(c.value); break;
    case Action::JamLeadLevel: meter_.jamLeadLevel=clamp(c.value,0,200); break;
    case Action::JamPreset: jam_.setInstrument(c.value==autoJamInstrument?autoJamInstrument:clamp(c.value,0,presetCount-1)); break;
    case Action::JamCapture:
      jam_.toggleCapture(clock_);
      if(jam_.status().capturing)
        for(auto& v:voices_) if(v.tag==4) v.gate=0;
      break;
    case Action::Transport:
      jam_.stop(clock_);
      meter_.playing=!meter_.playing;
      if(meter_.playing) { meter_.step=15; nextStep_=clock_; stepRemainder_=0; }
      else { meter_.recording=false; for(auto& v:voices_) if(v.tag<10) v.gate=0; }
      break;
    case Action::Record:
      meter_.recording=c.value==0?!meter_.recording:c.value>0;
      if(meter_.recording&&!meter_.playing) { meter_.playing=true; meter_.step=15; nextStep_=clock_; stepRemainder_=0; }
      break;
    case Action::Tempo: song_.bpm=clamp(c.value,30,300); break;
    case Action::Swing: song_.swing=clamp(c.value,0,60); break;
    case Action::Master: song_.master=clamp(c.value,0,100); break;
    case Action::Echo: song_.echo=clamp(c.value,0,60); break;
    case Action::Led: song_.led=!!c.value; break;
    case Action::Bank:
      if(meter_.playing) meter_.pendingBank=clamp(c.value,0,bankCount-1);
      else song_.bank=clamp(c.value,0,bankCount-1); break;
    case Action::Preset: if(t>=Bass) song_.patches[t-Bass]=preset(c.value); break;
    case Action::Param: if(t>=Bass&&c.item<parameterCount) song_.patches[t-Bass].values[c.item]=clamp(c.value,c.item==Ratio?1:0,parameterLimit(c.item)); break;
    case Action::Mute:
      song_.mute[t]=!song_.mute[t];
      if(song_.mute[t]) { if(t<3) drum_[t].env=0; for(auto& v:voices_) if(v.track==t) v.gate=0; } break;
    case Action::TrackLevel: song_.levels[t]=clamp(c.value,0,100); break;
    case Action::ToggleStep: song_.steps[song_.bank][t][i].enabled^=1; break;
    case Action::StepNote: song_.steps[song_.bank][t][i].note=clamp(c.value,24,96); break;
    case Action::StepVelocity: song_.steps[song_.bank][t][i].velocity=clamp(c.value,1,127); break;
    case Action::JamLoopOn: {
      const int note=jamStringNote(jam_.status().chord,clamp(c.value,0,5));
      const bool wasLoopInput=jam_.status().loopInput;
      if(jam_.status().capturing) jam_.capture(note,c.item,clock_,sampleRate_*15/song_.bpm,c.tag);
      else if(jam_.captureLoop(note,c.item,loopGridStep(inputDelayFrames),c.tag)&&!wasLoopInput) {
        for(auto& voice:voices_) if(voice.active&&voice.tag==4) {
          voice.gate=0;
          voice.envelope.SetTime(daisysp::ADSR_SEG_RELEASE,.030f);
        }
      }
      Patch patch=preset(jamLeadPreset(jam_.status()));
      patch.values[Attack]=patch.values[Attack]>16?16:patch.values[Attack];
      patch.values[Release]=patch.values[Release]>45?45:patch.values[Release];
      if(patch.values[Sustain]<55) patch.values[Sustain]=55;
      trigger(Lead,jamPitch(jam_.status(),note),c.item,UINT32_MAX,c.tag,&patch);
      break;
    }
    case Action::JamLoopOff:
      jam_.releaseCapture(c.tag,clock_);
      jam_.releaseLoop(c.tag,loopGridStep(inputDelayFrames));
      for(auto& voice:voices_) if(voice.active&&voice.tag==c.tag) voice.gate=0;
      break;
    case Action::JamLiveNote: {
      const int note=jamStringNote(jam_.status().chord,clamp(c.value,0,5));
      if(jam_.status().capturing) jam_.capture(note,c.item,clock_,sampleRate_*15/song_.bpm);
      const Patch patch=preset(jamLeadPreset(jam_.status()));
      trigger(Lead,jamPitch(jam_.status(),note),c.item,c.gateMs*sampleRate_/1000,c.tag,&patch);
      break;
    }
    case Action::Note:
      if(t<=Hat && c.tag==drumPadTag+t && !song_.mute[t]) jam_.tapDrum(t,loopGridStep(inputDelayFrames));
      if(t==Lead&&c.tag>=10&&c.tag<=15&&jam_.status().capturing)
        jam_.capture(c.value,c.item,clock_,sampleRate_*15/song_.bpm);
      trigger(t,c.value,c.item,c.gateMs*sampleRate_/1000,c.tag);
      break;
    case Action::Off: for(auto& v:voices_) if(v.tag==c.tag) v.gate=0; break;
    case Action::Bend: {
      const float next=clamp(c.value,-200,200)/100.f;
      const float scale=powf(2.f,(next-bend_)/12.f); bend_=next;
      for(auto& v:voices_) if(v.active) { v.fm.SetFrequency(midiFrequency(v.note)*powf(2.f,bend_/12.f)); v.depth*=scale; v.pulseStep*=scale; }
      break;
    }
    case Action::Panic:
      jam_.stop(clock_);
      meter_.playing=false; meter_.recording=false;
      for(auto& v:voices_) { v.gate=0; v.envelope.SetTime(daisysp::ADSR_SEG_RELEASE,.01f); }
      for(auto& d:drum_) d.env=0;
      memset(delay_,0,sizeof(delay_)); break;
    case Action::ClearBank:
      for(auto& track:song_.steps[song_.bank]) for(auto& step:track) step.enabled=0; break;
  }
  meter_.jam=jam_.status();
}
void Engine::advanceStep() {
  meter_.step=(meter_.step+1)%stepCount;
  if(meter_.step==0&&meter_.pendingBank<bankCount) { song_.bank=meter_.pendingBank; meter_.pendingBank=255; }
  const bool improvising=jam_.status().active;
  if(improvising&&meter_.step==0) {
    const auto oldChord=jam_.status().chord;
    const int oldOffset=jamPitch(jam_.status(),0);
    jam_.beginBar();
    meter_.jam=jam_.status();
    constexpr uint8_t harmonyPresets[jamStyleCount]={0, 2, 7, 2, 2, chipPulsePreset, 7, 0, 6, 4, 2, 7, 2, 0};
    harmonyPatch_=preset(harmonyPresets[meter_.jam.style]);
    harmonyPatch_.values[Level]=harmonyPatch_.values[Level]*3/4;
    jamBassPatch_=preset(1);
    constexpr uint8_t bassDepth[jamStyleCount]={124,88,71,92,100,125,154,135,160,105,92,65,100,114};
    jamBassPatch_.values[Ratio]=4;
    jamBassPatch_.values[Index]=bassDepth[meter_.jam.style];
    jamBassPatch_.values[Tone]=70;
    jamBassPatch_.values[Release]=(meter_.jam.style==Synthwave||meter_.jam.style==Rock)?85:130;
    if(meter_.jam.style==Bluegrass || meter_.jam.style==JazzSwing) harmonyPatch_.values[Release]=85;
    if(meter_.jam.style==BossaNova) {
      harmonyPatch_.values[Index]=105;
      harmonyPatch_.values[Tone]=48;
    }
    const int offset=jamPitch(meter_.jam,0);
    if(oldChord!=meter_.jam.chord||oldOffset!=offset) {
      for(auto& v:voices_) if(v.active&&(v.tag==3||v.tag==5)&&!jamChord(meter_.jam.chord).contains(int(v.note)-offset)) {
        v.gate=0;
        v.envelope.SetTime(daisysp::ADSR_SEG_RELEASE,.055f);
      }
    }
  }
  const double base=sampleRate_*15.0/song_.bpm;
  // Stretch the first eighth of the beat, not alternating sixteenths, for Swing.
  const double swing=improvising?(!meter_.jam.automatic?0:
    (meter_.jam.style==JazzSwing||meter_.jam.style==Blues?.33:(meter_.jam.style==LoFi?.08:0))):song_.swing/100.0;
  const bool early=improvising?meter_.step%4<2:meter_.step%2==0;
  const double duration=base*(1.0+(early?1:-1)*swing)+stepRemainder_;
  const uint32_t frames=uint32_t(duration); stepRemainder_=duration-frames; nextStep_=clock_+frames; lastStepFrames_=frames;
  if(improvising) { playJamStep(frames,swing); return; }
  for(int t=0;t<trackCount;++t) {
    const Step& step=song_.steps[song_.bank][t][meter_.step];
    if(step.enabled) trigger(t,step.note,step.velocity,frames*3/4,t);
  }
}
void Engine::playJamStep(uint32_t frames,double swing) {
  const JamBeat beat=jam_.beat(meter_.step);
  for(int t=0;t<3;++t) {
    const auto& hit=beat.drums[t];
    if(hit.velocity) trigger(t,hit.note,hit.velocity,frames,t);
  }
  if(beat.bass.velocity) {
    for(auto& voice:voices_) if(voice.active&&voice.tag==3) voice.gate=0;
    trigger(Bass,beat.bass.note,beat.bass.velocity,frames*beat.bass.gate/4,3,&jamBassPatch_);
  }
  for(const auto& hit:beat.harmony)
    if(hit.velocity) trigger(Lead,hit.note,hit.velocity,frames*hit.gate/4,5,&harmonyPatch_);
  if(beat.lead.velocity) {
    for(auto& voice:voices_) if(voice.active&&voice.tag==4&&voice.gate) voice.gate=0;
    Patch patch=preset(jamLeadPreset(meter_.jam));
    if(meter_.jam.style!=Ambient) {
      // A long FM attack can hide the actual beat even with a quantized trigger.
      patch.values[Attack]=patch.values[Attack]>28?28:patch.values[Attack];
      patch.values[Release]=patch.values[Release]>230?230:patch.values[Release];
    }
    if(!meter_.jam.automatic) {
      patch.values[Attack]=patch.values[Attack]>16?16:patch.values[Attack];
      patch.values[Release]=patch.values[Release]>45?45:patch.values[Release];
    }
    if(patch.values[Sustain]<55) patch.values[Sustain]=55;
    double cells=0;
    for(int units=0;units<beat.lead.gate;units+=4) {
      const int remaining=beat.lead.gate-units;
      const double portion=(remaining<4?remaining:4)*.25;
      const bool longCell=(meter_.step+units/4)%4<2;
      cells+=portion*(1+(longCell?swing:-swing));
    }
    const uint32_t gateFrames=uint32_t(cells*sampleRate_*15/song_.bpm+.5);
    trigger(Lead,beat.lead.note,beat.lead.velocity,gateFrames,4,&patch);
  }
}
float Engine::random() {
  noise_^=noise_<<13; noise_^=noise_>>17; noise_^=noise_<<5;
  return (int32_t(noise_)/2147483648.f);
}
float Engine::drums() {
  float mix=0;
  for(int i=0;i<3;++i) {
    Drum& d=drum_[i]; if(d.env<.0001f) continue;
    const float frequency=d.frequency+d.sweep*d.env*d.env;
    d.phase+=frequency/sampleRate_;
    if(d.phase>=1) d.phase-=1;
    float sound=d.body?d.body*daisysp::Oscillator::Sine(d.phase):0;
    if(i==Kick && kickAttack_>.001f) {
      sound+=kickAttack_*(.48f*daisysp::Oscillator::Sine(3*d.phase)+.24f*daisysp::Oscillator::Sine(6*d.phase));
      kickAttack_*=kickAttackDecay_;
    }
    if(d.noise) {
      const float value=random();
      d.filter+=d.cutoff*(value-d.filter);
      sound+=d.noise*(value-d.filter);
    }
    mix+=sound*d.env*d.gain; d.env*=d.decay;
  }
  return mix;
}
void Engine::render(int16_t* output,int frames) {
  float peak=0; int active=0;
  const bool jamActive=jam_.status().active;
  const bool loop=jamActive&&!jam_.status().automatic;
  const float leadGain=meter_.jamLeadLevel/100.f;
  for(int n=0;n<frames;++n,++clock_) {
    if(meter_.playing&&clock_>=nextStep_) advanceStep();
    float mix=drums(), harmony=0, presence=0;
    for(auto& v:voices_) {
      if(!v.active) continue;
      const float env=v.envelope.Process(v.gate>0); if(v.gate) --v.gate;
      v.amplitude=env;
      // Reduce brightness with the envelope while retaining the FM attack.
      v.fm.SetIndex(v.depth*(.25f+.75f*env));
      float sample;
      if(v.pulseStep>0) {
        // The first three odd harmonics give a pulse-like timbre without aliased far overtones.
        const float phase=v.pulsePhase;
        const auto sine=daisysp::Oscillator::Sine;
        const float third=phase*3.f, fifth=phase*5.f;
        sample=sine(phase)+sine(third-int(third))/3.f+sine(fifth-int(fifth))/5.f;
        v.pulsePhase+=v.pulseStep;
        if(v.pulsePhase>=1.f) v.pulsePhase-=1.f;
      } else sample=v.fm.Process();
      v.filtered+=v.tone*(sample-v.filtered);
      const float voice=v.filtered*env*v.gain*(song_.levels[v.track]/100.f)+v.tail;
      if(jamActive&&v.tag==5) harmony+=voice;
      else if(jamActive&&v.track==Lead&&(v.tag==4||v.tag>=10)) {
        mix+=voice*leadGain;
        presence=fmaxf(presence,env);
      } else mix+=voice;
      v.tail*=.995f;
      if(!v.envelope.IsRunning()&&fabsf(v.tail)<.00001f) v.active=false;
    }
    const float targetGain=1.f-.43f*fminf(1.f,presence*leadGain);
    harmonyGain_+=(targetGain-harmonyGain_)*(targetGain<harmonyGain_?.006f:.00055f);
    mix+=harmony*harmonyGain_;
    const uint32_t tap=(delayPos_+8192-uint32_t(sampleRate_*.185f))&8191;
    const float echo=delay_[tap]; delay_[delayPos_]=mix+echo*.35f; delayPos_=(delayPos_+1)&8191;
    if(!loop) mix+=echo*(song_.echo/100.f);
    // Remove DC, smooth gain and limit the mix before integer conversion.
    const float dc=mix-dcX_+.995f*dcY_; dcX_=mix; dcY_=dc;
    volume_+=.001f*(song_.master/100.f-volume_);
    const float scaled=dc*volume_, limited=scaled/(1.f+fabsf(scaled));
    const int16_t pcm=int16_t(limited*30000.f); output[2*n]=pcm; output[2*n+1]=pcm;
    peak=fmaxf(peak,fabsf(limited)); if(n<32) meter_.wave[n]=int8_t(limited*110.f);
  }
  for(const auto& v:voices_) if(v.active) ++active;
  meter_.voices=active; meter_.peak=fmaxf(peak,meter_.peak*.85f);
}
}
