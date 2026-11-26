// Modified for AutoJam, 2026. See NOTICE.md.
// SPDX-License-Identifier: Apache-2.0
#include "GrooveRuntime.h"
#include "config.h"
#ifdef GROOVE_HOST_TEST
#include "tests/CanvasMock.h"
#include "tests/Export.h"
#else
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#endif
#include <string.h>

namespace groove {
namespace {
constexpr uint16_t bg=0x0000, white=0xffff, dim=0x7bcf, line=0x2104;
constexpr uint16_t cyan=0x4e9f, green=0x9f66, coral=0xfb4d, yellow=0xfe47;
constexpr uint16_t trackColors[trackCount]={coral,yellow,green,0x3db8,cyan};
#ifndef GROOVE_HOST_TEST
class Panel:public Adafruit_ST7735 {
 public:
  Panel():Adafruit_ST7735(hw::tftCs,hw::tftDc,hw::tftReset) {}
  void offset() { setColRowStart(24,0); }
};
Panel panel;
#endif
GFXcanvas16 canvas(160,80);
#ifndef GROOVE_HOST_TEST
uint16_t previous[160*80];
uint16_t strip[160*8];
bool firstFrame=true;
#endif
void text(int x,int y,const char* value,uint16_t color=white,int size=1) {
  canvas.setFont(nullptr); canvas.setTextSize(size); canvas.setTextWrap(false);
  canvas.setTextColor(color); canvas.setCursor(x,y); canvas.print(value);
}
void right(int edge,int y,const char* value,uint16_t color=white) { text(edge-int(strlen(value))*6,y,value,color); }
void number(int x,int y,int value,uint16_t color=white,int size=1) {
  char buffer[16]; snprintf(buffer,sizeof(buffer),"%d",value); text(x,y,buffer,color,size);
}
void noteName(int note,char* output,size_t size) {
  static const char* names[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  snprintf(output,size,"%s%d",names[note%12],note/12-1);
}
void battery(const View& v) {
  const uint16_t color=v.full?green:(v.charging?green:(v.batteryPercent<20?coral:white));
  canvas.drawRect(140,3,17,8,color); canvas.fillRect(157,5,2,4,color);
  canvas.fillRect(142,5,v.batteryPercent*13/100,4,color);
}
void header(const View& v,const char* title) {
  text(3,3,title,cyan);
  if(v.meter.playing) canvas.fillTriangle(63,3,63,10,69,6,green);
  else canvas.fillRect(63,4,6,6,dim);
  if(v.meter.recording||v.meter.jam.capturing) canvas.fillCircle(76,6,3,coral);
  char buffer[12]; snprintf(buffer,sizeof(buffer),"%03d BPM",v.song.bpm); text(86,3,buffer,white);
  battery(v); canvas.drawFastHLine(0,14,160,line);
}
void footer(const View& v) {
  canvas.drawFastHLine(0,70,160,line);
  if(v.notice[0]) { text(3,72,v.notice,yellow); return; }
  if(v.page==JamPage) {
    const auto& jam=v.meter.jam;
    if (v.jamPage==JamChords) {
      char label[27];
      char name[21];
      if (jam.pendingChord>=0) {
        jamFormatChord(jam,jam.pendingChord,name,sizeof(name));
        snprintf(label,sizeof(label),"P%d  NEXT: %s",jam.progression+1,name);
      } else {
        jamFormatProgression(jam,name,sizeof(name));
        snprintf(label,sizeof(label),"P%d  %.19s",jam.progression+1,name);
      }
      text(3,72,label,jam.pendingChord>=0?yellow:dim);
      return;
    }
    if (v.jamPage==JamTracks) { text(3,72,"DOWN KD / UP SN / METER HH",dim); return; }
    if (v.jamPage==JamMenu) { text(3,72,"HOLD KNOB: BACK",dim); return; }
    const char* prompt=jam.capturing?"PLAY STRINGS / PRESS END"
                       : !v.touchReady?"TOUCH UNAVAILABLE"
                       : jam.waitingChord?"MOVE JOYSTICK: CHORD"
                       : jam.loopInput?"LOOP REC: 1/16"
                       : !jam.automatic?(jam.userMotif?"LOOP: PLAYING RESPONSE":"LOOP: PLAY A PHRASE")
                       : jam.userMotif?"MOTIF: PLAYER":"PRESS KNOB: CHORDS";
    text(3,72,prompt,jam.capturing||jam.loopInput?coral:dim);
    return;
  }
  const char* names[]={"LIVE","STEP","SOUND","MIX","SET"};
  const int xs[]={3,35,67,105,135};
  for(int i=0;i<5;++i) text(xs[i],72,names[i],v.page==i?white:dim);
}
void waveform(const View& v,int x,int y,int w,int h) {
  canvas.drawFastHLine(x,y+h/2,w,line);
  for(int i=1;i<32;++i) {
    const int a=constrain(int(v.meter.wave[i-1])*h/64,-h/2,h/2);
    const int b=constrain(int(v.meter.wave[i])*h/64,-h/2,h/2);
    canvas.drawLine(x+(i-1)*w/31,y+h/2-a,x+i*w/31,y+h/2-b,cyan);
  }
}
void live(const View& v) {
  header(v,"FM LIVE");
  text(3,19,presetNames[v.song.patches[1].preset],white);
  if(!v.touchReady) right(157,19,"TOUCH!",coral);
  else { char bank[8]; snprintf(bank,sizeof(bank),"P%d",v.song.bank+1); right(157,19,bank,dim); }
  const char* chord=v.chord<0?"E":chordNames[v.chord];
  text(3,35,chord,white,3); // The fixed-width Bdim label fits within the left 74 pixels.
  waveform(v,78,32,77,27);
  if(v.chord<0) text(3,62,"OPEN",dim);
  else text(3,62,"CHORD",dim);
  for(int t=0;t<trackCount;++t) {
    const int height=int(v.meter.peak*6)+1;
    canvas.fillRect(82+t*15,66-height,10,height,v.song.mute[t]?line:trackColors[t]);
  }
}
void center(int midpoint,int y,const char* label,uint16_t color) {
  text(midpoint-int(strlen(label))*3,y,label,color);
}
void jamChordsPage(const View& v) {
  header(v,"CHORDS");
  const auto& jam=v.meter.jam;
  const auto map=jamChordMap(jam);
  constexpr uint16_t colors[]={yellow,cyan,0xc39f,green};
  constexpr uint8_t x[]={80,133,133,133,80,27,27,27};
  constexpr uint8_t y[]={22,22,39,56,56,56,39,22};
  canvas.drawFastVLine(53,17,48,line);
  canvas.drawFastVLine(107,17,48,line);
  canvas.drawFastHLine(3,33,154,line);
  canvas.drawFastHLine(3,50,154,line);
  for (int d=0;d<jamDirectionCount;++d) {
    const int chord=map.chord[d];
    const auto color=colors[jamChordColor(chord)];
    const bool selected=!jam.waitingChord&&jam.chord==chord;
    const bool queued=jam.pendingChord==chord;
    if (selected||queued) canvas.drawRect(x[d]-25,y[d]-3,50,13,queued?white:color);
    char name[12];
    jamFormatChord(jam,chord,name,sizeof(name));
    center(x[d],y[d],name,color);
  }
  center(80,35,"NOW",dim);
  char current[12];
  jamFormatChord(jam,jam.chord,current,sizeof(current));
  center(80,43,jam.waitingChord?"DRUMS":current,white);
}
void jamTracksPage(const View& v) {
  header(v,"TRACKS");
  constexpr const char* labels[]={"KD","SN","HH","BS","CH","LD"};
  constexpr uint16_t colors[]={coral,yellow,green,0x3db8,0xc39f,cyan};
  const auto& jam=v.meter.jam;
  for (int i=0;i<4;++i) number(20+i*34,16,i+1,dim);
  for (int track=0;track<6;++track) {
    const int y=23+track*8;
    text(2,y,labels[track],colors[track]);
    const uint16_t pattern=track==5&&jam.loopInput?jam.loopCells:jam.trackCells[track];
    for (int step=0;step<16;++step) {
      const int x=20+step*8+(step/4)*2;
      const bool note=(pattern>>step)&1;
      const uint16_t color=note?(track==5&&jam.loopInput?coral:colors[track]):
                                (step%4==0?dim:line);
      const bool held=track==5 && ((jam.loopInput?jam.loopSustain:jam.replySustain)>>step&1);
      if(held&&!note) canvas.fillRect(x-3,y+2,9,2,jam.loopInput?coral:cyan);
      else canvas.fillRect(x,y,6,note?6:3,color);
    }
  }
  if (jam.active) {
    const int step=v.meter.step;
    const int x=19+step*8+(step/4)*2;
    canvas.drawRect(x,22,8,48,white);
  }
}
void jamSettingValue(const View& v,int field,char* buffer,size_t size) {
  const auto& jam=v.meter.jam;
  auto effective=jam;
  if(jam.pendingStyle>=0) effective.style=jam.pendingStyle;
  if(jam.pendingScale>=0) effective.scale=jam.pendingScale;
  if(jam.pendingKey>=0) effective.key=jam.pendingKey;
  if(jam.pendingProgression>=0) effective.progression=jam.pendingProgression;
  else if(jam.pendingStyle>=0||jam.pendingScale>=0) effective.progression=0;
  const int instrument=jam.pendingInstrument>=0?jam.pendingInstrument:jam.instrument;
  switch(field) {
    case JamModeField: snprintf(buffer,size,"%s",jam.automatic?"AUTO":"LOOP"); break;
    case JamStyleField: snprintf(buffer,size,"%s%s",jamStyleName(effective.style),jam.pendingStyle>=0?" >":""); break;
    case JamScaleField: {
      char name[18];
      jamFormatScale(effective,name,sizeof(name));
      snprintf(buffer,size,"%s%s%s",effective.scale==ScaleStyle?"STYLE / ":"",name,jam.pendingScale>=0?" >":"");
      break;
    }
    case JamKeyField:
      snprintf(buffer,size,"%s%s%s",effective.key==autoJamKey?"STYLE / ":"",
               jamKeyName(jamKey(effective)),jam.pendingKey>=0?" >":"");
      break;
    case JamProgressionField: {
      char name[21];
      jamFormatProgression(effective,name,sizeof(name));
      snprintf(buffer,size,"P%d %.19s",effective.progression+1,name);
      break;
    }
    case JamLeadField:
      snprintf(buffer,size,"%s%s",instrument==autoJamInstrument?"AUTO":presetNames[instrument],jam.pendingInstrument>=0?" >":"");
      break;
    case JamLeadLevelField: snprintf(buffer,size,"%d%%",v.meter.jamLeadLevel); break;
    default: snprintf(buffer,size,"%d BPM",v.song.bpm); break;
  }
}
void jamMenuPage(const View& v) {
  header(v,"SETTINGS");
  constexpr const char* names[]={"MODE","STYLE","SCALE","KEY","PROG","LEAD","LD LVL","TEMPO"};
  char value[26];
  if (v.jamEditing) {
    text(4,24,names[v.jamField],cyan);
    jamSettingValue(v,v.jamField,value,sizeof(value));
    text(4,40,value,white);
    text(4,57,"TURN / PRESS DONE",dim);
    return;
  }
  const int first=constrain(int(v.jamField)-1,0,int(jamFieldCount)-3);
  for (int row=0;row<3;++row) {
    const int field=first+row;
    const int y=20+row*12;
    const bool selected=field==v.jamField;
    if (selected) canvas.fillRect(2,y-2,156,11,line);
    text(5,y,names[field],selected?cyan:dim);
    jamSettingValue(v,field,value,sizeof(value));
    value[17]=0;
    right(155,y,value,selected?white:dim);
  }
  text(4,58,"TURN / PRESS EDIT",dim);
}
void jamPage(const View& v) {
  if (v.jamPage==JamChords) { jamChordsPage(v); return; }
  if (v.jamPage==JamTracks) { jamTracksPage(v); return; }
  if (v.jamPage==JamMenu) { jamMenuPage(v); return; }
  header(v,"AutoJam");
  const auto& jam=v.meter.jam;
  char name[18];
  jamFormatChord(jam,jam.chord,name,sizeof(name));
  text(3,20,jam.waitingChord?"DRUMS":name,white,2);
  char value[25];
  snprintf(value,sizeof(value),"VOL %d%%",v.song.master);
  right(157,23,value,yellow);
  if(jam.pendingChord>=0) {
    text(3,40,"NEXT",dim);
    jamFormatChord(jam,jam.pendingChord,name,sizeof(name));
    text(32,40,name,yellow);
  } else text(3,40,jamStyleName(jam.style),cyan);
  right(157,40,jam.automatic?"AUTO":"LOOP",dim);
  jamFormatScale(jam,name,sizeof(name));
  snprintf(value,sizeof(value),"P%d  %s",jam.progression+1,name);
  text(3,53,value,dim);
  if (jam.pendingKey>=0) right(157,53,"KEY >",yellow);
  else if (jam.pendingScale>=0) right(157,53,"SCALE >",yellow);
  const uint16_t cells=jam.loopInput?jam.loopCells:jam.replyCells;
  for (int i=0;i<16;++i) {
    const bool marked=(cells>>i)&1;
    const uint16_t color=jam.active&&v.meter.step==i?green:
      marked?(jam.loopInput?coral:cyan):i%4==0?dim:line;
    const bool held=((jam.loopInput?jam.loopSustain:jam.replySustain)>>i)&1;
    if(held&&!marked) canvas.fillRect(10*i,66,9,1,jam.loopInput?coral:cyan);
    else canvas.fillRect(3+i*10,65,6,marked?3:2,color);
  }
}
void grid(const View& v) {
  header(v,"GROOVE");
  const auto& steps=v.song.steps[v.song.bank];
  const char* initials[]={"K","S","H","B","L"};
  for(int t=0;t<trackCount;++t) {
    const int y=18+t*9;
    text(2,y,initials[t],t==v.track?white:dim);
    if(v.song.mute[t]) canvas.drawLine(1,y+3,9,y+3,coral);
    for(int s=0;s<16;++s) {
      const int x=15+s*9;
      canvas.fillRect(x,y,7,7,steps[t][s].enabled?(v.song.mute[t]?dim:trackColors[t]):line);
      if(v.meter.playing&&s==v.meter.step) canvas.drawFastVLine(x+3,y,7,white);
      if(t==v.track&&s==v.step) canvas.drawRect(x-1,y-1,9,9,white);
    }
  }
  char name[8],buffer[32]; const Step& step=steps[v.track][v.step]; noteName(step.note,name,sizeof(name));
  snprintf(buffer,sizeof(buffer),"P%d %s %02d %s V%03d",v.song.bank+1,trackNames[v.track],v.step+1,v.track>=Bass?name:"--",step.velocity);
  text(2,63,buffer,dim);
}
void sound(const View& v) {
  header(v,"FM SOUND");
  const Patch& patch=v.song.patches[v.track-Bass];
  text(3,18,trackNames[v.track],trackColors[v.track]); right(157,18,presetNames[patch.preset],white);
  const int first=min(int(v.parameter),parameterCount-3);
  for(int row=0;row<3;++row) {
    const int p=first+row,y=32+row*12;
    const bool selected=p==v.parameter;
    if(selected) canvas.fillRect(0,y-2,2,11,cyan);
    text(5,y,parameterNames[p],selected?white:dim);
    char buffer[16];
    if(p==Ratio) snprintf(buffer,sizeof(buffer),"%.2fx",patch.values[p]/4.f);
    else if(p==Index) snprintf(buffer,sizeof(buffer),"%.2f",patch.values[p]/100.f);
    else if(p==Attack||p==Decay||p==Release) snprintf(buffer,sizeof(buffer),"%dms",patch.values[p]);
    else snprintf(buffer,sizeof(buffer),"%d%%",patch.values[p]);
    right(157,y,buffer,selected?cyan:dim);
  }
}
void mix(const View& v) {
  header(v,"MIXER");
  const char* labels[]={"KIK","SNR","HAT","BAS","FM"};
  for(int t=0;t<trackCount;++t) {
    const int x=7+t*31;
    number(x,18,v.song.levels[t],t==v.track?white:dim);
    canvas.fillRect(x+3,29,12,27,line);
    const int height=v.song.levels[t]*27/100;
    canvas.fillRect(x+3,56-height,12,height,v.song.mute[t]?dim:trackColors[t]);
    if(t==v.track) canvas.drawRect(x+1,27,16,31,white);
    if(v.song.mute[t]) canvas.drawLine(x,28,x+18,56,coral);
    text(x,62,labels[t],t==v.track?white:dim);
  }
}
void optionValue(const View& v,int option,char* buffer,size_t size) {
  const Song& s=v.song;
  switch(option) {
    case 0: snprintf(buffer,size,"%d",s.bpm); break;
    case 1: snprintf(buffer,size,"%d%%",s.swing); break;
    case 2: snprintf(buffer,size,"%d%%",s.master); break;
    case 3: snprintf(buffer,size,"%d%%",s.echo); break;
    case 4:
      if(v.meter.pendingBank<bankCount) snprintf(buffer,size,"%d>%d",s.bank+1,v.meter.pendingBank+1);
      else snprintf(buffer,size,"%d",s.bank+1); break;
    case 5: snprintf(buffer,size,"%s",v.meter.recording?"ON":"OFF"); break;
    case 6: snprintf(buffer,size,"%s",s.led?"ON":"OFF"); break;
    default: snprintf(buffer,size,"%s",v.confirm&&v.option==option?"CONFIRM":"GO"); break;
  }
}
void settings(const View& v) {
  header(v,"SETTINGS");
  const int first=min(int(v.option),optionCount-3);
  for(int row=0;row<3;++row) {
    const int option=first+row,y=23+row*15;
    const bool selected=option==v.option;
    if(selected) canvas.fillRect(0,y-3,2,13,cyan);
    text(5,y,optionNames[option],selected?white:dim);
    char buffer[20]; optionValue(v,option,buffer,sizeof(buffer)); right(157,y,buffer,selected?cyan:dim);
  }
}
void standby(const View& v,uint32_t now) {
  text(59,5,"AutoJam",cyan);
  const uint16_t color=v.full?green:white;
  canvas.drawRect(39,24,78,33,color); canvas.fillRect(117,34,4,13,color);
  const int solid=v.batteryPercent>=100?3:v.batteryPercent*3/100;
  const int frame=(now/1000)%4;
  for(int i=0;i<3;++i) {
    const bool on=v.full||i<solid||(v.charging&&i<solid+frame);
    if(on) canvas.fillRect(43+i*24,28,21,25,color);
  }
  text(v.charging?56:62,65,v.full?"FULL":(v.charging?"CHARGING":"STANDBY"),v.full?green:dim);
}
void boot(uint32_t now,const char* stage="STARTING") {
  text(17,7,"Auto",white,3);
  text(89,7,"Jam",cyan,3);

  // The first three notes are the call. The answer shares its opening and rises.
  constexpr uint8_t x[]={12,33,46,77,98,111,125};
  constexpr uint8_t y[]={46,40,43,46,40,36,33};
  constexpr uint8_t width[]={17,9,15,17,9,10,23};
  for(int row=0;row<4;++row) canvas.drawFastHLine(12,35+row*5,136,line);
  canvas.drawFastVLine(68,33,20,dim);
  for(int i=0;i<7;++i) {
    const uint16_t color=i<3?coral:(now>=uint32_t(i-2)*180?cyan:dim);
    canvas.fillRect(x[i],y[i],width[i],3,color);
  }

  text(12,60,stage,white);
  canvas.fillRect(12,72,136,2,line);
  canvas.fillRect(12,72,int(min(now,uint32_t(5000))*136/5000),2,cyan);
}
#ifndef GROOVE_HOST_TEST
void flush() {
  const uint16_t* pixels=canvas.getBuffer();
  // Compare eight-row strips and submit only changed horizontal ranges.
  for(int y=0;y<80;y+=8) {
    int left=160,rightEdge=-1;
    for(int row=y;row<y+8;++row) for(int x=0;x<160;++x)
      if(firstFrame||pixels[row*160+x]!=previous[row*160+x]) { left=min(left,x); rightEdge=max(rightEdge,x); }
    if(rightEdge<left) continue;
    const int width=rightEdge-left+1;
    for(int row=0;row<8;++row) memcpy(strip+row*width,pixels+(y+row)*160+left,width*2);
    panel.startWrite(); panel.setAddrWindow(left,y,width,8); panel.writePixels(strip,width*8); panel.endWrite();
    for(int row=0;row<8;++row) memcpy(previous+(y+row)*160+left,pixels+(y+row)*160+left,width*2);
    taskYIELD();
  }
  firstFrame=false;
}
void displayTask(void*) {
  pinMode(hw::tftBacklight,OUTPUT); digitalWrite(hw::tftBacklight,LOW);
  SPI.begin(hw::tftClock,-1,hw::tftMosi,hw::tftCs);
  panel.initR(INITR_MINI160x80); panel.offset(); panel.setRotation(3); panel.setSPISpeed(40000000);
  const uint8_t madctl=ST77XX_MADCTL_MX|ST77XX_MADCTL_MV|ST7735_MADCTL_BGR;
  panel.sendCommand(ST77XX_MADCTL,&madctl,1); panel.invertDisplay(false); panel.fillScreen(bg);
  digitalWrite(hw::tftBacklight,HIGH);
  View state;
  const uint32_t started=millis();
  for(;;) {
    if(runtimeView(state)) {
      canvas.fillScreen(bg);
      if(state.booting||(state.active&&!state.audioReady&&!state.audioFault)) {
        const char* stage="STARTING";
        if(state.startup==TouchInit) stage="TOUCH INIT";
        if(state.startup==AudioInit) stage="AUDIO INIT";
        if(state.inputStalled) stage="INPUT WAIT";
        boot(millis()-started,stage);
      }
      else if(!state.active) standby(state,millis());
      else {
        switch(state.page) {
          case JamPage: jamPage(state); break;
          case Live: live(state); break;
          case Grid: grid(state); break;
          case Sound: sound(state); break;
          case Mix: mix(state); break;
          case Setup: settings(state); break;
        }
        if(state.audioFault) text(3,72,"AUDIO INIT ERROR",coral);
        else if(state.inputStalled) text(3,72,"INPUT WAIT",coral);
        else footer(state);
      }
      flush();
    }
    vTaskDelay(pdMS_TO_TICKS(state.active&&state.page!=Live?40:66));
  }
}
#endif
}
#ifdef GROOVE_HOST_TEST
GROOVE_EXPORT const uint16_t* fm_preview(int page,int chord,int playing) {
  View state{}; defaults(state.song); state.track=Lead; state.page=Page(page);
  state.touchReady=true; state.batteryPercent=73; state.chord=chord;
  state.meter.playing=playing; state.meter.step=5;
  for(int i=0;i<32;++i) state.meter.wave[i]=int8_t(daisysp::Oscillator::Sine(i/16.f)*15.f);
  canvas.fillScreen(bg);
  switch(page) {
    case Live: live(state); break;
    case Grid: state.step=5; grid(state); break;
    case Sound: state.parameter=Release; sound(state); break;
    case Mix: mix(state); break;
    case Setup: state.option=4; settings(state); break;
    case 5: state.charging=true; standby(state,2000); return canvas.getBuffer();
    case 7: boot(5000,"TOUCH INIT"); return canvas.getBuffer();
    case 8: boot(5000,"AUDIO INIT"); return canvas.getBuffer();
    case 9: boot(5000,"INPUT WAIT"); return canvas.getBuffer();
    case 10: case 11: case 12: case 13: case 14: case 15: case 16: {
      Jam demo;
      demo.setStyle(Pop);
      if(page==11 || page==13) demo.setKey(7);
      if(page==10 || page==11) demo.setMode(false);
      demo.selectChord(CMaj);
      demo.start();
      demo.beginBar();
      if(page==10 || page==11) demo.selectChord(AMin);
      if(page==14 || page==15) {
        demo.toggleCapture();
        if(page==15) {
          demo.capture(64,90,0,4000,10); demo.releaseCapture(10,12000);
          demo.capture(67,90,32000,4000,11); demo.releaseCapture(11,36000);
          demo.capture(72,90,40000,4000,12); demo.releaseCapture(12,48000);
          demo.toggleCapture(); demo.beginBar();
        }
      }
      state.page=JamPage; state.jamPage=page>=14?JamPerformance:JamPageView(page-10);
      state.jamField=JamKeyField;
      state.meter.jam=demo.status();
      if(page==10) state.meter.jam.replyCells=0x1114;
      jamPage(state); break;
    }
    default: boot(2500); return canvas.getBuffer();
  }
  footer(state); return canvas.getBuffer();
}
#else
void displayBegin() {
  if(!canvas.getBuffer()) { Serial.println("[FM] Display buffer allocation failed"); return; }
  if(xTaskCreatePinnedToCore(displayTask,"fm_display",4096,nullptr,1,nullptr,1)!=pdPASS) Serial.println("[FM] Display task creation failed");
}
#endif
}
