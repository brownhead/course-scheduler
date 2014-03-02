import heapq

def index_of(indices, width):
    result = 0
    running_product = 1
    for i in indices:
        result += i * running_product
        running_product *= width
    return result

def neighbors(indices):
    # Make indices mutable
    indices = list(indices)

    results = []
    for i in range(len(indices)):
        indices[i] -= 1
        results.append(Solution(indices))

        indices[i] += 2
        results.append(Solution(indices))

        indices[i] -= 1
    return results

def in_range(sets, indices):
    for i, j in zip(sets, indices):
        if not (0 <= j < len(i)):
            return False

    return True

# I'm a bad bad bad person
BAD_SETS = None
class Solution(tuple):
    def value(self):
        return sum(BAD_SETS[i][self[i]] for i in range(len(BAD_SETS)))

    def __lt__(self, other):
        return other.value() < self.value()

def foo(sets, visited, heap):
    global BAD_SETS
    BAD_SETS = sets

    next_solution = heapq.heappop(heap)

    for i in neighbors(next_solution):
        if i not in visited and in_range(sets, i):
            heapq.heappush(heap, i)
            visited.add(i)

    return next_solution

def main():
    sets = [
        [19, 12, 3],
        [33, 22],
        [5, 4, 3, 2, 1, 0]
    ]
    heap = [Solution([0, 0, 0])]
    visited = set(heap)

    solutions = []

    def blah(wah):
        return str(["{}:{}".format(i.value(), i) for i in wah])

    while heap:
        next_solution = foo(sets, visited, heap)
        solutions.append(next_solution)

        print "Solution:", next_solution.value(), next_solution
        print "Past Solutions:", blah(visited)
        print "Heap:", blah(heap)

    print [i.value() for i in solutions]

main()
