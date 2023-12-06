#include <array>
#include <print>
#include <format>
#include <limits>
#include <string>
#include <ranges>
#include <vector>
#include <thread>
#include <fstream>
#include <iostream>
#include <optional>
#include <algorithm>

struct RangeMap{
    const int64_t dest_start;
    const int64_t source_start;
    const int64_t dist;

    std::optional<int64_t> map(int64_t source_num) const{
        const int64_t offset = source_num - source_start;
        return (source_num >= source_start && offset < dist) ?
            dest_start + offset :
            std::optional<int64_t>{};
    }

   std::string print(){
        return std::format("Dest start: {} | Source start {} | Dist: {}\n", dest_start, source_start, dist);
    }
};

int64_t mapToNext(int64_t val, const std::vector<RangeMap>& range_maps){
    for (const RangeMap& map : range_maps){
        std::optional<int64_t> mapped_val = map.map(val);
        if (mapped_val.has_value()){
            val = *mapped_val;
            break;
        }
    }
    return val;
}

int main(){
    std::ifstream text_file{"day_5_data.txt"};
    std::string seeds_str;
    std::getline(text_file, seeds_str);
    
    // Process the first line to get a vector of ints of the seeds
    auto seeds = seeds_str 
        | std::views::drop_while([](char c){return c < '0' || c > '9';})
        | std::views::split(' ')
        | std::views::transform([](auto digits_view){return std::atoll(digits_view.data());})
        | std::ranges::to<std::vector<int64_t>>();
    
    // Remove the next two useless lines
    text_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    text_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Parse the mapping logic
    size_t group_idx = 0;
    std::array<std::vector<RangeMap>, 7> map_groups;
    for (std::string line; std::getline(text_file, line); ){
        // If we're moving from one group to the next, take care of the useless lines
        if (line.empty()) {
            group_idx++;
            text_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        // Extract the individual values from the line and create the corresponding RangeMap
        auto vals = line 
            | std::views::split(' ')
            | std::views::transform([](auto digits_view){return std::atoll(digits_view.data());});
        auto vals_it = vals.begin();
        map_groups[group_idx].emplace_back(RangeMap{
            .dest_start   = *vals_it++, 
            .source_start = *vals_it++, 
            .dist         = *vals_it++
        });
    }

    // Apply each seed to find the locations
    std::vector<int64_t> locations(seeds.size());
    for (auto [seed, location] : std::views::zip(seeds, locations)){
        // Problem 1
        int64_t intermediate_val = seed;
        for (const std::vector<RangeMap>& map_group : map_groups){
            intermediate_val = mapToNext(intermediate_val, map_group);
        }
        location = intermediate_val;
    }

    int64_t closest_location = *std::min_element(locations.begin(), locations.end());
    std::println("The closest location is {} for problem 1", closest_location);

    // Problem 2
    auto seed_ranges = seeds 
        | std::views::chunk(2) 
        | std::views::transform([](auto seed_pair){return std::views::iota(seed_pair[0], seed_pair[0] + seed_pair[1]);});
    size_t num_ranges = std::ranges::distance(seed_ranges);

    // There's a lot of number crunching, so split it up between threads
    std::vector<int64_t> min_locations(num_ranges, std::numeric_limits<int64_t>::max());
    std::vector<std::thread> workers(num_ranges);                       
    
    // Same as problem 1, but with the seed ranges we calculated above                                                                                                                                                                                                                                                                                                                                          ;
    for (auto [idx, seed_range] : seed_ranges | std::views::enumerate){
        int64_t& min_location = min_locations[idx];
        workers[idx] = std::thread([&min_location, seed_range, &map_groups, idx](){
            std::println("Starting processing seed range {}", idx, (size_t)&min_location);
            for (int64_t seed : seed_range){
                for (const std::vector<RangeMap>& map_group : map_groups){
                    seed = mapToNext(seed, map_group);
                }
                min_location = std::min(min_location, seed);
            }
            std::println("Done processing seed range {}", idx);
        });
    }

    for (std::thread& t : workers){
        t.join();
    }
    std::println("The closest location is {} for problem 2", *std::min_element(min_locations.begin(), min_locations.end()));
}