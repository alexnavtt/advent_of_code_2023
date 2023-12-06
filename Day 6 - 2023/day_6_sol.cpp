#include <print>
#include <string>
#include <ranges>
#include <vector>
#include <fstream>

bool isDigit(char c){
    return c >= '0' && c <= '9';
}

long long holdTimeToWin(long long dist, long long total_time){
    double root = std::sqrt(total_time*total_time - 4*dist);
    return std::floor(0.5*(total_time - root)) + 1;
}

long long numWinningSolutions(long long dist, long long total_time){
    long long breakpoint = holdTimeToWin(dist, total_time) - 1;
    return total_time - 2*breakpoint - 1; // -1 since we count zero as well
}

int main(){
    // Read the input into a string
    std::ifstream text_file{"day_6_data.txt"};

    // Parse the data to get the inputs
    auto parseNextLine = [&text_file]() -> std::vector<int> {
        std::string str;
        std::getline(text_file, str);
        return str 
            | std::views::drop_while([](char c){return !isDigit(c);})
            | std::views::chunk_by([](char c1, char c2){return !(isDigit(c1) ^ isDigit(c2));})
            | std::views::stride(2)
            | std::views::transform([](auto chunk){return std::atoi(chunk.data());})
            | std::ranges::to<std::vector<int>>();
    };
    std::vector<int> time_vals = parseNextLine();
    std::vector<int> dist_vals = parseNextLine();

    // Solve the problem
    long long product = 1;
    for (auto [time, dist] : std::views::zip(time_vals, dist_vals)){
        product *= numWinningSolutions(dist, time);
    }
    std::println("The total product was {}", product);

    // Reparse the problem 2 version
    auto reparse = [](const std::vector<int>& v) -> long long {
        std::string as_one_int = v
            | std::views::transform([](int i){return std::to_string(i);})
            | std::views::join
            | std::ranges::to<std::string>();
        return std::stoll(as_one_int);
    };
    long long big_dist = reparse(dist_vals);
    long long big_time = reparse(time_vals);
    
    std::println("Winning solutions: {}", numWinningSolutions(big_dist, big_time));

    return 0;
}