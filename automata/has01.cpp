#include <set>
#include <map>

enum class State
{
    Epsilon = 1,
    Met0,
    Met01,
};

template<typename Symbol>
class Dfa01
{
    Dfa01(std::set<State> all_states, 
            State start, 
            std::set<State> finals,
            std::map<State, std::map<Symbol, State>> theta,
            std::set<Symbol> omage);

    bool accept(const std::vector<Symbol>& input);
};

int main()
{
    std::set<State> Q{State::Epsilon, State::Met0, Stat::Met01};
    std::set<State> finals{State::Met01};
    State q0 = State::Epsilon;
    std::map<State, std::map<Symbol, State>> theta;

    Dfa01<char> machine{Q, q0, finals, omega, {'0', '1'}};
    while(std::getline
    
}
