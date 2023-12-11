#include <print>
#include <string>
#include <ranges>
#include <vector>
#include <chrono>
#include <algorithm>

#include "load_input.hpp"

struct Index{
    int x;
    int y;
};

size_t numGalaxies(auto range){
    return std::ranges::distance(range | std::views::filter([](char c){return c == '#';}));
}

int main(){
    std::string input_str = loadInput("day_11_data.txt");
    const auto start_time = std::chrono::steady_clock::now();

    // Get the size of each line
    const size_t line_length = input_str.find('\n');

    std::vector<Index> galaxy_indices;
    std::vector<size_t> empty_row_indices;
    std::vector<size_t> empty_col_indices;
    for (auto [y_idx, row] : input_str | std::views::split('\n') | std::views::enumerate){
        // Check if this row has no galaxies
        if (numGalaxies(row) == 0) empty_row_indices.push_back(y_idx);
        
        // Get the indices of all '#' characters
        auto galaxy_positions = row 
            | std::views::enumerate 
            | std::views::filter([](auto elem){return std::get<1>(elem) == '#';}) 
            | std::views::keys
            | std::views::transform([y_idx](auto x_idx){return Index{(int)x_idx, (int)y_idx};});
        
        // Append these to the existing list of indices
        galaxy_indices.insert(std::end(galaxy_indices), std::ranges::begin(galaxy_positions), std::ranges::end(galaxy_positions));
    }
    for (auto col_idx : std::views::iota(size_t(0), line_length)){
        // Construct a column view
        auto col = input_str | std::views::drop(col_idx) | std::views::stride(line_length+1);
        // Check to see if this column has no galaxies
        if (numGalaxies(col) == 0) empty_col_indices.push_back(col_idx);
    }

    size_t total_dist = 0;
    for (auto [gal_idx, g1] : galaxy_indices | std::views::enumerate){
        for (auto g2 : galaxy_indices | std::views::drop(gal_idx+1)){

            auto extra_x = empty_col_indices 
                | std::views::filter([x1 = g1.x, x2 = g2.x](size_t idx){
                    return (idx >= x1 && idx <= x2) || (idx <= x1 && idx >= x2);
                });

            auto extra_y = empty_row_indices
                | std::views::filter([y1 = g1.y, y2 = g2.y](size_t idx){
                    return (idx > y1 && idx < y2) || (idx < y1 && idx > y2);
                });
            
            size_t dist = std::abs(g1.x - g2.x) + std::abs(g1.y - g2.y) + 999999*std::ranges::distance(extra_x) + 999999*std::ranges::distance(extra_y);
            total_dist += dist;
        }
    }

    const auto stop_time = std::chrono::steady_clock::now();
    std::println("Total distance is {}", total_dist);
    std::println("Took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
}