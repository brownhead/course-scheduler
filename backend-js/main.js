function ScoreMatrix(headers) {
    this.headers = headers.slice();
    for (var i = 0; i < this.headers.length; ++i) {
        this.headers[i].sort(function (a, b) { return a < b; });
    }
}

ScoreMatrix.prototype = {
    get_score: function (position) {
        var sum = 0;
        for (var i = 0; i < this.headers.length; ++i) {
            sum += this.headers[i][position[i]];
        }
        return sum;
    }
}

// Stores positions in a score_matrix
function PositionHeap(score_matrix) {
    this.score_matrix = score_matrix;
    this.data = [];
}

PositionHeap.prototype = {
    push: function (position) {
        var score_matrix = this.score_matrix;
        var compare = function (a, b) {
            return score_matrix.get_score(a) > score_matrix.get_score(b);
        }
        this.data.push(position);
        this.data.sort(compare);
    },

    pop: function () {
        return this.data.pop();
    },

    empty: function() {
        return this.data.length == 0;
    }
}

function PositionSet(score_matrix) {
    this.score_matrix = score_matrix;
    this.data = {};
}

PositionSet.prototype = {
    add: function (position) {
        this.data[position.join()] = 1
    },

    contains: function (position) {
        return position.join() in this.data;
    },

    empty: function (position) {
        return Object.keys(this.data).length == 0
    }
}

function foo(score_matrix, neighbor_heap, visited_set) {
    // If we haven't been called before...
    if (neighbor_heap.empty() && visited_set.empty()) {
        position = []
        for (var i = 0; i < score_matrix.headers.length; ++i) {
            position.push(0)
        }
        neighbor_heap.push(position);
    } else if (neighbor_heap.empty()) {
        // If we've exhausted all results
        return null;
    }

    // Whatevers at the top of the heap is the next biggest solution
    var solution = neighbor_heap.pop()

    for (var i = 0; i < score_matrix.headers.length; ++i) {
        for (var j = 0; j < 2; ++j) {
            if (j == 0) {
                solution[i] -= 1;
            } else {
                solution[i] += 2;
            }

            if (solution[i] >= 0 &&
                    solution[i] < score_matrix.headers[i].length) {
                if (!visited_set.contains(solution)) {
                    neighbor_heap.push(solution.slice());
                    visited_set.add(solution.slice());
                }
            }
        }

        // Set the current indice back to normal
        solution[i] -= 1;
    }

    return solution;
}

var m = new ScoreMatrix([
    [5, 4, 3, 2, 1],
    [3, 10],
    [11, 5, 0]
]);

var some_positions = [
    [1, 1, 1],
    [0, 0, 0],
    [4, 0, 1],
    [2, 1, 2],
    [1, 0, 0]
];

var h = new PositionHeap(m);
var s = new PositionSet(m);

while (true) {
    var x = foo(m, h, s);
    if (x === null) {
        break;
    }

    console.log(x + " " + m.get_score(x))
}

// for (var i = 0; i < some_positions.length; ++i) {
//     h.push(some_positions[i])
// }
// for (var i = 0; i < some_positions.length; ++i) {
//     var x = h.pop();
//     console.log(x + " " + m.get_score(x));
// }
