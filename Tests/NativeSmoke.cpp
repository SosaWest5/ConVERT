#include "../Source/Plugin.cpp"
#include <iostream>
#include <stdexcept>
void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(int argc,char** argv){try{ConvertProcessor p;AudioBuffer<float> audio(2,128);MidiBuffer midi;for(int c=0;c<2;c++)for(int n=0;n<128;n++)audio.setSample(c,n,float(n)*.0001f);p.prepareToPlay(44100,128);p.processBlock(audio,midi);for(int c=0;c<2;c++)for(int n=0;n<128;n++)require(audio.getSample(c,n)==float(n)*.0001f,"Pass through changed samples");
 p.savedURL="https://youtu.be/DL6y845KZo4";p.bitrate=192;p.output=File::getSpecialLocation(File::tempDirectory).getChildFile("ConVERT test output");MemoryBlock state;p.getStateInformation(state);ConvertProcessor q;q.setStateInformation(state.getData(),int(state.getSize()));require(q.savedURL==p.savedURL&&q.bitrate==192&&q.output==p.output,"Host state restore");require(!q.engine.startConversion("https://evil.test",q.output,192),"Bad URL accepted");require(!q.engine.snapshot().busy,"Invalid job was launched");if(argc>1){auto root=File::getSpecialLocation(File::tempDirectory).getNonexistentChildFile("ConVERT integration","");auto tools=root.getChildFile("tools"),dest=root.getChildFile("output with spaces");tools.createDirectory();dest.createDirectory();
 #if JUCE_WINDOWS
 const String suffix=".exe";_putenv_s("CONVERT_TOOLS_DIR",tools.getFullPathName().toRawUTF8());
 #else
 const String suffix="";setenv("CONVERT_TOOLS_DIR",tools.getFullPathName().toRawUTF8(),1);
 #endif
 for(auto name:{"yt-dlp","ffmpeg","ffprobe","deno"}){auto f=tools.getChildFile(String(name)+suffix);require(File(argv[1]).copyFileTo(f),"Mock tool copy");f.setExecutePermission(true);}
 auto wait=[&]{for(int n=0;n<150&&q.engine.snapshot().busy;++n)Thread::sleep(50);require(!q.engine.snapshot().busy,"Worker did not finish");Thread::sleep(100);};
 require(q.engine.startConversion("https://youtu.be/DL6y845KZo4",dest,192),"Success job start");wait();auto success=q.engine.snapshot();require(success.progress==1&&success.result.existsAsFile(),"Completed file/progress parsing");auto args=dest.getChildFile("test-arguments.txt").loadFileAsString();require(args.contains("--no-playlist")&&args.contains("192K")&&args.contains("--ignore-config")&&args.contains("--no-plugin-dirs"),"Engine invocation flags");
 require(q.engine.startConversion("https://youtu.be/FAIL0000000",dest,320),"Failure job start");wait();require(q.engine.snapshot().result==File()&&q.engine.snapshot().status.contains("Controlled test failure"),"Failure handling");
 require(q.engine.startConversion("https://youtu.be/HANG0000000",dest,128),"Cancel job start");Thread::sleep(150);q.engine.cancel();wait();require(q.engine.snapshot().status.startsWith("Cancelled")&&q.engine.snapshot().result==File(),"Cancellation handling");root.deleteRecursively();}
 std::cout<<"Native audio pass-through, host state, job validation, command arguments, output parsing, failure, and cancellation passed.\n";return 0;}catch(const std::exception& e){std::cerr<<e.what();return 1;}}
