#include <fstream>
#include <string>
#include <sstream>

static inline std::string loadInput(const std::string& filename){
    std::ifstream text_file(filename);
    std::stringstream ss;
    ss << text_file.rdbuf();
    return ss.str();
}