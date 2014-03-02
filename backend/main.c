
// #include <fcgi_stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
    /// A flat 2-dimensional array of conformance lists
    unsigned int * lists;

    /// The szie of each array in lists
    unsigned int * lengths;

    /// The number of lists
    unsigned int nlists;
} Conformances;

int in_range(Conformances const * conformances, unsigned int * indices) {
    unsigned int i;
    for (i = 0; i < conformances->nlists; ++i) {
        if (indices[i] >= conformances->lengths[i]) {
            return 0;
        }
    }

    return 1;
}

unsigned int get_conformance(Conformances const * conformances,
        unsigned int const * indices) {
    // Our result, the sum of each individual conformance value
    unsigned int sum = 0;

    // We'll keep track of where we are in the flat 2-dimensional array
    unsigned int cur_list_start = 0;

    unsigned int i;
    for (i = 0; i < conformances->nlists; ++i) {
        sum += conformances->lists[cur_list_start + indices[i]];
        cur_list_start += conformances->lengths[i];
    }

    return sum;
}

typedef struct {
    /// The actual heap data
    unsigned int * data;

    /// The number of items in the heap
    unsigned int length;

    /// The number of items the heap can store without reallocating. The actual
    /// number of allocated bytes is
    /// capacity * sizeof(unsigned int) * ndimensions
    unsigned int capacity;

    /// The number of dimensions in each indice
    unsigned int ndimensions;
} IndicesHeap;

int heap_add(IndicesHeap * heap, Conformances const * conformances,
        unsigned int const * new_indices) {
    assert(heap->ndimensions == conformances->nlists);

    // Check if we have any room to grow. We don't resize the array here.
    if (heap->length >= heap->capacity) {
        return 0;
    }

    // The size of a single tuple of indices in bytes
    unsigned int indices_size = heap->ndimensions * sizeof(unsigned int);

    // Add the new indices to the back of the heap
    memcpy(heap->data + heap->length * heap->ndimensions, new_indices,
        indices_size);
    ++heap->length;

    // Get the conformance value of the new indices so we don't have to compute
    // it every time below.
    unsigned int cur_conformance = get_conformance(conformances, new_indices);

    unsigned int i = heap->length - 1;
    while (i > 0) {
        unsigned int parent_index = (i - 1) / 2;
        unsigned int * parent = heap->data + parent_index * heap->ndimensions;

        // Get the conformance value of the parent
        unsigned int parent_conformance = get_conformance(conformances,
            parent);

        // If the value doesn't need to be sifted up any further we're done
        if (cur_conformance <= parent_conformance) {
            break;
        }

        // Swap the two values
        unsigned int * cur = heap->data + i * heap->ndimensions;
        memcpy(cur, parent, indices_size);
        memcpy(parent, new_indices, indices_size);

        i = parent_index;
    }

    return 1;
}

int heap_pop(IndicesHeap * heap, Conformances const * conformances,
        unsigned int * result) {
    // Make sure that the heap isn't already empty
    if (heap->length <= 0) {
        return 0;
    }

    // The size of a single tuple of indices in bytes
    unsigned int indices_size = heap->ndimensions * sizeof(unsigned int);

    // Copy the top of the heap into result
    memcpy(result, heap->data, indices_size);

    // Check to see if we need to do anything further
    --heap->length;
    if (heap->length <= 0) {
        return 1;
    }

    // Copy the bottom of the heap into the top
    memcpy(heap->data, heap->data + heap->length * heap->ndimensions,
        indices_size);

    // Also store the values in a buffer to help us with swapping later
    unsigned int buffer[heap->ndimensions];
    memcpy(buffer, heap->data, indices_size);

    // Get the conformance value of the indices we're trickling down
    unsigned int cur_conformance = get_conformance(conformances, heap->data);

    unsigned int i = 0;
    while (i < heap->length - 1) {
        unsigned int * swap_with = NULL;
        unsigned int swap_with_index = 0;

        // Figure out the index of the left and right child
        unsigned int left_child_index = 2 * i + 1;
        unsigned int right_child_index = 2 * i + 2;

        // Determine whether we need to swap with the left or right child
        if (left_child_index < heap->length) {
            unsigned int * left_child =
                heap->data + left_child_index * heap->ndimensions;
            unsigned int left_child_conformance = get_conformance(conformances,
                left_child);
            if (cur_conformance < left_child_conformance) {
                swap_with = left_child;
                swap_with_index = left_child_index;
            }

            if (right_child_index < heap->length) {
                unsigned int * right_child =
                    heap->data + right_child_index * heap->ndimensions;
                unsigned int right_child_conformance = get_conformance(
                    conformances, right_child);
                if (left_child_conformance < right_child_conformance) {
                    swap_with = right_child;
                    swap_with_index = right_child_index;
                }
            }
        }

        // If the value doesn't need to be trickled down any further we're done
        if (swap_with == NULL) {
            break;
        }

        // Swap the two values
        unsigned int * cur = heap->data + i * heap->ndimensions;
        memcpy(cur, swap_with, indices_size);
        memcpy(swap_with, buffer, indices_size);

        i = swap_with_index;
    }

    return 1;
}

typedef struct {
    unsigned int * data;
    unsigned int length;
    unsigned int capacity;
    unsigned int ndimensions;
} VisitedSet;

int set_add(VisitedSet * set, unsigned int const * indices) {
    if (set->length >= set->capacity) {
        return 0;
    }

    // The size of a single tuple of indices in bytes
    unsigned int indices_size = set->ndimensions * sizeof(unsigned int);

    // Add the item onto the back of the "set"
    memcpy(set->data + set->length * set->ndimensions, indices, indices_size);
    ++set->length;

    return 1;
}

int set_in(VisitedSet const * set, unsigned int const * indices) {
    unsigned int i, j;
    for (i = 0; i < set->length; ++i) {
        for (j = 0; j < set->ndimensions; ++j) {
            // If we found a mismatch we know this tuple isn't it
            if (set->data[i * set->ndimensions + j] != indices[j]) {
                goto next_indices;
            }
        }

        // A tuple completely matched, we're done
        return 1;
    next_indices:;
    }

    return 0;
}

int set_clear(VisitedSet * set) {
    set->length = 0;
    return 1;
}

int foo(Conformances const * conformances, IndicesHeap * heap,
        VisitedSet * visited, unsigned int * solution) {
    // Grab the next solution
    if (!heap_pop(heap, conformances, solution)) {
        return 0;
    }

    // We can clear the visited set if we've found a different value
    // unsigned int solution_conformance = get_conformance(conformances,
    //     solution);
    // if (visited->length != 0) {
    //     unsigned int visited_conformance = get_conformance(conformances,
    //         visited->data);
    //     if (visited->length != 0 &&
    //             solution_conformance != visited_conformance) {
    //         visited_conformance = solution_conformance;
    //         set_clear(visited);
    //     }
    // }

    // Add all of the solutions neighbor's to the heap
    unsigned int i, j;
    for (i = 0; i < heap->ndimensions; ++i) {
        for (j = 0; j < 2; ++j) {
            if (j == 0) {
                solution[i] -= 1;
            } else {
                solution[i] += 2;
            }

            if (in_range(conformances, solution)) {
                //unsigned int cur_conformance = get_conformance(conformances,
                 //   solution);

                if (//cur_conformance <= solution_conformance &&
                        !set_in(visited, solution)) {
                    // Add the neighbor to the heaps
                    if (!heap_add(heap, conformances, solution)) {
                        printf("heap explode");
                        return 0;
                    }

                    // Add the neighbor to the visited set
                    if (!set_add(visited, solution)) {
                        printf("visited explode");
                        return 0;
                    }
                }
            }
        }

        // Set the current indice back to normal
        solution[i] -= 1;
    }

    return 1;
}

int decode_hex_grid(char const * data, unsigned int data_size,
        unsigned int * result) {
    if (data_size % 2 != 0) {
        return 0;
    }

    unsigned int i, j, k;
    for (i = 0, j = 0; i < data_size; i += 2, ++j) {
        unsigned int numeral[2];
        for (k = 0; k < 2; ++k) {
            if (data[i + k] >= '0' && data[i + k] <= '9') {
                numeral[k] = data[i + k] - '0';
            } else if (data[i + k] >= 'A' && data[i + k] <= 'F') {
                numeral[k] = data[i + k] - 'A' + 10;
            } else {
                return 0;
            }
        }
        result[j] = numeral[0] * 16 + numeral[1];
    }
}

unsigned int decode_base10_number(char const * data, unsigned int data_size) {
    unsigned int result = 0;
    unsigned int weight = 1;
    int i;
    for (i = data_size - 1; i >= 0; --i) {
        char numeral = data[i];
        if (numeral >= '0' && numeral <= '9') {
            result += ((unsigned int)(numeral - '0')) * weight;
            weight *= 10;
        } else {
            return 0;
        }
    }

    return result;
}

typedef struct {
    unsigned int number;
    unsigned int days[7]; // set to 1 if meets that day, 0 otherwise
    unsigned int time_start;
    unsigned int time_end;
} Section;

typedef struct {
    char * name;
    unsigned int nsections;
    unsigned int * lengths;
    Section * * sections;
} Course;

void generate_courses(Course * * courses, unsigned int * ncourses) {
    *ncourses = 30;
    *courses = malloc(sizeof(Course) * (*ncourses));

    unsigned int i, j, k, l;
    for (i = 0; i < *ncourses; ++i) {
        Course * cur_course = (*courses) + i;

        // Generate the course name
        cur_course->name = malloc(10);
        sprintf(cur_course->name, "CS %u", i);

        // Figure out how many secionts there are and allocate memory for them
        cur_course->nsections = rand() % 10 + 1;
        cur_course->sections = malloc(
            sizeof(Section *) * cur_course->nsections);
        cur_course->lengths = malloc(
            sizeof(unsigned int) * cur_course->nsections);
        for (j = 0; j < cur_course->nsections; ++j) {
            cur_course->lengths[j] = rand() % 10 + 1;
            cur_course->sections[j] = malloc(
                sizeof(Section) * cur_course->lengths[j]);
        }

        // Make data for the sections
        unsigned int section_counter = 1;
        for (j = 0; j < cur_course->nsections; ++j) {
            for (k = 0; k < cur_course->lengths[j]; ++k) {
                Section * cur_section = cur_course->sections[j] + k;
                cur_section->number = section_counter++;

                // Figure out what days the calss meets
                int nmeetups = 0;
                for (l = 0; l < 5; ++l) {
                    cur_section->days[l] = rand() % 2;
                    if (cur_section->days[l]) {
                        nmeetups++;
                    }
                }
                if (nmeetups == 0) {
                    cur_section->days[rand() % 5] = 1;
                }

                // Figure out the meeting times
                cur_section->time_start = rand() % 12 + 6;
                cur_section->time_end = rand() % 12  + 6;
                if (cur_section->time_start > cur_section->time_end) {
                    unsigned int buf = cur_section->time_start;
                    cur_section->time_start = cur_section->time_end;
                    cur_section->time_end = buf;
                }
            }
        }
    }
}

void print_course(Course const * course) {
    char const * DAYS = "MTWRFSU";
    unsigned int const NDAYS = 7;

    printf("> %s", course->name);
    unsigned int i, j, k;
    for (i = 0; i < course->nsections; ++i) {
        for (j = 0; j < course->lengths[i]; ++j) {
            printf("\tSECTION %u: ", course->sections[i][j].number);

            // Print the days they meet
            char dayman[NDAYS + 1];
            int cur_day = 0;
            for (k = 0; k < NDAYS; ++k) {
                if (course->sections[i][j].days[k]) {
                    dayman[cur_day++] = DAYS[k];
                }
            }
            dayman[cur_day] = '\0';
            printf("\t\tMeets on %s", dayman);

            // Print the times they meet
            printf(" from %u to %u.\n", course->sections[i][j].time_start,
                course->sections[i][j].time_end);
        }
    }
}

void print_courses(Course const * courses, unsigned int ncourses) {
    unsigned int i;
    for (i = 0; i < ncourses; ++i) {
        print_course(courses + i);
    }
}

unsigned int const
    CONFORMANCES_LIST_CAPACITY = 1000,
    CONFORMANCES_LENGTHS_CAPACITY = 500,
    HEAP_CAPACITY = 5000,
    VISITED_CAPACITY = 8000,
    SOLUTION_CAPACITY = 500;

int main()
{
    srand(time(NULL));

    unsigned int * conformances_lists = malloc(
        CONFORMANCES_LIST_CAPACITY * sizeof(unsigned int));
    if (conformances_lists == NULL) {
        fprintf(stderr, "Failed to allocate conformance_lists.");
        return 5;
    }

    unsigned int * lengths = malloc(
        CONFORMANCES_LENGTHS_CAPACITY * sizeof(unsigned int));
    if (lengths == NULL) {
        fprintf(stderr, "Failed to allocate lengths.");
        return 5;
    }
    //Conformances conformances = {lists, lengths, 3};


    unsigned int * heap_data = malloc(
        HEAP_CAPACITY * sizeof(unsigned int));
    if (heap_data == NULL) {
        fprintf(stderr, "Failed to allocate heap_data.");
        return 5;
    }
    // IndicesHeap heap = {heap_data, 0, 100, 3};

    unsigned int * visited_data = malloc(
        VISITED_CAPACITY * sizeof(unsigned int));
    if (visited_data == NULL) {
        fprintf(stderr, "Failed to allocate visited_data.");
        return 5;
    }
    // VisitedSet visited = {visited_data, 0, 100, 3};

    unsigned int * solution = malloc(
        SOLUTION_CAPACITY * sizeof(unsigned int));
    if (solution == NULL) {
        fprintf(stderr, "Failed to allocate solution.");
        return 5;
    }

    Course * courses;
    unsigned int ncourses;
    generate_courses(&courses, &ncourses);

    print_courses(courses, ncourses);

    // char data[21] = "AB2C30405060708090A";
    // unsigned int result[10];
    // if (decode_hex_grid(data, 20, result)) {
    //     unsigned int i;
    //     for (i = 0; i < 10; ++i) {
    //         printf("%u, ", result[i]);
    //     }
    //     printf("\n");
    // }

    // printf("%u\n", decode_base10_number("1234", 4));

    // printf("%u\n", index_of(indices, 4, 2));
    // while (FCGI_Accept() >= 0) {
    //     printf("Content-type: text/html\r\n"
    //        "\r\n"
    //        "<title>FastCGI Hello! (C, fcgi_stdio library)</title>"
    //        "<h1>FastCGI Hello! (C, fcgi_stdio library)</h1>"
    //        "Request number running on host\n");
    // }

    // heap_add(&heap, &conformances, first_item);
    // set_add(&visited, first_item);

    // unsigned int solution[3];
    // while (foo(&conformances, &heap, &visited, solution)) {
    //     unsigned int i, j;
    //     for (i = 0; i < 3; ++i) {
    //         printf("%u ", solution[i]);
    //     }
    //     printf("(%u), ", get_conformance(&conformances, solution));

        // printf("heap: ");
        // for (i = 0; i < heap.length; ++i) {
        //     for (j = 0; j < 3; ++j) {
        //         printf("%u ", heap.data[i * heap.ndimensions + j]);
        //     }
        //     printf("; ");
        // }
        // printf("\n");
    // }
    // printf("\n");


    // unsigned int indices[][3] = {
    //     //3,2,6
    //     {0, 1, 2},
    //     {0, 0, 0},
    //     {1, 1, 1},
    //     {1, 0, 1},
    //     {2, 1, 4},
    //     {1, 0, 4},
    //     {2, 0, 1},
    //     {0, 0, 1},
    //     {0, 1, 0},
    //     {1, 1, 3}
    // };
    // unsigned int i;
    // for (i = 0; i < 10; ++i) {
    //     heap_add(&heap, &conformances, indices[i]);
    // }

    // for (i = 0; i < 30; ++i) {
    //     printf("%u ", heap.data[i]);
    // }
    // printf("\n");

    // unsigned int result[3];
    // while (heap_pop(&heap, &conformances, result)) {
    //     for (i = 0; i < 3; ++i) {
    //         printf("%u ", result[i]);
    //     }
    //     printf("(%u)\n", get_conformance(&conformances, result));
    // }

    return 0;
}
