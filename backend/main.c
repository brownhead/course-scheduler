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

unsigned int get_conformance(Conformances const * conformances,
        unsigned int * indices) {
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
        unsigned int * new_indices) {
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
            } else if (right_child_index < heap->length) {
                unsigned int * right_child =
                    heap->data + right_child_index * heap->ndimensions;
                unsigned int right_child_conformance = get_conformance(
                    conformances, right_child);
                if (cur_conformance < right_child_conformance) {
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

// void foo(unsigned int const * lists, unsigned int cardinality,
//         unsigned int nsets, unsigned int const * previous_solutions,
//         unsigned int nprevious_solutions, unsigned int * * solutions,
//         unsigned int * nsolutions) {
//     // If there's no previous solutions just return the "top-left" corner of
//     // the matrix as the result.
//     if (nprevious_solutions == 0) {
//         *nsolutions = 1
//         *solutions = malloc(nsets * sizeof(unsigned int));
//         memset(*solutions, 0, nsets * sizeof(unsigned int));
//         return;
//     }
// }

int main()
{
    unsigned int lists[] = {
        19, 12, 3,
        33, 22,
        5, 4, 3, 2, 1, 0
    };
    unsigned int lengths[] = {3, 2, 6};
    Conformances conformances = {lists, lengths, 3};

    // unsigned int indices[] = {0, 1, 2};
    // printf("%u\n", get_conformance(&conformances, indices));

    unsigned int heap_data[30];
    memset(heap_data, 0, sizeof(unsigned int) * 30);
    IndicesHeap heap = {heap_data, 0, 10, 3};

    unsigned int indices[][3] = {
        //3,2,6
        {0, 1, 2},
        {0, 0, 0},
        {1, 1, 1},
        {1, 0, 1},
        {2, 1, 4},
        {1, 0, 4},
        {2, 0, 1},
        {0, 0, 1},
        {0, 1, 0},
        {1, 1, 3}
    };
    unsigned int i;
    for (i = 0; i < 10; ++i) {
        heap_add(&heap, &conformances, indices[i]);
    }

    for (i = 0; i < 30; ++i) {
        printf("%u ", heap.data[i]);
    }
    printf("\n");

    unsigned int result[3];
    while (heap_pop(&heap, &conformances, result)) {
        for (i = 0; i < 3; ++i) {
            printf("%u ", result[i]);
        }
        printf("(%u)\n", get_conformance(&conformances, result));
    }

    //printf("%u\n", index_of(indices, 4, 2));
    // while (FCGI_Accept() >= 0) {
    //     printf("Content-type: text/html\r\n"
    //        "\r\n"
    //        "<title>FastCGI Hello! (C, fcgi_stdio library)</title>"
    //        "<h1>FastCGI Hello! (C, fcgi_stdio library)</h1>"
    //        "Request number running on host\n");
    // }

    return 0;
}
