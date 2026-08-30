

#include "algorithm.h"

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace std;


namespace {

struct UniformCostNode {
    string state;
    string path;
    string parentState;
    int gCost;
    size_t insertionOrder;
};

// std::make_heap normally creates a max heap.  Reversing the comparisons
// puts the lowest-g node at the front.  The insertion order makes ties
// deterministic and preserves the required Up, Right, Down, Left order.
struct UniformCostMinHeapComparator {
    bool operator()(const UniformCostNode &left,
                    const UniformCostNode &right) const {
        if (left.gCost != right.gCost) {
            return left.gCost > right.gCost;
        }
        return left.insertionOrder > right.insertionOrder;
    }
};

struct AStarNode {
    string state;
    string path;
    string parentState;
    int gCost;
    int hCost;
    int fCost;
    size_t insertionOrder;
};

struct AStarMinHeapComparator {
    bool operator()(const AStarNode &left, const AStarNode &right) const {
        if (left.fCost != right.fCost) {
            return left.fCost > right.fCost;
        }

        // For equal f-costs, a larger g-cost has higher priority.
        if (left.gCost != right.gCost) {
            return left.gCost < right.gCost;
        }

        // Keep otherwise identical priorities deterministic and honour the
        // required Up, Right, Down, Left insertion order.
        return left.insertionOrder > right.insertionOrder;
    }
};

bool isValidPuzzleState(const string &state) {
    if (state.size() != 9) {
        return false;
    }

    bool seen[9] = {false};
    for (size_t i = 0; i < state.size(); ++i) {
        if (state[i] < '0' || state[i] > '8') {
            return false;
        }

        const int tile = state[i] - '0';
        if (seen[tile]) {
            return false;
        }
        seen[tile] = true;
    }
    return true;
}

int calculateHeuristic(const string &state,
                       const string &goalState,
                       const int goalPositions[9],
                       heuristicFunction heuristic) {
    int estimate = 0;

    if (heuristic == misplacedTiles) {
        for (size_t i = 0; i < state.size(); ++i) {
            if (state[i] != '0' && state[i] != goalState[i]) {
                ++estimate;
            }
        }
        return estimate;
    }

    for (size_t i = 0; i < state.size(); ++i) {
        if (state[i] == '0') {
            continue;
        }

        const int goalPosition = goalPositions[state[i] - '0'];
        const int row = static_cast<int>(i / 3);
        const int column = static_cast<int>(i % 3);
        const int goalRow = goalPosition / 3;
        const int goalColumn = goalPosition % 3;
        estimate += abs(row - goalRow) + abs(column - goalColumn);
    }

    return estimate;
}

} // namespace





///////////////////////////////////////////////////////////////////////////////////////////
//
// Search Algorithm:  UC with Strict Expanded List
//
// Move Generator:  
//
////////////////////////////////////////////////////////////////////////////////////////////
string uc_explist(string const initialState, string const goalState, int& pathLength, int &numOfStateExpansions, int& maxQLength,
                               float &actualRunningTime, int &numOfDeletionsFromMiddleOfHeap, int &numOfLocalLoopsAvoided, int &numOfAttemptedNodeReExpansions){
	string solutionPath;
	const clock_t startTime = clock();

	pathLength = 0;
	numOfStateExpansions = 0;
	maxQLength = 0;
	actualRunningTime = 0.0f;
	numOfDeletionsFromMiddleOfHeap = 0;
	numOfLocalLoopsAvoided = 0;
	numOfAttemptedNodeReExpansions = 0;

	if (!isValidPuzzleState(initialState) ||
	    !isValidPuzzleState(goalState)) {
		actualRunningTime = static_cast<float>(clock() - startTime) /
		                    CLOCKS_PER_SEC;
		return solutionPath;
	}

	UniformCostMinHeapComparator compareNodes;
	vector<UniformCostNode> q;
	q.push_back(UniformCostNode{initialState, "", "", 0, 0});
	make_heap(q.begin(), q.end(), compareNodes);
	maxQLength = 1;

	unordered_set<string> expanded;
	unordered_map<string, int> queuedCosts;
	expanded.reserve(181440);
	queuedCosts.reserve(50000);
	queuedCosts[initialState] = 0;
	size_t nextInsertionOrder = 1;

	while (!q.empty()) {
		pop_heap(q.begin(), q.end(), compareNodes);
		UniformCostNode current = std::move(q.back());
		q.pop_back();
		queuedCosts.erase(current.state);

		// This guard is normally made redundant by the checks performed when
		// successors are generated, but it keeps the expanded list strictly
		// enforced if a stale duplicate is ever present in Q.
		if (expanded.find(current.state) != expanded.end()) {
			++numOfAttemptedNodeReExpansions;
			continue;
		}

		if (current.state == goalState) {
			solutionPath = current.path;
			break;
		}

		expanded.insert(current.state);
		++numOfStateExpansions;

		const size_t blank = current.state.find('0');
		const int row = static_cast<int>(blank / 3);
		const int column = static_cast<int>(blank % 3);

		// Required successor/pushing order: Up, Right, Down, Left.
		const int rowChange[4] = {-1, 0, 1, 0};
		const int columnChange[4] = {0, 1, 0, -1};
		const char moveName[4] = {'U', 'R', 'D', 'L'};

		for (int move = 0; move < 4; ++move) {
			const int nextRow = row + rowChange[move];
			const int nextColumn = column + columnChange[move];
			if (nextRow < 0 || nextRow >= 3 ||
			    nextColumn < 0 || nextColumn >= 3) {
				continue;
			}

			string successorState = current.state;
			const size_t nextBlank =
				static_cast<size_t>(nextRow * 3 + nextColumn);
			swap(successorState[blank], successorState[nextBlank]);

			// Do not immediately undo the move that produced the current node.
			if (!current.parentState.empty() &&
			    successorState == current.parentState) {
				++numOfLocalLoopsAvoided;
				continue;
			}

			// A strict expanded list never reopens an expanded state.
			if (expanded.find(successorState) != expanded.end()) {
				++numOfAttemptedNodeReExpansions;
				continue;
			}

			const int successorGCost = current.gCost + 1;
			unordered_map<string, int>::iterator queued =
				queuedCosts.find(successorState);

			if (queued != queuedCosts.end()) {
				// Keep the shortest path to each state currently in Q.  An
				// equal-cost duplicate is discarded, leaving the earlier URDL
				// insertion in place.
				if (queued->second <= successorGCost) {
					continue;
				}

				vector<UniformCostNode>::iterator duplicate =
					find_if(q.begin(), q.end(),
					        [&successorState](const UniformCostNode &node) {
						        return node.state == successorState;
					        });
				q.erase(duplicate);
				make_heap(q.begin(), q.end(), compareNodes);
				++numOfDeletionsFromMiddleOfHeap;
			}

			q.push_back(UniformCostNode{
				successorState,
				current.path + moveName[move],
				current.state,
				successorGCost,
				nextInsertionOrder++
			});
			queuedCosts[successorState] = successorGCost;
			push_heap(q.begin(), q.end(), compareNodes);

			if (static_cast<int>(q.size()) > maxQLength) {
				maxQLength = static_cast<int>(q.size());
			}
		}
	}

	pathLength = static_cast<int>(solutionPath.size());
	actualRunningTime = static_cast<float>(clock() - startTime) /
	                    CLOCKS_PER_SEC;
	return solutionPath;
		
}




///////////////////////////////////////////////////////////////////////////////////////////
//
// Search Algorithm:  A* with the Strict Expanded List
//
// Move Generator:  
//
////////////////////////////////////////////////////////////////////////////////////////////
string aStar_ExpandedList(string const initialState, string const goalState, int& pathLength, int &numOfStateExpansions, int& maxQLength,
                               float &actualRunningTime, int &numOfDeletionsFromMiddleOfHeap, int &numOfLocalLoopsAvoided, int &numOfAttemptedNodeReExpansions, heuristicFunction heuristic){
	string solutionPath;
	const clock_t startTime = clock();

	pathLength = 0;
	numOfStateExpansions = 0;
	maxQLength = 0;
	actualRunningTime = 0.0f;
	numOfDeletionsFromMiddleOfHeap = 0;
	numOfLocalLoopsAvoided = 0;
	numOfAttemptedNodeReExpansions = 0;

	if (!isValidPuzzleState(initialState) ||
	    !isValidPuzzleState(goalState) ||
	    (heuristic != misplacedTiles && heuristic != manhattanDistance)) {
		actualRunningTime = static_cast<float>(clock() - startTime) /
		                    CLOCKS_PER_SEC;
		return solutionPath;
	}

	int goalPositions[9];
	for (size_t i = 0; i < goalState.size(); ++i) {
		goalPositions[goalState[i] - '0'] = static_cast<int>(i);
	}

	AStarMinHeapComparator compareNodes;
	vector<AStarNode> q;
	const int initialHCost =
		calculateHeuristic(initialState, goalState, goalPositions, heuristic);
	q.push_back(AStarNode{
		initialState, "", "", 0, initialHCost, initialHCost, 0
	});
	make_heap(q.begin(), q.end(), compareNodes);
	maxQLength = 1;

	unordered_set<string> expanded;
	unordered_map<string, int> queuedCosts;
	expanded.reserve(181440);
	queuedCosts.reserve(50000);
	queuedCosts[initialState] = 0;
	size_t nextInsertionOrder = 1;

	while (!q.empty()) {
		pop_heap(q.begin(), q.end(), compareNodes);
		AStarNode current = std::move(q.back());
		q.pop_back();
		queuedCosts.erase(current.state);

		if (expanded.find(current.state) != expanded.end()) {
			++numOfAttemptedNodeReExpansions;
			continue;
		}

		if (current.state == goalState) {
			solutionPath = current.path;
			break;
		}

		expanded.insert(current.state);
		++numOfStateExpansions;

		const size_t blank = current.state.find('0');
		const int row = static_cast<int>(blank / 3);
		const int column = static_cast<int>(blank % 3);

		// Required successor/pushing order: Up, Right, Down, Left.
		const int rowChange[4] = {-1, 0, 1, 0};
		const int columnChange[4] = {0, 1, 0, -1};
		const char moveName[4] = {'U', 'R', 'D', 'L'};

		for (int move = 0; move < 4; ++move) {
			const int nextRow = row + rowChange[move];
			const int nextColumn = column + columnChange[move];
			if (nextRow < 0 || nextRow >= 3 ||
			    nextColumn < 0 || nextColumn >= 3) {
				continue;
			}

			string successorState = current.state;
			const size_t nextBlank =
				static_cast<size_t>(nextRow * 3 + nextColumn);
			swap(successorState[blank], successorState[nextBlank]);

			if (!current.parentState.empty() &&
			    successorState == current.parentState) {
				++numOfLocalLoopsAvoided;
				continue;
			}

			if (expanded.find(successorState) != expanded.end()) {
				++numOfAttemptedNodeReExpansions;
				continue;
			}

			const int successorGCost = current.gCost + 1;
			unordered_map<string, int>::iterator queued =
				queuedCosts.find(successorState);

			if (queued != queuedCosts.end()) {
				if (queued->second <= successorGCost) {
					continue;
				}

				vector<AStarNode>::iterator duplicate =
					find_if(q.begin(), q.end(),
					        [&successorState](const AStarNode &node) {
						        return node.state == successorState;
					        });
				q.erase(duplicate);
				make_heap(q.begin(), q.end(), compareNodes);
				++numOfDeletionsFromMiddleOfHeap;
			}

			const int successorHCost = calculateHeuristic(
				successorState, goalState, goalPositions, heuristic);
			q.push_back(AStarNode{
				successorState,
				current.path + moveName[move],
				current.state,
				successorGCost,
				successorHCost,
				successorGCost + successorHCost,
				nextInsertionOrder++
			});
			queuedCosts[successorState] = successorGCost;
			push_heap(q.begin(), q.end(), compareNodes);

			if (static_cast<int>(q.size()) > maxQLength) {
				maxQLength = static_cast<int>(q.size());
			}
		}
	}

	pathLength = static_cast<int>(solutionPath.size());
	actualRunningTime = static_cast<float>(clock() - startTime) /
	                    CLOCKS_PER_SEC;
	return solutionPath;
		
}

