#include <print>
#include <ranges>
#include <string>
#include <fstream>
#include <unordered_map>

using namespace std::string_view_literals;

static constexpr std::string filename{"day_2_data.txt"};

int main(){
    std::ifstream text_file(filename);
    
    uint32_t power    = 0;
    uint32_t passcode = 0;

    for (std::string line; std::getline(text_file, line);){

        // Remove the "Game " prefix
        std::string_view line_view(line);
        line_view.remove_prefix(5);

        // Find the game index
        size_t colon_idx = line_view.find(':');
        const std::string_view game_idx_view = line_view.substr(0, colon_idx);
        const int game_idx = std::atoi(std::string(game_idx_view).c_str());
        line_view.remove_prefix(colon_idx+1);

        // Extract the data
        std::unordered_map<std::string, int> max_vals{{"red", 0}, {"blue", 0}, {"green", 0}};
        bool possible = true;

        // Split into each round based on semicolons
        for (auto round : line_view | std::views::split(";"sv)){
            int round_total = 0;

            // Each instance in a round is separated by commas
            for (auto instance : round | std::views::split(","sv)){
                auto sections = instance | std::views::split(" "sv);

                // Account for the space at the beginning of the instance
                auto it = sections.begin();
                std::string_view count_view{*++it};
                std::string_view label_view{*++it};
                int count = std::atoi(std::string(count_view).c_str());

                // Determine the total ball count for this instance
                round_total += count;
                possible = possible && round_total <= (12 + 13 + 14);

                // Update the max values seen
                std::string label{label_view};
                max_vals.at(label) = std::max(max_vals.at(label), count);
            }
            // Keep track of whether or not the game is possible
            possible = possible && !(max_vals["red"] > 12 || max_vals["blue"] > 14 || max_vals["green"] > 13);
        }

        // Increment the power and passcode
        power += max_vals["red"]*max_vals["green"]*max_vals["blue"];
        passcode += game_idx * possible;
    }

    std::println("The passcode is {} and the power is {}", passcode, power);
    return 0;
}