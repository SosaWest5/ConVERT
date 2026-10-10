#pragma once
#include <juce_core/juce_core.h>
#if JUCE_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cwchar>
// Use a job object so cancellation/plugin unloading also stops FFmpeg children.
class ProcessRunner {
 HANDLE proc=nullptr,readPipe=nullptr,job=nullptr;juce::CriticalSection guard;
 static juce::String quote(const juce::String& arg){juce::String result="\"";int slashes=0;for(auto c:arg){if(c=='\\'){++slashes;continue;}if(c=='\"'){result+=juce::String::repeatedString("\\",slashes*2+1);result+='\"';}else{result+=juce::String::repeatedString("\\",slashes);result+=juce::String::charToString(c);}slashes=0;}result+=juce::String::repeatedString("\\",slashes*2);return result+"\"";}
public:
 ~ProcessRunner(){kill();if(proc)CloseHandle(proc);if(readPipe)CloseHandle(readPipe);if(job)CloseHandle(job);}
 bool start(const juce::StringArray& args){const juce::ScopedLock lock(guard);SECURITY_ATTRIBUTES sa{sizeof(sa),nullptr,TRUE};HANDLE writePipe=nullptr;if(!CreatePipe(&readPipe,&writePipe,&sa,0))return false;SetHandleInformation(readPipe,HANDLE_FLAG_INHERIT,0);job=CreateJobObjectW(nullptr,nullptr);JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};info.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;if(!job||!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&info,sizeof(info))){CloseHandle(writePipe);return false;}
  juce::String command;for(const auto& arg:args)command+=quote(arg)+" ";const wchar_t* wide=command.toWideCharPointer();std::vector<wchar_t> buffer(wide,wide+std::wcslen(wide)+1);buffer.push_back(0);STARTUPINFOW si{};si.cb=sizeof(si);si.dwFlags=STARTF_USESTDHANDLES;si.hStdOutput=writePipe;si.hStdError=writePipe;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);PROCESS_INFORMATION pi{};
  const bool ok=CreateProcessW(args[0].toWideCharPointer(),buffer.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT,nullptr,nullptr,&si,&pi)!=FALSE;CloseHandle(writePipe);if(!ok)return false;proc=pi.hProcess;if(!AssignProcessToJobObject(job,proc)){TerminateProcess(proc,1);CloseHandle(pi.hThread);return false;}ResumeThread(pi.hThread);CloseHandle(pi.hThread);return true;
 }
 int readProcessOutput(void* data,int count){DWORD available=0,read=0;if(!readPipe||!PeekNamedPipe(readPipe,nullptr,0,nullptr,&available,nullptr)||available==0)return 0;return ReadFile(readPipe,data,juce::jmin(DWORD(count),available),&read,nullptr)?int(read):0;}
 bool isRunning(){return proc&&WaitForSingleObject(proc,0)==WAIT_TIMEOUT;}
 uint32_t getExitCode(){DWORD code=1;if(proc)GetExitCodeProcess(proc,&code);return code;}
 void kill(){const juce::ScopedLock lock(guard);if(job)TerminateJobObject(job,1);else if(proc)TerminateProcess(proc,1);}
};
#else
using ProcessRunner=juce::ChildProcess;
#endif
