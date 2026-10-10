#pragma once
#include <string>
#include <regex>
#include <algorithm>
#include <cctype>
namespace convert {
inline std::string canonicalYouTubeURL(const std::string& input) {
 if(input.size()>2048||input.find_first_of("\r\n\t\"\\")!=std::string::npos)return {};
 std::smatch m;static const std::regex uri(R"(^https://([^/\s?#]+)(/[^\s]*)?$)",std::regex::icase);
 if(!std::regex_match(input,m,uri))return {};auto host=m[1].str();std::transform(host.begin(),host.end(),host.begin(),[](unsigned char c){return char(std::tolower(c));});
 const auto path=m[2].str();std::smatch v;const std::string id="([A-Za-z0-9_-]{11})";
 if(host=="youtu.be"||host=="www.youtu.be"){if(!std::regex_search(path,v,std::regex("^/"+id+"(?:[?#].*)?$")))return {};}
 else if(host=="youtube.com"||host=="www.youtube.com"||host=="m.youtube.com"||host=="music.youtube.com"){
  if(path.rfind("/watch?",0)==0){if(!std::regex_search(path,v,std::regex("[?&]v="+id+"(?:[&#]|$)")))return {};}
  else if(!std::regex_search(path,v,std::regex("^/(?:shorts|live|embed)/"+id+"(?:[?#].*)?$")))return {};
 }else return {};
 return "https://www.youtube.com/watch?v="+v[1].str();
}
inline int safeBitrate(int n){return n==128||n==192||n==256||n==320?n:320;}
}
