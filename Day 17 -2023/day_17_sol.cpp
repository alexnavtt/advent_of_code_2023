#include <print>
#include <array>
#include <queue>
#include <ranges>
#include <string>
#include <chrono>
#include <optional>
#include <unordered_map>
#include "load_input.hpp"

namespace views = std::views;
namespace ranges = std::ranges;

// Problem configuration
static constexpr bool part2 = true;
static constexpr int min_steps = part2 ? 4 : 0;
static constexpr int max_steps = part2 ? 10 : 3;

// Set up grid directions
// ------------------------------------------------------------------------------------------------

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

// Define the grid state
// ------------------------------------------------------------------------------------------------

struct Index{
    int xi;
    int yi;

    bool operator==(const Index& other) const{
        return xi == other.xi && yi == other.yi;
    }
};

struct State{
    Index idx;
    int cost;
    Direction dir;
    uint8_t step_count;
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
        return (s1.idx        == s2.idx       )
            && (s1.dir        == s2.dir       )
            && (s1.step_count == s2.step_count);
    }
};

// Use a greater than comparison to get min out of std::priority_queue
struct CostGreater{
    bool operator()(const State& s1, const State& s2) const {
        return s1.cost > s2.cost;
    }
};

// Logic for deciding where to go next
// ------------------------------------------------------------------------------------------------

// Determine which directions can be turned towards
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

// Update state in a given direction
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

// Gather all valid neighbours considering grid bounds and step limits
std::array<std::optional<State>, 3> getNeighbours(const State& s, const std::vector<std::vector<uint8_t>>& grid){
    std::array<std::optional<State>, 3> ret_val{};
    if (grid.empty()) return ret_val;

    // Bounds checking lambda
    auto inRange = [&grid](const Index& idx) -> bool {
        return idx.xi >= 0 && idx.yi >=0 && idx.yi < grid.size() && idx.xi < grid.front().size();
    };

    // Get the three potential next states
    auto [side1, side2] = perpendicularDirections(s.dir);
    Index forward = travel(s.idx, s.dir);
    Index step1   = travel(s.idx, side1);
    Index step2   = travel(s.idx, side2);

    // Check for validity moving forward
    if (s.step_count < max_steps && inRange(forward)) {
        ret_val[0] = State{
            .idx        = forward,
            .cost       = s.cost + grid[forward.yi][forward.xi], 
            .dir        = s.dir, 
            .step_count = static_cast<uint8_t>(s.step_count + 1),
        };
    }

    // Check for validity turning to either side
    if ((s.step_count == 0 || s.step_count >= min_steps) && inRange(step1)){
        ret_val[1] = State{
            .idx        = step1,
            .cost       = s.cost + grid[step1.yi][step1.xi], 
            .dir        = side1, 
            .step_count = 1,
        };
    }

    if ((s.step_count == 0 || s.step_count >= min_steps) && inRange(step2)){
        ret_val[2] = State{
            .idx        = step2,
            .cost       = s.cost + grid[step2.yi][step2.xi], 
            .dir        = side2, 
            .step_count = 1,
        };
    }

    return ret_val;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

int main(){
    std::string input_str = loadInput("day_17_data.txt");
    const auto start_time = std::chrono::steady_clock::now();

    // Convert the input to a vector of vectors of uint8_t's
    const auto grid = input_str 
        | views::split('\n')
        | views::transform([](auto line){
            return line 
                | views::transform([](char c){return static_cast<uint8_t>(c - '0');})
                | ranges::to<std::vector>();
        })
        | ranges::to<std::vector>();

    // Use Dijkstra
    std::priority_queue<State, std::vector<State>, CostGreater> queue;
    std::unordered_map<State, int, HashState, EqualState> visited_states;

    const Index goal_index{(int)(grid.front().size()-1), (int)(grid.size() - 1)};

    // Start it off with the initial state
    queue.push(State{.idx = Index{0, 0}, .cost = 0, .dir = EAST, .step_count = 0});
    while (!queue.empty()){
        // Get the path with the current lowest cost
        const State curr = queue.top();

        // Check if we've found the goal
        if (curr.idx == goal_index && curr.step_count >= min_steps) break;

        queue.pop();
        for (std::optional<State>& candidate : getNeighbours(curr, grid)){
            if ( !candidate.has_value() || 
                (visited_states.contains(*candidate) && candidate->cost >= visited_states.at(*candidate))
            ) continue;

            visited_states[*candidate] = candidate->cost;
            queue.push(*candidate);
        }
    }

    const auto stop_time = std::chrono::steady_clock::now();
    const State& final_state = queue.top();
    std::println("Min cost is {} and the final state is at ({}, {})", final_state.cost, final_state.idx.xi, final_state.idx.yi);
    std::println("Calculations took {} milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count());
}