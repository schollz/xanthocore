#include <array>
#include <chrono>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
using namespace std;

const int maximum_number_of_tries = 16;

typedef array<bool, 24> BitArray;

inline int toInt(const BitArray& bits) {
  int value = 0;
  for (int i = 0; i < 24; ++i) {
    value = (value << 1) | bits[i];
  }
  return value;
}

struct State {
  BitArray bits;
  int cost;
  vector<int> sequence;

  bool operator>(const State& other) const { return cost > other.cost; }
};

vector<int> greedyOptimalInputSequence(const BitArray& start,
                                       const BitArray& target) {
  unordered_set<int> visited;
  priority_queue<State, vector<State>, greater<State>> pq;

  int startValue = toInt(start);
  int targetValue = toInt(target);
  pq.push({start, 0, {}});
  visited.insert(startValue);

  int tries = 0;
  while (!pq.empty() && tries < maximum_number_of_tries) {
    State current = pq.top();
    pq.pop();

    if (current.bits == target) {
      return current.sequence;
    }

    for (int bit : {0, 1}) {
      BitArray newSequence = current.bits;
      for (int i = 0; i < 23; ++i) {
        newSequence[i] = newSequence[i + 1];
      }
      newSequence[23] = bit;

      int newValue = toInt(newSequence);
      if (visited.find(newValue) == visited.end()) {
        visited.insert(newValue);
        vector<int> newInputSequence = current.sequence;
        newInputSequence.push_back(bit);

        int heuristic = 0;
        for (int i = 0; i < 24; ++i) {
          heuristic += (newSequence[i] != target[i]);
        }

        pq.push({newSequence, heuristic, newInputSequence});
      }
    }
    tries++;
  }

  vector<int> fallbackSequence;
  for (int i = 0; i < 24; ++i) {
    fallbackSequence.push_back(target[i]);
  }
  return fallbackSequence;
}
int main() {
  BitArray start = {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  BitArray target = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

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
