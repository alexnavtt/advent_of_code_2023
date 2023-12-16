#include <span>
#include <list>
#include <array>
#include <print>
#include <ranges>
#include <string>
#include <chrono>
#include <algorithm>
#include "load_input.hpp"

namespace views = std::views;
namespace ranges = std::ranges;

using Slot = std::pair<std::string, int>;
using Box  = std::list<Slot>;

int main(){
    std::string input_str = loadInput("day_15_data.txt");
    auto start = std::chrono::steady_clock::now();

    // Convert the inputs into its constituent chunks
    auto inputs = input_str | views::split(',');

    // Part 1
    auto hashFn = [](std::span<char> str) -> int64_t {
        int64_t val = 0;
        for (char c : str){
            val += static_cast<int>(c);
            val *= 17;
            val %= 256;
        }
        return val;
    };
    auto result = ranges::fold_left(inputs | views::transform(hashFn), 0, std::plus{});

    // Part 2
    std::array<Box, 256> map{};
    auto hashMapFn = [&](std::span<char> str) -> void {
        // Transform the input to the values we need
        std::string label = str | views::take_while([](char c){return c != '-' && c != '=';}) | ranges::to<std::string>();
        int box_number    = hashFn(label);
        char action       = (str | views::reverse | views::drop(1)).front();
        int focal_length  = std::atoi(&str.back());

        // Check to see if this already exists
        Box& box = map[box_number];
        auto elem = ranges::find(box, label, [](auto pair){return pair.first;});
        const bool has_value = elem == ranges::end(box);

        // Perform the actions
        if (has_value){
            if (action == '=') box.push_back(std::pair{label, focal_length});
            if (action == '-') box.erase(elem);
        }else{
            if (action == '=') (*elem).second = focal_length;
            if (action == '-') return;
        }
    };
    std::ranges::for_each(inputs, hashMapFn);

    // Accumulate the focusing power
    size_t focusing_power = 0;
    for (auto [box_idx, box] : map | views::enumerate){
        for (auto [slot_idx, slot] : box | views::enumerate){
            auto [label, focal_length] = slot;
            focusing_power += (box_idx+1) * (slot_idx+1) * focal_length;
        }
    }

    auto stop = std::chrono::steady_clock::now();
    std::println("Answer is {}", result);
    std::println("Focusing power is {}", focusing_power);
    std::println("Calculations took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count());
}