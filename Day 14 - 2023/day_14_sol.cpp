#include <print>
#include <string>
#include <ranges>
#include <chrono>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "load_input.hpp"

namespace ranges = std::ranges;
namespace views  = std::ranges::views;

enum Direction{
    NORTH, 
    EAST, 
    WEST, 
    SOUTH
};

template<ranges::forward_range Range>
void tilt_impl(Range grid){
    for (auto row_or_col : grid){
        // Split the row or column into sections based on '#' characters
        auto chunks = row_or_col | views::split('#');

        for (auto chunk : chunks){
            // Count the number of 'O' characters and fill the first n slots with 'O' and the rest with '.'
            int num_rocks = ranges::count(chunk, 'O');
            ranges::fill(chunk | views::take(num_rocks), 'O');
            ranges::fill(chunk | views::drop(num_rocks), '.');
        }
    }
}

void tilt(std::vector<std::span<char>>& grid, Direction dir){
    // Transpose the grid for a column view
    int num_cols = grid[0].size();
    auto cols = views::iota(0, num_cols) | views::transform(
        [&](int i){
            return grid 
                | views::join
                | views::drop(i)
                | views::stride(num_cols);
        }
    );

    // Have to use an impl function because the type passed to it is different in every case
    switch (dir){
        default:
        case NORTH:
            tilt_impl(cols);
            return;
        
        case EAST:
            tilt_impl(grid | views::transform([](auto row){return row | views::reverse;}));
            return;

        case SOUTH:
            tilt_impl(cols | views::transform([](auto col){return col | views::reverse;}));
            return;
            
        case WEST:
            tilt_impl(grid | views::all);
            return;
    }
}

// Calculate the weight on the north beam
int evaluate(std::vector<std::span<char>>& grid){
    int total = 0;
    for (auto [idx, row] : grid | views::reverse | views::enumerate){
        total += ranges::count(row, 'O') * (idx+1);
    }
    return total;
}

int main(){
    std::string input_str = loadInput("day_14_data.txt");
    auto start_time = std::chrono::steady_clock::now();

    // Prepare the data
    auto rows = input_str | views::split('\n') | ranges::to<std::vector<std::span<char>>>();

    // Part 1
    std::string input_copy{input_str};
    tilt(rows, NORTH);
    size_t part1_total = evaluate(rows);
    input_str = input_copy;

    // Part 2
    const size_t num_cycles = 1'000'000'000;
    auto cycle = [&](){
        tilt(rows, NORTH);
        tilt(rows, WEST) ;
        tilt(rows, SOUTH);
        tilt(rows, EAST) ;
    };

    // Keep track of previous states to detect if we enter a cycle of states
    std::unordered_map<std::string, size_t> known_states;
    for (size_t i = 0; i < num_cycles; i++){
        
        // Check for a repeated state. If one is found, we have a cycle
        if (known_states.contains(input_str)){
            // Calculate the properties of the state cycle
            const size_t cycle_start_idx = known_states[input_str];
            const size_t cycle_frequency = i - cycle_start_idx;
            const size_t target_index    = (num_cycles - cycle_start_idx) % cycle_frequency;

            // Extract the state that matches with the final target state
            auto target_state = known_states 
                | views::drop(cycle_start_idx) 
                | views::drop_while([&](auto pair){return pair.second - cycle_start_idx != target_index;})
                | views::take(1);
            input_str = target_state.front().first;
            break;
        }

        // Record the current state and cycle again
        known_states.insert({input_str, i});
        cycle();
    }

    auto stop_time = std::chrono::steady_clock::now();
    std::println("Part 1 total is {}", part1_total);
    std::println("Part 2 total is {}", evaluate(rows));
    std::println("Calculations took {} milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count());
}