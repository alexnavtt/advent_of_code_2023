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

    bool outOfBounds(int max_x, int max_y) const{
        return x < 0 || x >= max_x|| y < 0|| y >= max_y;
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

static const std::unordered_map<Dir, Index> directions{
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

enum DirectionChange : char{
    NONE            = '0',
    RIGHT           = '1',
    LEFT            = '2',
    NONE_HORIZONTAL = '3',
    NONE_VERTICAL   = '4'
};

// Label actions based on how they turn a path moving northward
static const std::unordered_map<char, char> remap{
    {'F', DirectionChange::RIGHT},
    {'L', DirectionChange::RIGHT},
    {'7', DirectionChange::LEFT},
    {'J', DirectionChange::LEFT},
    {'-', DirectionChange::NONE_HORIZONTAL},
    {'|', DirectionChange::NONE_VERTICAL}
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
    char last_change = NONE;
    for (; idx.y >= 0; idx.y--){
        char test_char = grid[idx.y][idx.x];
        switch (test_char){
        case RIGHT:
            if (last_change == LEFT) {num_intersections++; last_change = NONE;}
            else if (last_change == RIGHT) last_change = NONE;
            else last_change = RIGHT;
            break;
        case LEFT:
            if (last_change == RIGHT) {num_intersections++; last_change = NONE;}
            else if (last_change == LEFT) last_change = NONE;
            else last_change = LEFT;
            break;
        case NONE_HORIZONTAL:
            num_intersections++;
            break;
        case NONE:
        case NONE_VERTICAL:
            break; // Do nothing in this case
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

    // Find the two starting pipes from S
    bool first_found = false;
    Dir path1_step, path2_step;
    for (const Dir dir : {NORTH, SOUTH, EAST, WEST}){
        const Index next = start + directions.at(dir);
        if (next.outOfBounds(grid.front().size(), grid.size())) continue;

        // Check to see if this character gives a valid move from S
        std::optional<Dir> first_move = getNext(grid[next.y][next.x], dir);
        if (first_move && !first_found){
            path1_step = dir;
            first_found = true;
        }else if (first_move && first_found){
            path2_step = dir;
            break;
        }
    }

    // Determine the pipe type of S for part 2
    char& start_char = grid[start.y][start.x];
    if (path1_step == NORTH){
        switch (path2_step){
            case SOUTH: start_char = NONE_VERTICAL; break;
            case EAST : start_char = RIGHT; break;
            case WEST : start_char = LEFT; break;
        }
    }else if (path1_step == SOUTH){
        switch (path2_step){
            case EAST : start_char = RIGHT; break;
            case WEST : start_char = LEFT; break;
        }
    }else{
        start_char = NONE_HORIZONTAL;
    }

    int64_t steps = 0;
    for (Index idx1 = start, idx2 = start; (idx1 != idx2) || (steps == 0); steps++){
        idx1 = idx1 + directions.at(path1_step);
        idx2 = idx2 + directions.at(path2_step);

        char& c1 = grid[idx1.y][idx1.x];
        char& c2 = grid[idx2.y][idx2.x];

        path1_step = getNext(c1, path1_step).value();
        path2_step = getNext(c2, path2_step).value();

        // Part 2
        if (remap.contains(c1)) c1 = remap.at(c1);
        if (remap.contains(c2)) c2 = remap.at(c2);
    }

    size_t num_inside = 0;
    for (auto [y, line] : grid | std::views::enumerate){
        for (auto [x, c] : line | std::views::enumerate){
            if (c >= '1' && c <= '4') continue;
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