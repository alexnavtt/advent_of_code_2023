#include <print>
#include <array>
#include <string>
#include <ranges>
#include <chrono>
#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include "load_input.hpp"

namespace views = std::ranges::views;
namespace ranges = std::ranges;

enum PartType : char{
    X = 'x',
    M = 'm',
    A = 'a',
    S = 's'
};

struct Rating{
    std::unordered_map<char, int64_t> data{
        {X, 0},
        {M, 0},
        {A, 0},
        {S, 0},
    };
};

// Range of what a rating can be, inclusive on both ends
struct RatingRange{
    Rating lower{{
        {X, 1},
        {M, 1},
        {A, 1},
        {S, 1},
    }};
    Rating upper{{
        {X, 4000},
        {M, 4000},
        {A, 4000},
        {S, 4000},
    }};
};

struct Rule{
    PartType type;
    std::function<bool(int64_t, int64_t)> comparator;
    int64_t limit;
    std::string destination;
};

struct Workflow{
    std::vector<Rule> rules;
    std::string final;
};

std::unordered_map<std::string_view, Workflow> parseWorkflows(const std::string& input){
    std::unordered_map<std::string_view, Workflow> workflows;
    for (auto workflow_str : input | views::split('\n')){
        if (workflow_str.empty()) break;
        
        // Get the workflow label and its associated rules
        auto sections = workflow_str | views::split('{') | ranges::to<std::vector>();
        std::string_view label{sections[0]};
        std::string_view rules{sections[1]};
        Workflow& workflow = workflows[label];

        // Find the final destination of this workflow
        size_t last_comma = rules.find_last_of(",");
        std::string_view final_destination = rules.substr(last_comma + 1);
        final_destination.remove_suffix(1);
        workflow.final = std::string(final_destination);

        // Assign the rules
        rules = rules.substr(0, last_comma);
        for (auto rule_str : rules | views::split(',') | views::transform([](auto rule){return std::string_view(rule);})){
            Rule new_rule;
            new_rule.type  = static_cast<PartType>(rule_str[0]);
            new_rule.limit = std::atoll(rule_str.substr(2).data());
            new_rule.destination = std::string(rule_str.substr(rule_str.find(':') + 1));
            if (rule_str[1] == '>'){
                new_rule.comparator = std::greater<int64_t>{};
            }else{
                new_rule.comparator = std::less<int64_t>{};
            }
            workflow.rules.push_back(new_rule);
        } 
    }

    return workflows;
}

std::vector<Rating> parseRatings(std::string& input){
    std::vector<Rating> ratings;
    for (auto line : input | views::split('\n') | views::drop_while([](auto line){return !line.empty();}) | views::drop(1)){
        std::string_view rating(line);
        rating.remove_prefix(1);
        rating.remove_suffix(1);

        Rating& new_rating = ratings.emplace_back();
        for (auto chunk : rating | views::split(',')){
            new_rating.data.at(chunk.front()) = std::atoll((chunk | views::drop(2)).data());
        }
    }

    return ratings;
}

size_t processWorkflow(std::unordered_multimap<std::string, RatingRange>& result, const std::unordered_map<std::string_view, Workflow>& workflows){

    size_t total = 0;
    auto storeRange = [&total, &result](const std::string& destination, const RatingRange& range){
        if (destination == "A"){
            total += (range.upper.data.at(X) - range.lower.data.at(X) + 1)
                   * (range.upper.data.at(M) - range.lower.data.at(M) + 1)
                   * (range.upper.data.at(A) - range.lower.data.at(A) + 1)
                   * (range.upper.data.at(S) - range.lower.data.at(S) + 1);
        }else if (destination != "R"){
            result.insert({destination, range});
        }
    };

    // Pop the fist element in the results range
    auto it = result.begin();
    RatingRange old_range = it->second;
    const Workflow& workflow = workflows.at(it->first);
    result.erase(it);

    // Run the range through its assigned workflow and split it as necessary for each rule
    for (const Rule& rule : workflow.rules){
        int64_t& old_lower_bound = old_range.lower.data.at(rule.type);
        int64_t& old_upper_bound = old_range.upper.data.at(rule.type);

        // If both ends of the range give the same result, then we don't need to split it
        bool lower_works = std::invoke(rule.comparator, old_lower_bound, rule.limit);
        bool upper_works = std::invoke(rule.comparator, old_upper_bound, rule.limit);
        if (!(lower_works ^ upper_works)) continue;

        RatingRange new_range = old_range;
        int64_t& new_lower_bound = new_range.lower.data.at(rule.type);
        int64_t& new_upper_bound = new_range.upper.data.at(rule.type);

        if (lower_works && !upper_works){
            old_lower_bound = rule.limit;
            new_upper_bound = rule.limit-1;
        }else if(upper_works && !lower_works){
            new_lower_bound = rule.limit+1;
            old_upper_bound = rule.limit;
        }
        storeRange(rule.destination, new_range);
    }

    // Whatever's left also needs to be reinserted into the list of ranges
    storeRange(workflow.final, old_range);

    return total;
}

int main(){
    std::string input_str = loadInput("day_19_data.txt");
    const auto start_time = std::chrono::steady_clock::now();

    auto workflows = parseWorkflows(input_str);
    auto ratings   = parseRatings(input_str);

    // Part 1
    size_t total = 0;
    for (const Rating& rating : ratings){
        std::string next = "in";
        while(next != "A" & next != "R"){
            Workflow& workflow = workflows.at(next);
            bool path_found = false;
            for (const Rule& rule : workflow.rules){
                if (std::invoke(rule.comparator, rating.data.at(rule.type), rule.limit)){
                    next = rule.destination;
                    path_found = true;
                    break;
                }
            }
            if (!path_found) next = workflow.final;
        }

        if (next == "A"){
            total += ranges::fold_left(rating.data | views::values, 0, std::plus{});
        }
    }

    // Part 2
    std::unordered_multimap<std::string, RatingRange> all_ranges{{"in", RatingRange{}}};
    size_t total_combos = 0;
    while (!all_ranges.empty()){
        total_combos += processWorkflow(all_ranges, workflows);
    }

    const auto stop_time = std::chrono::steady_clock::now();
    std::println("Total is {}", total);
    std::println("Total number of combinations is {}", total_combos);
    std::println("Calculations took {} microseconds", std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
}