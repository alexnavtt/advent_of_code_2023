#include <print>
#include <array>
#include <deque>
#include <string>
#include <ranges>
#include <chrono>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "load_input.hpp"

namespace ranges = std::ranges;
namespace views  = std::views;
using namespace std::string_view_literals;

enum Signal {
    HIGH,
    LOW,
    NONE
};

struct SignalQueueType{
    std::string_view source;
    std::string_view target;
    Signal signal;
};

std::string toString(Signal sig){
    switch (sig){
        case HIGH: return "HIGH";
        case LOW : return "LOW";
        case NONE: return "NONE";
    }
    std::unreachable();
}

Signal invert(Signal sig){
    switch (sig){
        case HIGH: return LOW;
        case LOW : return HIGH;
        default  : return NONE;
    }
}

enum ModuleTag{
    FLIPFLOP,
    CONJUNCTION,
    BROADCASTER,
};

struct Module{
    static size_t high_pulses;
    static size_t low_pulses;
    std::vector<std::string_view> outputs;
    virtual Signal process(Signal signal, std::string_view source) = 0;
    virtual ModuleTag tag() const = 0;
    virtual bool isInit() const = 0;
    virtual std::vector<bool> state() const{
        return {};
    };
    Signal recordOutput(Signal output){
        if (output == HIGH) high_pulses += outputs.size();
        if (output == LOW)  low_pulses  += outputs.size();
        return output;
    }
};

struct FlipFlop : public Module{
    Signal internal_state = LOW;
    ModuleTag tag() const override {return FLIPFLOP;}
    bool isInit() const override {
        return internal_state == LOW;
    };
    Signal process(Signal signal, std::string_view /*source*/) override {
        if (signal == LOW) {
            internal_state = invert(internal_state);
            return recordOutput(internal_state);
        }
        else return NONE;
    }
    std::vector<bool> state() const override {
        return {internal_state == HIGH};
    }
};

struct Conjunction : public Module{
    std::unordered_map<std::string_view, Signal> last_inputs;
    ModuleTag tag() const override {return CONJUNCTION;}
    bool isInit() const override{
        return ranges::all_of(last_inputs | views::values, [](Signal sig){return sig == LOW;});
    }
    Signal process(Signal signal, std::string_view source) override {
        last_inputs.at(source) = signal;
        if (ranges::all_of(last_inputs | views::values, [](Signal sig){return sig == HIGH;})){
            return recordOutput(LOW);
        }else{
            return recordOutput(HIGH);
        }
    }
    std::vector<bool> state() const override {
        return last_inputs | views::values | views::transform([](Signal sig){return sig == HIGH;}) | ranges::to<std::vector>();
    }
};

struct Broadcaster : public Module{
    ModuleTag tag() const override {return BROADCASTER;}
    bool isInit() const override {return true;}
    Signal process(Signal signal, std::string_view /*source*/) override {
        return recordOutput(signal);
    }
};

std::unordered_map<std::string_view, std::shared_ptr<Module>> parseInput(const std::string& input_str){
    std::unordered_map<std::string_view, std::shared_ptr<Module>> all_modules;
    std::unordered_set<std::string_view> conjunctions;
    for (auto line : input_str | views::split('\n')){
        std::string_view line_view(line);
        size_t first_space  = line_view.find(' ');
        size_t second_space = line_view.find(' ', first_space+1);

        char type = line_view.front();
        std::string_view input_label   = line_view.substr(0, first_space);
        std::string_view output_labels = line_view.substr(second_space+1);
        
        switch (type){
            case '%':
                input_label.remove_prefix(1);
                all_modules[input_label] = std::make_shared<FlipFlop>();
                break;
            
            case '&':
                input_label.remove_prefix(1);
                all_modules[input_label] = std::make_shared<Conjunction>();
                conjunctions.insert(input_label);
                break;

            default:
                all_modules[input_label] = std::make_shared<Broadcaster>();
                break;
        }

        for (auto output : output_labels | views::split(", "sv)){
            all_modules[input_label]->outputs.push_back(std::string_view(output));
        }
    }

    for (auto [name, module] : all_modules){
        for (std::string_view output : module->outputs){
            if (conjunctions.contains(output)){
                std::dynamic_pointer_cast<Conjunction>(all_modules.at(output))->last_inputs.insert({name, LOW});
            }
        }
    }

    return all_modules;
}

size_t Module::high_pulses = 0;
size_t Module::low_pulses  = 0;

int main(){
    std::string input_str = loadInput("day_20_data.txt");
    auto start_time = std::chrono::steady_clock::now();

    std::deque<SignalQueueType> signal_queue;
    std::unordered_map<std::string_view, std::shared_ptr<Module>> all_modules = parseInput(input_str);

    auto pressButton = [&all_modules, &signal_queue](std::string source_of_interest = "", Signal signal_of_interest = NONE) -> bool {
        bool signal_of_interest_detected = false;
        
        // Push the button
        Module::low_pulses++; 
        signal_queue.push_back({.source="button", .target="broadcaster", .signal=LOW});
        while (!signal_queue.empty()){
            SignalQueueType signal_package = signal_queue.front();
            signal_queue.pop_front();
            signal_of_interest_detected = signal_of_interest_detected || 
                (signal_package.source == source_of_interest && signal_package.signal == signal_of_interest);

            if (!all_modules.contains(signal_package.target)) continue;

            Module& target_module = *all_modules.at(signal_package.target);
            Signal output_signal  = target_module.process(signal_package.signal, signal_package.source);
            if (output_signal != NONE) {
                for (std::string_view output_label : target_module.outputs){
                    signal_queue.push_back({signal_package.target, output_label, output_signal});
                }
            }
        }
        return signal_of_interest_detected;
    };

    constexpr bool part1 = false;
    if constexpr (part1){
        for (auto _ : views::iota(0, 1000)){
            pressButton();
        }
    }else{
        // Done manually because I didn't want to fully automate this
        std::string source_of_interest = "ln"; // "xp", "gp", "xl"
        Signal signal_of_interest = HIGH; 
        size_t button_press_count = 1;
        for (; !pressButton(source_of_interest, signal_of_interest); button_press_count++)
            ;
        std::println("{} cycles every {} button presses", source_of_interest, button_press_count);
    }

    auto stop_time = std::chrono::steady_clock::now();
    std::println("There were {} low signals and {} high signals for a total of {}", Module::low_pulses, Module::high_pulses, Module::low_pulses*Module::high_pulses);
    std::println("Calculations took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
}
