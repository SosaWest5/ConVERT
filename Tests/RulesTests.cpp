#include "../Source/ConversionRules.h"
#include <iostream>
#include <stdexcept>
void check(bool good){if(!good)throw std::runtime_error("URL/quality test failed");}
int main(){try{const std::string wanted="https://www.youtube.com/watch?v=DL6y845KZo4";
 for(const auto& url:{"https://www.youtube.com/watch?v=DL6y845KZo4&list=test","https://youtu.be/DL6y845KZo4?t=15","https://music.youtube.com/watch?foo=1&v=DL6y845KZo4","https://www.youtube.com/shorts/DL6y845KZo4","https://www.youtube.com/live/DL6y845KZo4","https://www.youtube.com/embed/DL6y845KZo4"})check(convert::canonicalYouTubeURL(url)==wanted);
 for(const auto& url:{"http://youtube.com/watch?v=DL6y845KZo4","https://youtube.com.evil.test/watch?v=DL6y845KZo4","https://youtube.com@evil.test/watch?v=DL6y845KZo4","https://youtube.com/playlist?list=123","--exec dangerous","file:///tmp/a","https://youtu.be/short","https://youtube.com/watch?v=DL6y845KZo4XX","https://youtube.com/watch?v=DL6y845KZo4\n--exec","https://youtube.com:443/watch?v=DL6y845KZo4"})check(convert::canonicalYouTubeURL(url).empty());
 check(convert::safeBitrate(192)==192);check(convert::safeBitrate(1)==320);std::cout<<"URL normalization, domain isolation, injection rejection, playlist rejection, and bitrate tests passed.\n";return 0;}catch(const std::exception& e){std::cerr<<e.what();return 1;}}
