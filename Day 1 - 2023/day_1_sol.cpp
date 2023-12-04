#include <print>
#include <ranges>
#include <string>
#include <format>
#include <fstream>
#include <iostream>

static constexpr std::string filename{"day_1_data.txt"}; 
static constexpr std::array<std::string, 10> digit_names{
    "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
};
static constexpr std::array<char, 10> digits{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

// Digit filter
bool isDigit(char c){
    return c >= '0' && c <= '9';
};

uint32_t problem1(std::ifstream& file){
    

    uint32_t sum = 0;
    for (std::string line; std::getline(file, line);){
        auto digits = line | std::views::filter(isDigit);
        if (digits.empty()) continue;
        char val[2] = {digits.front(), digits.back()};
        sum += std::atoi(val);
    }
    return sum;
}

uint32_t problem2(std::ifstream& file){
    uint32_t sum = 0;

    for (std::string line; std::getline(file, line);){

        // Transform all words to digits
        for(std::span line_span(line); !line_span.empty(); line_span = line_span.subspan(1)){
            std::string_view line_view(line_span);
            for (auto [digit, name] : std::views::zip(digits, digit_names)){
                if (line_view.starts_with(name))
                    line_span[0] = digit;
            }
        }

        // Apply the problem 1 solution
        auto digits = line | std::views::filter(isDigit);
        if (digits.empty()) continue;
        char val[2] = {digits.front(), digits.back()};
        sum += std::atoi(val);
    }

    return sum;
}

int main(){
    std::ifstream text_file(filename);

    // Problem 1
    uint32_t sum1 = problem1(text_file);
    std::println("The sum is {}", sum1);

    // Reset the file
    text_file.clear();
    text_file.seekg(std::ios_base::beg);

    // Problem 2
    std::println("Starting problem 2");
    uint32_t sum2 = problem2(text_file);
    std::println("The sum is {}", sum2);
}