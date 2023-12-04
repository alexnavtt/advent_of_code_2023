#include <print>
#include <ranges>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_set>

using namespace std::string_literals;

uint32_t collectCopies(size_t card_idx, const std::vector<int>& match_counts, std::vector<int>& copies_collected){
    const int num_matches = match_counts[card_idx];
    int& copy_count = copies_collected[card_idx];

    // Cache results
    if (copy_count >= 0){return copy_count;}

    // This card collects copies as long as there are enough cards after it
    copy_count = std::min(num_matches, static_cast<int>(match_counts.size() - card_idx - 1));

    // If not cached, recursively score cards
    size_t max_card_idx = card_idx + copy_count + 1;
    for (size_t new_card_idx = card_idx + 1; new_card_idx < max_card_idx; new_card_idx++){
        copy_count += collectCopies(new_card_idx, match_counts, copies_collected);
    }
    return copy_count;
}

int main(){
    // Convert input to a vector of lines
    std::ifstream text_file{"day_4_data.txt"};
    std::vector<std::string> cards(1);
    while (std::getline(text_file, cards.back())){
        cards.push_back({});
    }
    cards.pop_back();

    uint32_t total_score = 0;
    std::vector<int> card_match_counts(cards.size(), 0);
    for (auto [line_idx, line] : cards | std::views::enumerate){
        std::string_view line_view(line);

        // Remove the card number
        size_t delim = line_view.find(':');
        line_view.remove_prefix(delim+2);

        // Split by the vertical line
        auto sets = line_view | std::views::split(" | "s);
        std::string_view winning_nums{*sets.begin()};
        std::string_view our_numbers{*(std::ranges::next(sets.begin()))};

        // Create sets for both groups of numbers
        std::unordered_set<int> winning_set;
        for (auto string_num : winning_nums | std::views::split(' ')){
            if (string_num.front() == ' ') continue;
            winning_set.insert(std::atoi(string_num.data()));
        }
        std::unordered_set<int> our_num_set;
        for (auto string_num : our_numbers | std::views::split(' ')){
            if (string_num.front() == ' ') continue;
            our_num_set.insert(std::atoi(string_num.data()));
        }

        // Determine the overlap
        uint16_t score = 0x0001;
        for (int val : winning_set){
            if (our_num_set.contains(val)) {
                score <<= 1;
                card_match_counts[line_idx]++;
            }
        }

        // Add on the score we got
        total_score += score >> 1;
    }
    std::println("Total score was {}", total_score);

    // Loop through again and count how many cards we collect
    uint32_t cards_collected = cards.size();
    std::vector<int> copy_collection_counts(cards.size(), -1);
    for (size_t card_idx = 0; card_idx < cards.size(); card_idx++){
        cards_collected += collectCopies(card_idx, card_match_counts, copy_collection_counts);
    }

    std::println("Total collected cards was {}", cards_collected);
}