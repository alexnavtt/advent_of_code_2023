#include <print>
#include <array>
#include <vector>
#include <string>
#include <ranges>
#include <chrono>
#include <optional>
#include <string_view>
#include <unordered_map>

#include "load_input.hpp"

enum Dir{
    NORTH, 
    SOUTH, 
    EAST,
    WEST
};

struct Index{
    int x = 0;
    int y = 0;

    bool outOfBounds(int max_x, int max_y){
        return x < 0 
            || x >= max_x
            || y < 0
            || y >= max_y;
    }

    bool operator!=(const Index& other) const{
        return x != other.x || y != other.y;
    }

    bool operator==(const Index& other) const{
        return x == other.x && y == other.y;
    }

    Index operator+(const Index& other) const{
        return {x + other.x, y + other.y};
    }
};

std::unordered_map<Dir, Index> directions{
    {NORTH, { 0, -1}},
    {SOUTH, { 0,  1}},
    {EAST , { 1,  0}},
    {WEST , {-1,  0}}
};

std::optional<Dir> getNext(char symbol, Dir last_move){
    Dir next_move;
    switch (symbol){
        case '-':
            if (last_move == EAST || last_move == WEST) return last_move;
            return {};
        case '|':
            if (last_move == NORTH || last_move == SOUTH) return last_move;
            return {};
        case 'L':
            if (last_move == WEST) return NORTH;
            if (last_move == SOUTH) return EAST;
            return {};
        case 'J':
            if (last_move == SOUTH) return WEST;
            if (last_move == EAST) return NORTH;
            return {};
        case '7':
            if (last_move == EAST) return SOUTH;
            if (last_move == NORTH) return WEST;
            return {};
        case 'F':
            if (last_move == WEST) return SOUTH;
            if (last_move == NORTH) return EAST;
            return {};
    }
    return {};
}

// Label actions based on how they turn a path moving northward
static const std::unordered_map<char, char> remap{
    {'F', '1'},  // Turns to the right
    {'L', '1'},  // Turns to the right
    {'7', '2'},  // Turns to the left
    {'J', '2'},  // Turns to the left
    {'-', '3'},  // Does not turn (horizontal)
    {'|', '4'}   // Does not turn (vertical)
};

// To be inside, a point cast northwards must cross the path an odd number of times.
// Crossing two corners on the path which point in different East/West directions counts
// but if they point in the same East/West direction it doesn't count
// Crossing a '-' on the path counts as crossing
// Crossing a '|' on the path doesn't count as anything
// Crossing any of these values not on the path doesn't count as anything
bool isInside(Index idx, const std::vector<std::span<char>>& grid){
    // Cast a ray upwards and see how many times we cross the loop
    size_t num_intersections = 0;
    char last_change = 'x';
    for (; idx.y >= 0; idx.y--){
        char test_char = grid[idx.y][idx.x];
        switch (test_char){
        case '1':
            if (last_change == '2') {num_intersections++; last_change = 'x';}
            else if (last_change == '1') last_change = 'x';
            else last_change = '1';
            break;
        case '2':
            if (last_change == '1') {num_intersections++; last_change = 'x';}
            else if (last_change == '2') last_change = 'x';
            else last_change = '2';
            break;
        case '3':
            num_intersections++;
            break;
        }
    }
    return num_intersections % 2;
}

int main(){
    const auto start_time = std::chrono::steady_clock::now();
    std::string input = loadInput("day_10_data.txt");

    Index start;
    std::vector<std::span<char>> grid;
    for (auto [idx, line] : input | std::views::split('\n') | std::views::enumerate){
        auto pos = std::string_view(line).find('S');
        if (pos != std::string_view::npos){
            start.x = pos;
            start.y = idx;
        }
        grid.push_back(line);
    }
    std::println("Starting at index ({}, {})", start.x, start.y);

    bool first_found = true;
    Dir path1_step, path2_step;
    for (Dir dir : {NORTH, SOUTH, WEST, EAST}){
        Index next = start + directions.at(dir);
        if (next.outOfBounds(grid.front().size(), grid.size())) continue;

        std::optional<Dir> first_move = getNext(grid[next.y][next.x], dir);
        if (first_move && first_found){
            path1_step = dir;
            first_found = false;
        }else if (first_move && !first_found){
            path2_step = dir;
            break;
        }
    }

    int64_t steps = 0;
    for (Index idx1 = start, idx2 = start; (idx1 != idx2) || (steps == 0); ){
        idx1 = idx1 + directions[path1_step];
        idx2 = idx2 + directions[path2_step];

        char& c1 = grid[idx1.y][idx1.x];
        char& c2 = grid[idx2.y][idx2.x];

        path1_step = getNext(c1, path1_step).value();
        path2_step = getNext(c2, path2_step).value();

        steps++;

        // Part 2
        if (remap.contains(c1)) c1 = remap.at(c1);
        if (remap.contains(c2)) c2 = remap.at(c2);
    }

    size_t num_inside = 0;
    for (auto [y, line] : grid | std::views::enumerate){
        for (auto [x, c] : line | std::views::enumerate){
            if (c >= '1' && c <= '4' || c == 'S') continue;
            if (isInside(Index{(int)x, (int)y}, grid)) {c = 'X'; num_inside++;}
            else c = 'O';
        }
        std::println("{}", std::string_view(line));
    }

    const auto stop_time = std::chrono::steady_clock::now();
    std::println("Took {} steps", steps);
    std::println("There are {} internal tiles", num_inside);
    std::println("Computation tool {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time-start_time));
}