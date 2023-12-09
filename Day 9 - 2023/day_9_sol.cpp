#include <print>
#include <ranges>
#include <vector>
#include <chrono>
#include <algorithm>

#include "load_input.hpp"

using data_t = int64_t;

std::pair<data_t, data_t> getNextInSequence(const std::vector<data_t>& seq){
    // If all are zero, return 0
    if (seq.empty() || std::ranges::all_of(seq, [](int i){return i == 0;})) return {0, 0};    

    // Recursively call for the next
    std::vector<data_t> next_level = seq 
        | std::views::adjacent_transform<2>([](data_t a, data_t b){return b - a;}) 
        | std::ranges::to<std::vector<data_t>>();
    auto [next_first, next_last] = getNextInSequence(next_level);

    return {seq.front() - next_first, seq.back() + next_last};
}

int main(){
    auto start = std::chrono::steady_clock::now();
    std::string input_data = loadInput("day_9_data.txt");
    
    // Convert the input to a range of vectors of ints
    auto int_ranges = input_data 
        | std::views::split('\n')
        | std::views::transform([](auto line){
            return line 
                | std::views::split(' ')
                | std::views::transform([](auto chunk){return std::atoll(chunk.data());})
                | std::ranges::to<std::vector<data_t>>();
        });

    // Might as well solve both parts at the same time
    int64_t front_sum = 0;
    int64_t back_sum = 0;
    for (const std::vector<data_t>& line_of_ints : int_ranges){
        auto [front_val, back_val] = getNextInSequence(line_of_ints);
        front_sum += front_val;
        back_sum += back_val;
    }

    auto stop = std::chrono::steady_clock::now();
    std::println("Front sum is {} and Back sum is {}", front_sum, back_sum);
    std::println("Calculations took {} us", std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count());
}