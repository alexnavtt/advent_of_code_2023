#include <list>
#include <print>
#include <array>
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
    SOUTH,
    EAST,
    WEST
};

std::string to_string(Direction dir){
    switch (dir){
        case NORTH: return "NORTH";
        case SOUTH: return "SOUTH";
        case EAST:  return "EAST";
        case WEST:  return "WEST";
    }
    std::unreachable();
}

struct Index{
    int xi;
    int yi;
};

struct State{
    Index idx;
    int cost;
    Direction dir;
    uint8_t step_count;
    State* parent = nullptr;
};

struct HashState{
    auto operator()(const State& s) const{
        return (std::hash<int>{}(s.idx.xi) << 1) ^ std::hash<int>{}(s.idx.yi);
    }
};

// Two states are equal if they occupy the same location and have been 
// travelling the same direction for an identical number of steps
struct EqualState{
    bool operator()(const State& s1, const State& s2) const{
        return (s1.idx.xi     == s2.idx.xi    )
            && (s1.idx.yi     == s2.idx.yi    )
            && (s1.dir        == s2.dir       )
            && (s1.step_count == s2.step_count);
    }
};

uint8_t asInt(char c){
    return c - '0';
}

std::pair<Direction, Direction> perpendicularDirections(Direction dir){
    switch (dir){
        case NORTH:
        case SOUTH:
            return {EAST, WEST};
        case EAST:
        case WEST:
            return {NORTH, SOUTH};
    }
    std::unreachable();
}

Index travel(Index idx, Direction dir){
    switch(dir){
        case NORTH: 
            idx.yi -= 1;
            break;
        case SOUTH:
            idx.yi += 1;
            break;
        case EAST:
            idx.xi += 1;
            break;
        case WEST:
            idx.xi -= 1;
    }
    return idx;
}

bool inRange(const Index& idx, const auto& grid){
    if (grid.empty()) return false;
    return idx.xi >= 0 && idx.yi >=0 && idx.yi < grid.size() && idx.xi < grid.front().size();
}

std::array<std::optional<State>, 3> getNeighbours(const State& s, const std::vector<std::vector<uint8_t>>& grid){
    std::array<std::optional<State>, 3> ret_val{};

    // Get the three potential next states
    auto [side1, side2] = perpendicularDirections(s.dir);
    Index forward = travel(s.idx, s.dir);
    Index step1   = travel(s.idx, side1);
    Index step2   = travel(s.idx, side2);

    // Check for validity
    if (s.step_count < 10 && inRange(forward, grid)) {
        ret_val[0] = State{
            .idx        = forward,
            .cost       = s.cost + grid[forward.yi][forward.xi], 
            .dir        = s.dir, 
            .step_count = static_cast<uint8_t>(s.step_count + 1),
        };
    }

    if ((s.step_count == 0 || s.step_count >= 4) && inRange(step1, grid)){
        ret_val[1] = State{
            .idx        = step1,
            .cost       = s.cost + grid[step1.yi][step1.xi], 
            .dir        = side1, 
            .step_count = 1,
        };
    }

    if ((s.step_count == 0 || s.step_count >= 4) && inRange(step2, grid)){
        ret_val[2] = State{
            .idx        = step2,
            .cost       = s.cost + grid[step2.yi][step2.xi], 
            .dir        = side2, 
            .step_count = 1,
        };
    }

    return ret_val;
}

int main(){
    std::string input_str = loadInput("day_17_data.txt");
    auto start_time = std::chrono::steady_clock::now();

    // Convert the input to a vector of vectors of uint8_t's
    auto grid = input_str 
        | views::split('\n')
        | views::transform([](auto line){
            return line 
                | views::transform(asInt)
                | ranges::to<std::vector>();
        })
        | ranges::to<std::vector>();

    // For simplicity: BFS
    std::unordered_map<State, int, HashState, EqualState> visited_states;
    std::list<State> frontier;

    const Index goal_index{(int)(grid.front().size()-1), (int)(grid.size() - 1)};
    State* final_state = nullptr;

    // Start it off with the initial state
    frontier.push_back(State{.idx = Index{0, 0}, .cost = 0, .dir = EAST, .step_count = 0});
    for (State& curr : frontier){

        if (curr.idx.xi == goal_index.xi 
         && curr.idx.yi == goal_index.yi 
         && curr.step_count >= 4
         && (final_state == nullptr || curr.cost < final_state->cost))
        {
            final_state = &curr;
        }

        for (std::optional<State>& candidate : getNeighbours(curr, grid)){
            if (!candidate.has_value()) continue;

            bool update = (!visited_states.contains(*candidate)) || (candidate->cost < visited_states.at(*candidate));
            if (update){
                candidate->parent = &curr;
                visited_states[*candidate] = candidate->cost;
                frontier.push_back(*candidate);
            }
        }
    }

    std::println("Min cost is {} and the final state is at ({}, {})", final_state->cost, final_state->idx.xi, final_state->idx.yi);
    while (final_state->parent){
        grid[final_state->idx.yi][final_state->idx.xi] = 0;
        final_state = final_state->parent;
    }

    for (auto row : grid){
        std::println("{}", row | views::transform([](uint8_t i){return std::to_string(i);}) | views::join | ranges::to<std::string>());
    }

    auto stop_time = std::chrono::steady_clock::now();
    std::println("Calculations took {} milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count());
}