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
        if (!std::ranges::contains(row, '#')) {empty_row_indices.push_back(y_idx); continue;}
        
        // Get the indices of all '#' characters
        auto galaxy_positions = row 
            | std::views::enumerate 
            | std::views::filter([](auto elem){return std::get<1>(elem) == '#';}) 
            | std::views::keys
            | std::views::transform([y_idx](auto x_idx){return Index{(int)x_idx, (int)y_idx};});
        
        // Append these to the existing list of indices
        std::ranges::copy(galaxy_positions, std::back_inserter(galaxy_indices));
    }
    for (auto col_idx : std::views::iota(size_t(0), line_length)){
        // Construct a column view and check to see if it has any galaxies
        auto col = input_str | std::views::drop(col_idx) | std::views::stride(line_length+1);
        if (!std::ranges::contains(col, '#')) empty_col_indices.push_back(col_idx);
    }

    size_t total_dist = 0;
    for (auto [gal_idx, g1] : galaxy_indices | std::views::enumerate){
        for (auto g2 : galaxy_indices | std::views::drop(gal_idx+1)){

            const size_t& xmin = std::min(g1.x, g2.x);
            const size_t& xmax = std::max(g1.x, g2.x);
            const size_t& ymin = std::min(g1.y, g2.y);
            const size_t& ymax = std::max(g1.y, g2.y);

            auto extra_x = empty_col_indices | std::views::filter([xmin,xmax](size_t idx){
                    return idx == std::clamp<size_t>(idx, xmin, xmax);
                });

            auto extra_y = empty_row_indices | std::views::filter([ymin, ymax](size_t idx){
                    return idx == std::clamp<size_t>(idx, ymin, ymax);
                });
            
            size_t dist = std::abs(g1.x - g2.x) + std::abs(g1.y - g2.y) + 999999*std::ranges::distance(extra_x) + 999999*std::ranges::distance(extra_y);
            total_dist += dist;
        }
    }

    const auto stop_time = std::chrono::steady_clock::now();
    std::println("Total distance is {}", total_dist);
    std::println("Took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
}