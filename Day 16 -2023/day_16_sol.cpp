#include <print>
#include <ranges>
#include <string>
#include <chrono>
#include <optional>
#include <algorithm>
#include <unordered_map>
#include "load_input.hpp"

namespace views = std::views;
namespace ranges = std::ranges;

enum Direction{
    NORTH,
    EAST,
    SOUTH,
    WEST
};

struct Index{
    int x;
    int y;
};

struct IndexHash{
    size_t operator()(const Index& idx) const {
        return (std::hash<int>{}(idx.x) << 1) ^ (std::hash<int>{}(idx.y));
    }
};

struct IndexEqual{
    bool operator()(const Index& s1, const Index& s2) const {
        return s1.x == s2.x && s1.y == s2.y;
    }
};

std::pair<int, int> travel(Direction dir){
    switch (dir){
        case NORTH: return { 0, -1};
        case EAST : return { 1,  0};
        case SOUTH: return { 0,  1};
        case WEST : return {-1,  0};
    }
    std::unreachable();
}

std::pair<Direction, std::optional<Direction>> updateDirection(Direction dir, char next_char){
    switch (next_char){
        case '.':
            return {dir, {}};
        case '/':
            if (dir == NORTH) return {EAST , {}};
            if (dir == SOUTH) return {WEST , {}};
            if (dir == EAST)  return {NORTH, {}};
            if (dir == WEST)  return {SOUTH, {}};
        case '\\':
            if (dir == NORTH) return {WEST , {}};
            if (dir == SOUTH) return {EAST , {}};
            if (dir == EAST)  return {SOUTH, {}};
            if (dir == WEST)  return {NORTH, {}};
        case '-':
            if (dir == NORTH) return {EAST , WEST};
            if (dir == SOUTH) return {EAST , WEST};
            if (dir == EAST)  return {EAST, {}};
            if (dir == WEST)  return {WEST, {}};
        case '|':
            if (dir == NORTH) return {NORTH, {}};
            if (dir == SOUTH) return {SOUTH, {}};
            if (dir == EAST)  return {NORTH, SOUTH};
            if (dir == WEST)  return {NORTH, SOUTH};
    }
    std::unreachable();
}

int main(){
    std::string input_str = loadInput("day_16_data.txt");
    auto start = std::chrono::steady_clock::now();

    // Get a double indexable grid of characters
    auto rows = input_str 
        | views::split('\n') 
        | views::transform([](auto line){return std::span<char>(line);}) 
        | ranges::to<std::vector>();
    const int x_dim = rows[0].size();
    const int y_dim = rows.size();

    // Use a multimap to keep track of energized states
    Index start_state{.x = -1, .y = 0};
    Direction start_direction = EAST;
    std::unordered_multimap<Index, Direction, IndexHash, IndexEqual> energized_states{};

    // Recursive lambda to cast a beam in the grid
    auto castBeam = [&]<typename Self>(this const Self& self, Index idx, Direction dir) -> void {
        // Step the state along to the next value
        const auto [dx, dy] = travel(dir);
        idx.x += dx;
        idx.y += dy;
        if (idx.x < 0 || idx.x >= x_dim || idx.y < 0 || idx.y >= y_dim) return;
        
        // Check to see if this state has already been visited
        auto [start, end] = energized_states.equal_range(idx);
        bool already_visited = ranges::contains(ranges::subrange(start, end) | views::values, dir);
        if (already_visited) return;

        // Determine what the new direction will be when leaving this state
        energized_states.insert({idx, dir});
        auto [dir1, dir2] = updateDirection(dir, rows[idx.y][idx.x]);
        self(idx, dir1);
        if (dir2) self(idx, *dir2);
    };
    castBeam(start_state, start_direction);

    // Count the number of energized states
    auto num_energized = ranges::distance(energized_states | views::keys | views::chunk_by(IndexEqual{}));

    // Part 2 - Do the same for all the starting positions
    auto top_start   = views::iota(0, x_dim) | views::transform([&](int i){return Index{i, -1};});
    auto bot_start   = views::iota(0, x_dim) | views::transform([&](int i){return Index{i, y_dim};});
    auto left_start  = views::iota(0, y_dim) | views::transform([&](int i){return Index{-1, i};});
    auto right_start = views::iota(0, y_dim) | views::transform([&](int i){return Index{x_dim, i};});
    auto count_energized = [&](Index idx){
        energized_states.clear();
        Direction start_dir = (idx.x == -1)    ? EAST  : 
                              (idx.x == x_dim) ? WEST  : 
                              (idx.y == -1)    ? SOUTH : NORTH;
        castBeam(idx, start_dir);
        return ranges::distance(energized_states | views::keys | views::chunk_by(IndexEqual{}));
    };

    // Find the maximum value
    auto max_energized = ranges::max({
        ranges::max(top_start   | views::transform(count_energized)),
        ranges::max(bot_start   | views::transform(count_energized)),
        ranges::max(left_start  | views::transform(count_energized)),
        ranges::max(right_start | views::transform(count_energized))
    });

    auto stop = std::chrono::steady_clock::now();
    std::println("Done, energizing {} states", num_energized);
    std::println("Maximum energization is {}", max_energized);
    std::println("Calculations took {} milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count());
}