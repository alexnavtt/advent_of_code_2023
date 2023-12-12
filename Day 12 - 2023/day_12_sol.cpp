#include <span>
#include <print>
#include <string>
#include <ranges>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <unordered_map>

#include <assert.h>
#include "load_input.hpp"

typedef std::pair<std::string, std::vector<int64_t>> InputType;

struct PairHash{
    auto operator()(const InputType& pair) const{
        return std::hash<std::string>{}(pair.first);
    }
};

struct PairEqual{
    auto operator()(const InputType& pair1, const InputType& pair2) const{
        return pair1.first == pair2.first && pair1.second == pair2.second;
    }
};

int64_t processLine(std::string_view data, std::span<int64_t> chunk_sizes, int64_t level = 0){
    // Handle caching
    static std::unordered_map<InputType, int64_t, PairHash, PairEqual> cache;
    std::vector<int64_t> chunk_vec(chunk_sizes.begin(), chunk_sizes.end());
    auto pair_val = InputType{std::string(data), chunk_vec};
    if (cache.contains(pair_val)){
        return cache.at(pair_val);
    }
    
    // Extract the first chunk
    int64_t size = chunk_sizes[0];
    if (data.size() < size) {return 0;}

    // Find out which chunks have hashes
    auto hashed_chunks = data 
        | std::views::chunk_by([](char c1, char c2){return (c1 == '.' && c2 == '.') || (c1 != '.' && c2 != '.');})
        | std::views::filter([](auto chunk){return std::string_view(chunk).contains('#');});

    // If we have more hashed chunks left than chunks to find, it's impossible
    if (std::ranges::distance(hashed_chunks) > chunk_sizes.size()) {return 0;}

    // Find all the sections that could hold the next chunk
    int64_t num_combinations = 0;
    const int64_t window_cutoff = data.size() - size + 1;
    for (int64_t idx : std::views::iota(0, window_cutoff)){
        std::string_view window    = data.substr(idx, size);
        std::string_view remainder = data.substr(idx + size);

        if (!window.contains('.') && !remainder.starts_with('#')) {
            if (chunk_sizes.size() == 1 && !remainder.contains('#')){
                num_combinations++; 
            }else if (remainder.size() >= chunk_sizes[1]){
                num_combinations += processLine(remainder.substr(1), chunk_sizes.subspan(1), level+1);
            }
        }

        if (window.starts_with('#')) break;
    }

    cache[pair_val] = num_combinations;
    return num_combinations;
}

int64_t processLineBruteForce(std::string_view data, std::span<int64_t> chunk_sizes){
    const int num_questions = std::ranges::count(data, '?');
    const int num_iters = 1 << num_questions;
    int num_combinations = 0;
    for (int iter = num_iters-1; iter >= 0; iter--){
        std::string copy(data);
        int bit_mask = 1 << (num_questions-1);
        for (auto idx = 0; idx < copy.size(); idx++){
            if (copy[idx] == '?'){
                copy[idx] = (iter & bit_mask ? '#' : '.');
                bit_mask = bit_mask >> 1;
            }
        }
        
        auto chunks = copy 
            | std::views::chunk_by([](char c1, char c2){return c1 == c2;}) 
            | std::views::filter([](auto chunk){return std::ranges::contains(chunk, '#');});
        if (std::ranges::distance(chunks) != chunk_sizes.size()) continue;
        bool valid = true;
        for (auto [chunk, count] : std::views::zip(chunks, chunk_sizes)){
            valid = valid && (chunk.size() == count);
        }
        num_combinations += static_cast<int>(valid);
    }
    return num_combinations;
}

int main(){
    std::string data_str = loadInput("day_12_data.txt");
    auto start_time = std::chrono::steady_clock::now();

    const bool part2 = true;

    int64_t num_combinations = 0;
    for (auto line : data_str | std::views::split('\n') | std::views::transform([](auto l){return std::string_view(l);})){
        // Divide the input into the two parts
        auto divider = line.find(' ');
        auto data = line.substr(0, divider);
        auto chunks = line.substr(divider+1) 
            | std::views::split(',') 
            | std::views::transform([](auto chars){return std::atoi(chars.data());})
            | std::ranges::to<std::vector<int64_t>>();

        if constexpr(part2){
            std::string new_data = std::views::repeat(data, 5) | std::views::join_with('?') | std::ranges::to<std::string>();
            std::vector<int64_t> new_chunks = std::views::repeat(chunks, 5) | std::views::join | std::ranges::to<std::vector<int64_t>>();
            num_combinations += processLine(std::string_view(new_data), new_chunks);
        }else{
            num_combinations += processLine(data, chunks);
        }
    }

    auto stop_time = std::chrono::steady_clock::now();
    std::println("Total of {} combinations", num_combinations);
    std::println("Took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
}