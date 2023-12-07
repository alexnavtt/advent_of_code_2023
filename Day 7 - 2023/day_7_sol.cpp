#include <print>
#include <array>
#include <deque>
#include <string>
#include <ranges>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <spanstream>

constexpr std::string filename{"day_7_data.txt"};

struct Hand{
    std::array<int, 5> cards;
    int bid;
};

// Mapping of labels
static const std::unordered_map<char, int> card_map{
    {'1', 1},
    {'2', 2},
    {'3', 3},
    {'4', 4},
    {'5', 5},
    {'6', 6},
    {'7', 7},
    {'8', 8},
    {'9', 9},
    {'T', 10},
    {'J', 11},
    {'Q', 12},
    {'K', 13},
    {'A', 14},
};

// Labels for scoring
enum HandLabel : int {
    HIGH_CARD,
    ONE_PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    FIVE_OF_A_KIND
};

// State of a card after being scored
struct ScoredHand{
    HandLabel label;
    Hand hand;
};

// Score a hand
ScoredHand scoreHand(Hand hand){
    // Sort the cards for scoring
    auto sorted_cards = hand.cards;
    std::ranges::sort(sorted_cards);
    HandLabel label{HIGH_CARD};

    // Score based on the number of card groups in the hand
    auto sets = sorted_cards | std::views::chunk_by([](int card1, int card2){return card1 == card2;});
    switch(std::ranges::distance(sets)){
        case 1:
            label = FIVE_OF_A_KIND;
            break;

        case 2:{
            std::ranges::subrange set1 = sets.front();
            if (set1.size() == 4 || set1.size() == 1){
                label = FOUR_OF_A_KIND;
            }else{
                label = FULL_HOUSE;
            }
            break;
        }

        case 3:{
            std::ranges::subrange set1 = *std::ranges::next(sets.begin(), 0);
            std::ranges::subrange set2 = *std::ranges::next(sets.begin(), 1);
            if (set1.size() == 2 || set2.size() == 2){
                label = TWO_PAIR;
            }else{
                label = THREE_OF_A_KIND;
            }
            break;
        }

        case 4: 
            label = ONE_PAIR;
            break;

        default:
            break;
    }

    return ScoredHand{.label = label, .hand = hand};
}

// Compare scored hands for sorting
bool scoreLess(const ScoredHand& h1, const ScoredHand& h2){
    if (h1.label == h2.label) {
        for (auto [card1, card2] : std::views::zip(h1.hand.cards, h2.hand.cards)){
            if (card1 == card2) continue;
            return card1 < card2;
        }
    }
    return h1.label < h2.label;
}

// Problem 2 - Rescore a hand based on the new rules
ScoredHand rescore(const ScoredHand& scored_hand){
    // Replace Jacks with Jokers
    ScoredHand new_hand = scored_hand;
    int num_jokers = 0;
    std::for_each(new_hand.hand.cards.begin(), new_hand.hand.cards.end(), 
        [&num_jokers](int& card){if (card == 11) {num_jokers++; card = 1;}});
    
    // If no jokers then we don't have to do anything
    if (num_jokers == 0) return new_hand;

    // Rescore the hand    
    switch (scored_hand.label){
        case HIGH_CARD:
            if (num_jokers == 1) new_hand.label = ONE_PAIR;
            break;

        case ONE_PAIR:
            if (num_jokers != 0) new_hand.label = THREE_OF_A_KIND;
            break;

        case TWO_PAIR:
            if (num_jokers == 1) new_hand.label = FULL_HOUSE;
            else if (num_jokers == 2) new_hand.label = FOUR_OF_A_KIND;
            break;

        case THREE_OF_A_KIND:
            if (num_jokers != 0) new_hand.label = FOUR_OF_A_KIND;
            break;

        case FULL_HOUSE:
        case FOUR_OF_A_KIND:
            if (num_jokers != 0) new_hand.label = FIVE_OF_A_KIND;
            break;
    }

    return new_hand;
}

int main(){
    std::ifstream text_file{filename};

    // Extract the hands and bids
    std::deque<Hand> hands;
    for (std::string line; std::getline(text_file, line);){
        auto cards = line
            | std::views::take(5)
            | std::views::transform([](char c){return card_map.at(c);});
        auto bid = line | std::views::drop(6);

        hands.push_back(Hand{
            .cards = {cards[0], cards[1], cards[2], cards[3], cards[4]},
            .bid = std::atoi(bid.data())
        });
    }

    // Score the hands
    std::vector<ScoredHand> scored_hands = hands 
        | std::views::transform([](Hand& h){return scoreHand(h);})
        | std::ranges::to<std::vector<ScoredHand>>();

    // Sort the scored hands
    std::ranges::sort(scored_hands.begin(), scored_hands.end(), scoreLess);
    std::println("Sorted hands: ");

    // Determine the final answer
    long long winnings = 0;
    for (auto [rank, sorted_hand] : std::views::enumerate(scored_hands)){
        winnings += (rank+1) * sorted_hand.hand.bid;
    }
    std::println("Initial winnings are ${}", winnings);

    // Problem 2
    std::println("Adding the joker rule and recalculating...");
    std::vector<ScoredHand> scored_with_new_rules = scored_hands
        | std::views::transform(rescore)
        | std::ranges::to<std::vector<ScoredHand>>();

    // Resort with the new values
    std::ranges::sort(scored_with_new_rules.begin(), scored_with_new_rules.end(), scoreLess);

    // Recalculate the winnings
    long long new_winnings = 0;
    for (auto [rank, sorted_hand] : std::views::enumerate(scored_with_new_rules)){
        for (auto elem : sorted_hand.hand.cards){
            std::print("{: <2}, ", elem);
        }
        std::println(" : {} - Rank {}", (int)sorted_hand.label, rank+1);
        new_winnings += (rank+1) * sorted_hand.hand.bid;
    }
    std::println("Final winnings are ${}", new_winnings);

    return 0;
}