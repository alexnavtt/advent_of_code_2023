#include <print>
#include <string>
#include <ranges>
#include <chrono>
#include <vector>
#include "load_input.hpp"

namespace views = std::ranges::views;
using namespace std::string_view_literals;

bool zipNotEqual(auto zip_view){
    return std::get<0>(zip_view) != std::get<1>(zip_view);
}

template<std::ranges::common_range R>
bool checkSymmetryRow(R pattern, size_t idx, bool fix_smudge = false){
    // Special equality operator that's allowed to fix a smudge once
    bool fixed = false;
    auto checkEqual = [fix_smudge, &fixed](auto line1, auto line2) -> bool {
        auto diff = views::zip(line1, line2) | views::filter([](auto zip_view){return zipNotEqual(zip_view);});
        const int num_chars_different = std::ranges::distance(diff);
        if (num_chars_different == 0) return true;
        if (fix_smudge && num_chars_different == 1 && !fixed) return fixed = true;
        return false; 
    };

    // Iterate forward and backward along the pattern until we reach one of the ends
    auto forward  = pattern | views::drop(idx+1);
    auto backward = pattern | views::take(idx+1) | views::reverse;
    if (forward.empty() || backward.empty()) return false;
    for (auto [l1, l2] : views::zip(forward, backward)){
        if (!checkEqual(l1, l2)) return false;
    }

    // If we're fixing smudges we don't want symmetry from a non-fixed version
    if (fix_smudge && !fixed) return false;
    return true;
}

template<std::ranges::common_range R>
bool checkSymmetryCol(R pattern, size_t idx, bool fix_smudge = false){
    // Transpose the pattern
    const int line_length = pattern.front().size();
    auto cols = views::iota(0, line_length) | views::transform(
        [&](int i){
            return pattern 
            | views::join 
            | views::drop(i) 
            | views::stride(line_length);
        }
    );

    // Pass to the row function
    return checkSymmetryRow(cols, idx, fix_smudge);
}

int main(){
    const bool part2 = true;
    std::string input_str = loadInput("day_13_data.txt");
    auto start_time = std::chrono::steady_clock::now();

    // Split the input into individual patterns, each of which is a vector of string_views
    auto patterns = input_str
        | views::split("\n\n"sv)
        | views::transform([](auto pattern){
            return pattern
            | views::split('\n')
            | views::transform([](auto split_chunk){return std::string_view(split_chunk);})
            | std::ranges::to<std::vector<std::string_view>>();
        }
    );

    size_t total = 0;
    for (auto lines : patterns){
        // Extract the features of this pattern
        const int line_length = lines.front().size();
        const int num_lines   = std::ranges::distance(lines);

        // Check each pair of rows for symmetry
        bool found = false;
        for (size_t idx : views::iota(0, num_lines)){
            if (checkSymmetryRow(lines, idx, part2)){
                total += 100*(idx+1);
                found = true;
                break;
            }
        }

        // For some reason we only want the first match
        if (found) continue;

        // Transpose the columns so they are now the rows
        auto cols = views::iota(0, line_length) | views::transform(
            [&](int i){
                return lines 
                | views::join 
                | views::drop(i) 
                | views::stride(line_length);
            }
        );

        // Check each pair of columns for symmetry
        for (auto idx : views::iota(0, line_length)){
            if (checkSymmetryRow(cols, idx, part2)){
                total += idx+1;
            }
        }
    }

    auto stop_time = std::chrono::steady_clock::now();
    std::println("Total was {}", total);
    std::println("Calculations took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
}