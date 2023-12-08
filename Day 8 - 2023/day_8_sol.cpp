#include <print>
#include <ranges>
#include <string>
#include <fstream>
#include <numeric>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

using namespace std::string_view_literals;

static constexpr std::string filename{"day_8_data.txt"};

int main(){
    // Open the files and extract the first line
    std::ifstream text_file(filename);
    std::string instructions;
    std::getline(text_file, instructions);

    // Skip over empty line
    text_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Load the nodes from the rest of the input into a map
    std::unordered_map<std::string, std::pair<std::string, std::string>> desert_map;
    for (std::string line; std::getline(text_file, line);){
        std::string label = line.substr(0, 3);
        std::string dest1 = line.substr(7, 3);
        std::string dest2 = line.substr(12, 3);

        desert_map.insert({label, {dest1, dest2}});
    }

    // Make walk function so it can be reused for part 2
    auto walkToEnd = [&desert_map, &instructions](std::string current, std::function<bool(const std::string&)> termination) -> size_t {
        size_t num_steps = 0;
        for (char instruction : std::views::repeat(instructions) | std::views::join){
            if (termination(current)) break;
            auto [left, right] = desert_map.at(current);
            current = (instruction == 'L' ? left : right);
            num_steps++;
        }
        return num_steps;
    }; 
    size_t num_steps = walkToEnd("AAA", [](const std::string& s){return s == "ZZZ";});
    std::println("Took {} steps", num_steps);

    // Problem 2 solution
    auto endsWithA = [](const std::string& s){return s.ends_with("A");};
    auto endsWithZ = [](const std::string& s){return s.ends_with("Z");};

    size_t walk_steps = 1;
    for (auto start_point : desert_map | std::views::keys | std::views::filter(endsWithA)){
        walk_steps = std::lcm(walk_steps, walkToEnd(start_point, endsWithZ));
    }
    std::println("Minimum steps is {}", walk_steps);

    return 0;
}
