#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
static std::string json(const std::string& s){std::string r="\"";for(char c:s){if(c=='\\'||c=='"')r+='\\';r+=c;}return r+'"';}
int main(int argc,char** argv){std::filesystem::path output;std::string url;for(int i=1;i<argc;i++){if(std::string(argv[i])=="-P"&&i+1<argc)output=argv[i+1];url=argv[i];}if(url.find("FAIL0000000")!=std::string::npos){std::cout<<"ERROR: Controlled test failure\n";return 1;}if(url.find("HANG0000000")!=std::string::npos){std::cout<<"__CONVERT_PROGRESS__ 1.0%\n"<<std::flush;std::this_thread::sleep_for(std::chrono::seconds(30));return 0;}
 if(output.empty())return 2;std::ofstream args(output/"test-arguments.txt");for(int i=1;i<argc;i++)args<<argv[i]<<'\n';args.close();auto file=output/"test 'track' fixture.mp3";std::ofstream data(file,std::ios::binary);data<<"ID3 fake fixture for process integration only";data.close();std::cout<<"__CONVERT_PROGRESS__ 50.0%\n"<<"__CONVERT_FILE__"<<json(file.string())<<"\n";return 0;}
