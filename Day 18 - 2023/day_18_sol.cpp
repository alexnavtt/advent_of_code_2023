#include <list>
#include <print>
#include <array>
#include <ranges>
#include <string>
#include <chrono>
#include <algorithm>
#include "load_input.hpp"

namespace views = std::views;
namespace ranges = std::ranges;

struct Index{
    int64_t x{0};
    int64_t y{0};

    bool operator==(const Index& other) const {return x == other.x && y == other.y;}
    bool operator!=(const Index& other) const {return !(*this == other);}
    bool outOfRange(const auto& grid){
        return x < 0 || x >= grid.front().size() || y < 0 || y >= grid.size();
    }
};

struct CompIndex{
    bool operator()(const Index& idx1, const Index& idx2) const{
        if (idx1.y == idx2.y) return idx1.x < idx2.x;
        else return idx1.y < idx2.y;
    }
};

template<>
struct std::formatter<Index>{
    constexpr auto parse(std::format_parse_context& ctx){return ctx.begin();}
    auto format(const Index& idx, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "({}, {})", idx.x, idx.y);
    }
};

using Shape = std::list<Index>;
using ShapeIterator = Shape::iterator;

enum Direction{
    RIGHT, 
    DOWN,
    LEFT,
    UP
};
constexpr static std::array<Direction, 4> directions{UP, RIGHT, DOWN, LEFT};

struct Node{
    Direction dir;
    size_t step_count = 0;
    size_t color_code;
};

Node parseLine(std::span<char> line){
    Node output;
    auto segments = line | views::split(' ');
    
    // Assign the direction
    auto dir_label = segments.front().front();
    output.dir = dir_label == 'R' ? RIGHT : 
                 dir_label == 'L' ? LEFT  :
                 dir_label == 'U' ? UP    :
                                    DOWN  ;

    // Assign the step count
    auto count_label = *ranges::next(segments.begin(), 1);
    output.step_count = std::atoll(count_label.data());

    // Assign the hex color code
    auto color_label  = *ranges::next(segments.begin(), 2) | views::drop(2) | views::take(6);
    output.color_code = std::stoll(std::string(std::string_view(color_label)), nullptr, 16);

    return output;
}

void reinterpretNode(Node& n){
    n.step_count = (n.color_code & 0xFFFFF0) >> 4;
    n.dir = static_cast<Direction>(n.color_code & 0x00000F);
}

void update(Index& idx, Direction dir, size_t step_count){
    switch(dir){
        case RIGHT:
            idx.x += step_count;
            break;
        case LEFT:
            idx.x -= step_count;
            break;
        case UP:
            idx.y -= step_count;
            break;
        case DOWN:
            idx.y += step_count;
            break;
    }
}

std::list<Index>::iterator loopingNext(std::list<Index>& vertices, std::list<Index>::iterator& place){
    if (ranges::next(place) == vertices.end()) return vertices.begin();
    else return ranges::next(place);
}

std::list<Index>::iterator loopingPrev(std::list<Index>& vertices, std::list<Index>::iterator& place){
    if (place == vertices.begin()) return ranges::prev(vertices.end());
    else return ranges::prev(place);
}

size_t splitShape(std::list<Shape>& shapes) {
    size_t area_removed = 0;

    // Get the first shape if it exists
    if (shapes.front().empty()) {return area_removed;}
    Shape& shape = shapes.front();

    // Determine what direction to walk the shape in
    ShapeIterator top_left_corner = ranges::min_element(shape, CompIndex{});
    auto stepClockwise = loopingNext(shape, top_left_corner)->y == top_left_corner->y ?
                         loopingNext:
                         loopingPrev;
    ShapeIterator top_right_corner = stepClockwise(shape, top_left_corner);

    // Every time we cut into the top row, split off a new shape
    ShapeIterator shape_start = top_right_corner;
    for (ShapeIterator it = top_right_corner; it != top_left_corner; it = stepClockwise(shape, it)){

        // If we hit a crossing, copy all points between the last time and now
        if (it->y == top_left_corner->y && it->x > top_left_corner->x && it->x < top_right_corner->x){
            // Generate a new shape and fill it with the vertices
            Shape& new_shape = shapes.emplace_back();

            it = stepClockwise(shape, it);
            for (auto moved_it = shape_start; moved_it != it; moved_it = stepClockwise(shape, moved_it)){
                new_shape.push_back(*moved_it);
                shape.erase(moved_it);
            }
            shape_start = it;
            area_removed += std::abs(new_shape.back().x - shape_start->x - 1);
        }
    }

    return area_removed;
};

std::pair<size_t, bool> slice(Shape& shape) {
    // Find vertex with the highest (smallest) y index
    auto left1 = ranges::min_element(shape, CompIndex{});

    // Get the neighbours 2 to the left and 3 to the right of this vertex
    bool forward = loopingNext(shape, left1)->y == left1->y;
    auto stepRight = forward ? &loopingNext : loopingPrev;
    auto stepLeft  = forward ? &loopingPrev : loopingNext;

    auto right1 = stepRight(shape, left1);
    auto right2 = stepRight(shape, right1);
    auto right3 = stepRight(shape, right2);
    auto left2  = stepLeft(shape, left1);
    auto left3  = stepLeft(shape, left2);

    // Calculate the area from the given indices
    int64_t min_x = left1->x;
    int64_t min_y = left1->y;
    int64_t max_x = right1->x;
    int64_t max_y = std::min(left2->y, right2->y);

    // Terminating condition
    if (shape.size() == 4){
        int64_t final_area = (max_x - min_x + 1)*(max_y - min_y) + max_x - min_x + 1;
        shape.clear();
        return {final_area, false};
    }

    // Determine if we've reached the critical point where it needs to be split
    bool needs_split = false;
    for (const Index& vert : shape){
        if (vert.y <= max_y && vert.x < max_x && vert.x > min_x && vert != *right3 && vert != *left3){
            max_y = vert.y;
            needs_split = true;
        }
    }

    // Calculate the area
    int64_t area = (max_x - min_x + 1)*(max_y - min_y);

    // Shift vertices so we still have a valid convex shape for the next iteration
    right1->y = max_y;
    if (*right1 == *right2){
        shape.erase(right2);
        right2 = right3;
    }

    left1->y = max_y;
    if (*left1 == *left2){
        shape.erase(left2);
        left2 = left3;
    }

    if (left2->y == max_y){
        if (left2->x > left1->x) area += left2->x - left1->x;
        shape.erase(left1);
    }

    if (right2->y == right1->y){
        if (right1->x > right2->x) area +=right1->x - right2->x;
        shape.erase(right1);
    }

    return {area, needs_split};
};

int main(){
    std::string input_str = loadInput("day_18_data.txt");
    const auto start_time = std::chrono::steady_clock::now();

    auto nodes = input_str 
        | views::split('\n') 
        | views::transform(parseLine)
        | ranges::to<std::vector>();

    if (auto part2 = true)
        ranges::for_each(nodes, reinterpretNode);

    // Define the vertices of our outline
    std::list<Shape> shapes(1);
    Index curr{0, 0};
    for (const Node& node: nodes){
        shapes.front().push_back(curr);
        update(curr, node.dir, node.step_count);
    }

    size_t total = 0; 
    while (!shapes.empty()){
        auto [area_chunk, needs_split] = slice(shapes.front());
        total += area_chunk;
        if (needs_split) total += splitShape(shapes);
        if (shapes.front().empty()){
            shapes.pop_front();
        }
    }

    const auto stop_time = std::chrono::steady_clock::now();
    std::println("Total is {}", total);
    std::println("Calculations took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
}