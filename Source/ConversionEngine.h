#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "ConversionRules.h"
#include "ProcessRunner.h"
class ConversionEngine : private juce::Thread {
public:
 struct Snapshot {juce::String status="Paste a YouTube video link to begin.", log;juce::File result;double progress=0;bool busy=false;};
 ConversionEngine():Thread("ConVERT background conversion"){}
 ~ConversionEngine() override{cancel();stopThread(-1);}
 Snapshot snapshot() const{const juce::ScopedLock guard(lock);return state;}
 static juce::File toolsFolder(){
  const auto overridePath=juce::SystemStats::getEnvironmentVariable("CONVERT_TOOLS_DIR",{});
  if(overridePath.isNotEmpty())return juce::File(overridePath);
 #if JUCE_WINDOWS
  const auto root=juce::SystemStats::getEnvironmentVariable("ProgramW6432",{});
  auto system=root.isNotEmpty()?juce::File(root):juce::File::getSpecialLocation(juce::File::globalApplicationsDirectory);
  auto folder=system.getChildFile("ConVERT/tools");if(folder.getChildFile("yt-dlp.exe").existsAsFile())return folder;
 #endif
  return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("ConVERT/tools");
 }
 bool startConversion(juce::String url,juce::File destination,int rate){
  if(isThreadRunning())return false;
  auto normalized=convert::canonicalYouTubeURL(url.trim().toStdString());if(normalized.empty()){setStatus("Use a YouTube video link (watch, shorts, or youtu.be).");return false;}
  if(!destination.createDirectory().wasOk()){setStatus("Cannot write to this output folder. Choose another folder.");return false;}
  auto probe=destination.getNonexistentChildFile(".convert-write-test","");if(!probe.replaceWithText("")){setStatus("This folder is not writable.");return false;}probe.deleteFile();
  auto tools=toolsFolder();
 #if JUCE_WINDOWS
  const juce::String suffix=".exe";
 #else
  const juce::String suffix="";
 #endif
  for(auto name:{"yt-dlp","ffmpeg","ffprobe","deno"})if(!tools.getChildFile(juce::String(name)+suffix).existsAsFile()){setStatus("Conversion tools are missing. Install the complete ConVERT package.");return false;}
  source=normalized;output=destination;bitrate=convert::safeBitrate(rate);cancelled=false;
  {const juce::ScopedLock guard(lock);state={};state.status="Connecting to YouTube…";state.busy=true;}
  startThread();return true;
 }
 void cancel(){cancelled=true;signalThreadShouldExit();std::shared_ptr<ProcessRunner> p;{const juce::ScopedLock guard(lock);p=process;}if(p)p->kill();}
private:
 mutable juce::CriticalSection lock;Snapshot state;std::shared_ptr<ProcessRunner> process;std::atomic<bool> cancelled{false};juce::String source;juce::File output;int bitrate=320;
 void setStatus(juce::String text){const juce::ScopedLock guard(lock);state.status=text;}
 void line(juce::String text){text=text.trim();const juce::ScopedLock guard(lock);if(text.startsWith("__CONVERT_FILE__")){auto parsed=juce::JSON::parse("["+text.substring(16)+"]")[0];if(parsed.isString())state.result=juce::File(parsed.toString());return;}if(text.startsWith("__CONVERT_PROGRESS__")){state.progress=juce::jlimit(0.,.99,text.substring(20).getDoubleValue()/100.);state.status="Downloading audio… "+juce::String(int(state.progress*100))+"%";return;}if(text.isNotEmpty()){state.log=(state.log+text+"\n").getLastCharacters(12000);if(text.contains("[ExtractAudio]"))state.status="Encoding "+juce::String(bitrate)+" kbps MP3…";if(text.startsWith("ERROR:"))state.status=text.substring(6).trim().substring(0,220);}}
 void run() override{
  const auto tools=toolsFolder();
 #if JUCE_WINDOWS
  const juce::String suffix=".exe";
 #else
  const juce::String suffix="";
 #endif
  // Argument arrays avoid invoking cmd.exe, PowerShell, or a shell with user text.
  juce::StringArray args{tools.getChildFile("yt-dlp"+suffix).getFullPathName(),"--ignore-config","--no-plugin-dirs","--no-js-runtimes","--js-runtimes","deno:"+tools.getChildFile("deno"+suffix).getFullPathName(),"--encoding","utf-8","--no-playlist","--no-simulate","--no-overwrites","--socket-timeout","15","--retries","3","--fragment-retries","3","--match-filters","!is_live","--max-filesize","1G","--windows-filenames","--newline","--no-colors","--progress","--progress-delta","0.5","--progress-template","download:__CONVERT_PROGRESS__%(progress._percent_str)s","--print","after_move:__CONVERT_FILE__%(filepath)j","-f","bestaudio/best","--extract-audio","--audio-format","mp3","--audio-quality",juce::String(bitrate)+"K","--embed-metadata","--ffmpeg-location",tools.getFullPathName(),"-P",output.getFullPathName(),"-o","%(title).140B [%(id)s].%(ext)s","--",source};
  auto child=std::make_shared<ProcessRunner>();{const juce::ScopedLock guard(lock);process=child;}
  if(cancelled||threadShouldExit()){finish(false);return;}
  if(!child->start(args)){setStatus("Could not start the conversion engine.");finish(false);return;}
  if(cancelled)child->kill();
  char buf[4096];juce::MemoryBlock pending;const auto started=juce::Time::getMillisecondCounterHiRes();
  for(;;){if(cancelled||threadShouldExit()){child->kill();break;}if(juce::Time::getMillisecondCounterHiRes()-started>30*60*1000){child->kill();setStatus("Conversion timed out after 30 minutes.");break;}
   const auto count=child->readProcessOutput(buf,sizeof(buf));if(count>0){pending.append(buf,size_t(count));auto* b=static_cast<const char*>(pending.getData());size_t start=0;for(size_t n=0;n<pending.getSize();++n)if(b[n]=='\n'){line(juce::String::fromUTF8(b+start,int(n-start)));start=n+1;}if(start)pending.removeSection(0,start);if(pending.getSize()>65536)pending.reset();}
   else if(!child->isRunning())break;else wait(40);
  }
  if(pending.getSize())line(juce::String::fromUTF8(static_cast<const char*>(pending.getData()),int(pending.getSize())));
  const auto exitCode=child->getExitCode();finish(exitCode==0);
 }
 void finish(bool success){const juce::ScopedLock guard(lock);state.busy=false;process.reset();if(cancelled){state.status="Cancelled. Partial downloads stay in the output folder for retry.";state.result=juce::File();return;}if(success&&state.result.existsAsFile()&&state.result.hasFileExtension("mp3")&&state.result.getSize()>0){state.progress=1;state.status="MP3 ready. Drag it into the FL Studio playlist.";}else{state.result=juce::File();if(state.status.startsWith("Connecting")||state.status.startsWith("Downloading")||state.status.startsWith("Encoding"))state.status="Conversion failed. Open Details to see the engine message.";}}
};
