#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>

using namespace std;
const int maximum_number_of_tries = 16;
const int BITS_SIZE = 24;

// Use std::bitset instead of array<bool> for more efficient bit operations
typedef bitset<BITS_SIZE> BitSet;

// Use an integer directly instead of converting back and forth
using StateId = uint32_t;

// Pre-compute bit masks for fast operations
const StateId LAST_BIT_MASK = 1 << (BITS_SIZE - 1);
const StateId ALL_BITS_MASK = (1 << BITS_SIZE) - 1;

struct State {
  StateId bits;
  int cost;
  vector<int> sequence;

  bool operator>(const State& other) const { return cost > other.cost; }
};

vector<int> greedyOptimalInputSequence(StateId start, StateId target) {
  unordered_set<StateId> visited;
  priority_queue<State, vector<State>, greater<State>> pq;

  pq.push({start, 0, {}});
  visited.insert(start);

  int tries = 0;

  while (!pq.empty() && tries < maximum_number_of_tries) {
    State current = pq.top();
    pq.pop();

    if (current.bits == target) {
      return current.sequence;
    }

    // Process both possible new bits in one loop
    for (int bit : {0, 1}) {
      // Shift left and add new bit in one operation
      StateId newBits = ((current.bits << 1) & ALL_BITS_MASK) | bit;

      if (visited.find(newBits) == visited.end()) {
        visited.insert(newBits);

        vector<int> newInputSequence = current.sequence;
        newInputSequence.push_back(bit);

        // Faster heuristic calculation using XOR and popcount
        int heuristic = __builtin_popcount(newBits ^ target);

        pq.push({newBits, heuristic, newInputSequence});
      }
    }

    tries++;
  }

  // Fallback sequence
  vector<int> fallbackSequence;
  for (int i = 0; i < BITS_SIZE; ++i) {
    fallbackSequence.push_back((target >> i) & 1);
  }
  return fallbackSequence;
}

// Helper function to convert array<bool> to StateId
StateId boolArrayToStateId(const array<bool, BITS_SIZE>& bits) {
  StateId value = 0;
  for (int i = 0; i < BITS_SIZE; ++i) {
    value = (value << 1) | bits[i];
  }
  return value;
}

int main() {
  array<bool, BITS_SIZE> startArray = {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  array<bool, BITS_SIZE> targetArray = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

  // Convert to StateId once at the beginning
  StateId start = boolArrayToStateId(startArray);
  StateId target = boolArrayToStateId(targetArray);

  auto start_time = chrono::high_resolution_clock::now();

  for (int i = 0; i < 1000; ++i) {
    vector<int> inputSequence = greedyOptimalInputSequence(start, target);
  }

  auto end_time = chrono::high_resolution_clock::now();
  chrono::duration<double> elapsed = end_time - start_time;
  double microseconds_per_run = (elapsed.count() * 1e6) / 1000;

  cout << "Elapsed time for 1000 runs: " << elapsed.count() << " seconds"
       << endl;
  cout << "Average time per run: " << microseconds_per_run << " microseconds"
       << endl;

  // Run once more to get the actual sequence
  vector<int> inputSequence = greedyOptimalInputSequence(start, target);
  cout << "Optimal Input Sequence: ";
  for (int bit : inputSequence) {
    cout << bit << " ";
  }
  cout << endl;

  return 0;
}