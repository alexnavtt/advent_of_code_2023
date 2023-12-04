#include <print>
#include <string>
#include <fstream>
#include <ranges>
#include <iostream>
#include <sstream>
#include <numeric>

static size_t str_pos{};

// Digit filter
bool isDigit(char c){
    return c >= '0' && c <= '9';
};

// Given that an index is a digit, collect all digits on either side into 
// a string view and convert them to the corresponding numeric value
int expandDigits(std::ptrdiff_t index, std::string_view str){
    // Search to the left
    size_t idx1 = str.substr(0, index).find_last_not_of("0123456789");
    if (idx1 == std::string_view::npos) idx1 = 0;
    else idx1++;

    // Search to the right
    size_t idx2 = str.substr(index).find_first_not_of("0123456789");
    if (idx2 == std::string_view::npos) idx2 = index+1;
    else idx2 += index;

    std::string_view digits_view = str.substr(idx1, idx2 - idx1);
    return std::atoi(digits_view.data());
}

// Given an index in a string, get either 1 or 2 numbers from that index or to its sides
std::pair<int, int> getNumsFromString(std::ptrdiff_t index, std::string_view str){
    std::pair<int, int> ret_val{0, 0};
    size_t left_idx  = std::max(index-1, std::ptrdiff_t(0));
    size_t right_idx = std::min(index+1, std::ptrdiff_t(str.size()-1));

    if (isDigit(str[index])){
        ret_val.first = expandDigits(index, str);
    }else{
        if (isDigit(str[left_idx ])) ret_val.first  = expandDigits(left_idx, str);
        if (isDigit(str[right_idx])) ret_val.second = expandDigits(right_idx, str);
    }
    return ret_val;
}

int main(){
    uint32_t total_sum = 0;
    uint64_t gear_ratio = 0;

    // Load the data into memory
    std::ifstream text_file("day_3_data.txt");

    // Append a row of periods to the beginning and end of the data
    std::string line_of_periods;
    if (std::getline(text_file, line_of_periods) ){
        std::memset(line_of_periods.data(), '.', line_of_periods.size());
        text_file.clear();
        text_file.seekg(std::ios::beg);
    }
    const size_t line_len = line_of_periods.size();

    std::stringstream data;
    data << line_of_periods << '\n';
    data << text_file.rdbuf();
    data << '\n' << line_of_periods;
    std::string data_str = data.str();

    // Iterate through the lines three at a time
    size_t line_idx = 0;
    for (auto lines : std::string_view(data_str) | std::views::split('\n') | std::views::slide(3)){
        auto line_iterator = lines.begin();
        std::string_view line1(*line_iterator++);
        std::string_view line2(*line_iterator++);
        std::string_view line3(*line_iterator++);

        // Look for special characters in the second line
        auto special_chars_indices = line2 
            | std::views::enumerate
            | std::views::filter([](auto val){char c = std::get<1>(val); return !isDigit(c) && (c != '.');})
            | std::views::keys;

        // For each special character, examine the surrounding areas
        for (auto idx : special_chars_indices){
            // Get the numbers from all three lines
            auto [line1_1, line1_2] = getNumsFromString(idx, line1);
            auto [line2_1, line2_2] = getNumsFromString(idx, line2);
            auto [line3_1, line3_2] = getNumsFromString(idx, line3);

            std::array<int, 6> nums{line1_1, line1_2, line2_1, line2_2, line3_1, line3_2};
            total_sum += std::reduce(nums.begin(), nums.end());

            // Check if this was a gear character
            if (line2[idx] != '*') continue;

            // If it had exactly 2 characters, add on their product to the gear ratio
            auto vals = nums | std::views::filter([](int i){return i != 0;}); 
            if (std::ranges::distance(vals) == 2){
                gear_ratio += std::reduce(std::begin(vals), std::end(vals), 1, std::multiplies{});
            }
        }
        line_idx++;
    }
    std::println("The total sum is {}", total_sum);
    std::println("The total gear ratio is {}", gear_ratio);
    return 0;
}