#include <stdio.h>   // For file input/output (fopen, fclose, fgets, fprintf, sscanf, perror)
#include <stdlib.h>  // For dynamic memory allocation (malloc, realloc, free), and qsort
#include <string.h>  // For string manipulation (strtok, strlen) - though sscanf is used here
#include <math.h>    // For floating point operations (fabs, NAN, isnan)
#include <float.h>   // For DBL_EPSILON (for comparing doubles)

// Define a maximum line length to prevent buffer overflow when reading CSV lines
#define MAX_LINE_LENGTH 1024

// Structure to hold original data as read from the input CSV file (data.csv)
typedef struct {
    double value1;
    double time1;
    double value2;
    double time2;
    // Flags indicating if a trade started at this specific original data point
    int original_trade1_started;
    int original_trade2_started;
} RawTradeEntry;

// Node for a linked list to store raw data.
// A linked list is used initially because the number of rows in the input CSV is unknown,
// making dynamic array resizing more complex during the initial read.
typedef struct RawTradeNode {
    RawTradeEntry data;
    struct RawTradeNode *next;
} RawTradeNode;

// Structure for the final synchronized output data, to be written to output.csv
typedef struct {
    double synchronized_time; // The common time point for this specific row
    double value1;            // Value1 at the synchronized_time
    int trade1_started;       // 1 if a trade for Value1 started at synchronized_time, 0 otherwise
    double value2;            // Value2 at the synchronized_time
    int trade2_started;       // 1 if a trade for Value2 started at synchronized_time, 0 otherwise
} SynchronizedTradeEntry;

// Comparison function for qsort, used to sort arrays of doubles.
// It returns -1 if arg1 < arg2, 1 if arg1 > arg2, and 0 if they are equal (within DBL_EPSILON).
int compare_doubles(const void *a, const void *b) {
    double arg1 = *(const double *)a;
    double arg2 = *(const double *)b;
    // Use epsilon for floating point comparison to handle precision issues
    if (fabs(arg1 - arg2) < DBL_EPSILON) return 0;
    if (arg1 < arg2) return -1;
    return 1;
}

// Function to free all nodes in the RawTradeNode linked list.
void free_raw_trade_list(RawTradeNode *head) {
    RawTradeNode *current = head;
    RawTradeNode *next;
    while (current != NULL) {
        next = current->next; // Store next node before freeing current
        free(current);       // Free current node
        current = next;      // Move to the next node
    }
}

int main() {
    FILE *input_file;
    FILE *output_file;
    char line[MAX_LINE_LENGTH]; // Buffer to hold each line read from the CSV

    RawTradeNode *head = NULL; // Head of the linked list for raw data
    RawTradeNode *tail = NULL; // Tail of the linked list for raw data
    int raw_data_count = 0;    // Counter for the number of raw data entries

    double *all_times = NULL; // Dynamic array to store all time1 and time2 values
    int all_times_count = 0;
    int all_times_capacity = 10; // Initial capacity for all_times array

    // Variables to keep track of previous values for calculating 'trade_started' flags
    double prev_value1 = 0.0;
    double prev_value2 = 0.0;
    int first_data_row = 1; // Flag to identify the very first data row for trade_started logic

    // --- 1. Open and Read data.csv into a Linked List ---
    input_file = fopen("data.csv", "r");
    if (input_file == NULL) {
        perror("Error opening data.csv"); // Print system error message
        return 1; // Indicate error
    }

    // Allocate initial memory for all_times array
    all_times = (double *)malloc(all_times_capacity * sizeof(double));
    if (all_times == NULL) {
        perror("Memory allocation failed for all_times");
        fclose(input_file);
        return 1;
    }

    // Read data line by line until end of file
    while (fgets(line, sizeof(line), input_file)) {
        // Allocate memory for a new raw trade node
        RawTradeNode *new_node = (RawTradeNode *)malloc(sizeof(RawTradeNode));
        if (new_node == NULL) {
            perror("Memory allocation failed for RawTradeNode");
            free_raw_trade_list(head); // Clean up already allocated nodes
            free(all_times);
            fclose(input_file);
            return 1;
        }

        // Parse the line using sscanf. Expecting 4 double values separated by commas.
        if (sscanf(line, "%lf,%lf,%lf,%lf",
                   &new_node->data.value1,
                   &new_node->data.time1,
                   &new_node->data.value2,
                   &new_node->data.time2) != 4) {
            fprintf(stderr, "Skipping malformed line: %s", line); // Report malformed lines
            free(new_node); // Free the node if parsing failed
            continue;       // Skip to the next line
        }

        // Calculate original_trade_started flags based on value change
        if (first_data_row) {
            // The very first data row always marks the start of a trade
            new_node->data.original_trade1_started = 1;
            new_node->data.original_trade2_started = 1;
            // Initialize previous values with the first row's values
            prev_value1 = new_node->data.value1;
            prev_value2 = new_node->data.value2;
            first_data_row = 0; // Turn off the flag after processing the first row
        } else {
            // For subsequent rows, a trade starts if the current value differs from the previous
            new_node->data.original_trade1_started = (fabs(new_node->data.value1 - prev_value1) > DBL_EPSILON) ? 1 : 0;
            new_node->data.original_trade2_started = (fabs(new_node->data.value2 - prev_value2) > DBL_EPSILON) ? 1 : 0;
            // Update previous values for the next iteration
            prev_value1 = new_node->data.value1;
            prev_value2 = new_node->data.value2;
        }

        new_node->next = NULL; // New node is always added at the end

        // Add the new node to the linked list
        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
        raw_data_count++; // Increment count of raw data entries

        // Add time1 and time2 to the all_times array
        // Check if there's enough capacity; if not, reallocate
        if (all_times_count + 2 > all_times_capacity) { // Need space for both time1 and time2
            all_times_capacity *= 2; // Double the capacity
            double *temp = (double *)realloc(all_times, all_times_capacity * sizeof(double));
            if (temp == NULL) {
                perror("Memory reallocation failed for all_times");
                free_raw_trade_list(head);
                free(all_times);
                fclose(input_file);
                return 1;
            }
            all_times = temp;
        }
        all_times[all_times_count++] = new_node->data.time1;
        all_times[all_times_count++] = new_node->data.time2;
    }
    fclose(input_file); // Close the input file

    if (raw_data_count == 0) {
        fprintf(stderr, "No valid data found in data.csv. Please ensure the file exists and contains data.\n");
        free(all_times);
        return 1;
    }

    // --- 2. Sort all_times and Create sorted_unique_times array ---
    // Sort the collected times
    qsort(all_times, all_times_count, sizeof(double), compare_doubles);

    double *sorted_unique_times = NULL; // Dynamic array for unique sorted times
    int unique_times_count = 0;
    int unique_times_capacity = 10; // Initial capacity for unique times array
    sorted_unique_times = (double *)malloc(unique_times_capacity * sizeof(double));
    if (sorted_unique_times == NULL) {
        perror("Memory allocation failed for sorted_unique_times");
        free_raw_trade_list(head);
        free(all_times);
        return 1;
    }

    // Populate sorted_unique_times by iterating through sorted all_times and adding only unique values
    if (all_times_count > 0) {
        sorted_unique_times[unique_times_count++] = all_times[0]; // Add the first element
        for (int i = 1; i < all_times_count; i++) {
            // Add element only if it's significantly different from the last unique element
            if (fabs(all_times[i] - sorted_unique_times[unique_times_count - 1]) > DBL_EPSILON) {
                // Reallocate if capacity is exceeded
                if (unique_times_count >= unique_times_capacity) {
                    unique_times_capacity *= 2;
                    double *temp = (double *)realloc(sorted_unique_times, unique_times_capacity * sizeof(double));
                    if (temp == NULL) {
                        perror("Memory reallocation failed for sorted_unique_times");
                        free_raw_trade_list(head);
                        free(all_times);
                        free(sorted_unique_times);
                        return 1;
                    }
                    sorted_unique_times = temp;
                }
                sorted_unique_times[unique_times_count++] = all_times[i];
            }
        }
    }
    free(all_times); // Free the temporary all_times array

    // --- 3. Convert Linked List of RawTradeEntry to an Array ---
    // This makes it easier to search for specific time points later.
    RawTradeEntry *raw_data_array = (RawTradeEntry *)malloc(raw_data_count * sizeof(RawTradeEntry));
    if (raw_data_array == NULL) {
        perror("Memory allocation failed for raw_data_array");
        free_raw_trade_list(head);
        free(sorted_unique_times);
        return 1;
    }
    current_node = head; // Start from the head of the linked list
    for (int i = 0; i < raw_data_count; i++) {
        raw_data_array[i] = current_node->data; // Copy data from linked list node to array
        current_node = current_node->next;       // Move to the next node
    }
    free_raw_trade_list(head); // Free the linked list nodes as data is now in the array

    // --- 4. Generate Synchronized Output Data ---
    SynchronizedTradeEntry *synchronized_data_array = NULL;
    int synchronized_data_count = 0;
    int synchronized_data_capacity = 10; // Initial capacity for the synchronized data array
    synchronized_data_array = (SynchronizedTradeEntry *)malloc(synchronized_data_capacity * sizeof(SynchronizedTradeEntry));
    if (synchronized_data_array == NULL) {
        perror("Memory allocation failed for synchronized_data_array");
        free(raw_data_array);
        free(sorted_unique_times);
        return 1;
    }

    double current_value1_at_sync_time = 0.0; // Holds the last known Value1
    double current_value2_at_sync_time = 0.0; // Holds the last known Value2
    int value1_initialized = 0; // Flag to check if Value1 has been set from actual data
    int value2_initialized = 0; // Flag to check if Value2 has been set from actual data

    // Iterate through each unique sorted time point to build the synchronized data
    for (int i = 0; i < unique_times_count; i++) {
        double current_sync_time = sorted_unique_times[i];
        SynchronizedTradeEntry current_output_row;
        current_output_row.synchronized_time = current_sync_time;
        current_output_row.trade1_started = 0; // Default to 0, only set to 1 if an actual trade started
        current_output_row.trade2_started = 0; // Default to 0

        RawTradeEntry *matching_raw_entry1 = NULL; // Pointer to raw entry if time1 matches current_sync_time
        RawTradeEntry *matching_raw_entry2 = NULL; // Pointer to raw entry if time2 matches current_sync_time

        // Search for raw data entries that exactly match the current synchronized time
        for (int j = 0; j < raw_data_count; j++) {
            if (fabs(raw_data_array[j].time1 - current_sync_time) < DBL_EPSILON) {
                matching_raw_entry1 = &raw_data_array[j];
            }
            if (fabs(raw_data_array[j].time2 - current_sync_time) < DBL_EPSILON) {
                matching_raw_entry2 = &raw_data_array[j];
            }
        }

        // Determine Value1 and Trade1Started for the current synchronized row
        if (matching_raw_entry1 != NULL) {
            // An original data point exists for time1 at this synchronized time
            current_output_row.value1 = matching_raw_entry1->value1;
            current_output_row.trade1_started = matching_raw_entry1->original_trade1_started;
            current_value1_at_sync_time = matching_raw_entry1->value1; // Update last known value
            value1_initialized = 1;
        } else {
            // No original data point for time1 at this synchronized time, carry forward the last known value
            if (!value1_initialized) {
                // If no value1 has been set yet (e.g., first few sync times only have value2 data)
                current_output_row.value1 = 0.0; // Default or a sentinel value
            } else {
                current_output_row.value1 = current_value1_at_sync_time;
            }
            current_output_row.trade1_started = 0; // No new trade started if value is carried forward
        }

        // Determine Value2 and Trade2Started for the current synchronized row
        if (matching_raw_entry2 != NULL) {
            // An original data point exists for time2 at this synchronized time
            current_output_row.value2 = matching_raw_entry2->value2;
            current_output_row.trade2_started = matching_raw_entry2->original_trade2_started;
            current_value2_at_sync_time = matching_raw_entry2->value2; // Update last known value
            value2_initialized = 1;
        } else {
            // No original data point for time2 at this synchronized time, carry forward the last known value
            if (!value2_initialized) {
                current_output_row.value2 = 0.0; // Default or a sentinel value
            } else {
                current_output_row.value2 = current_value2_at_sync_time;
            }
            current_output_row.trade2_started = 0; // No new trade started if value is carried forward
        }

        // Add the populated synchronized row to the output array
        // Reallocate if capacity is exceeded
        if (synchronized_data_count >= synchronized_data_capacity) {
            synchronized_data_capacity *= 2;
            SynchronizedTradeEntry *temp = (SynchronizedTradeEntry *)realloc(synchronized_data_array, synchronized_data_capacity * sizeof(SynchronizedTradeEntry));
            if (temp == NULL) {
                perror("Memory reallocation failed for synchronized_data_array");
                free(raw_data_array);
                free(sorted_unique_times);
                free(synchronized_data_array);
                return 1;
            }
            synchronized_data_array = temp;
        }
        synchronized_data_array[synchronized_data_count++] = current_output_row;
    }

    // --- 5. Open and Write to output.csv ---
    output_file = fopen("output.csv", "w");
    if (output_file == NULL) {
        perror("Error opening output.csv");
        // Clean up all allocated memory before exiting
        free(raw_data_array);
        free(sorted_unique_times);
        free(synchronized_data_array);
        return 1;
    }

    // Write the header row to the output CSV file
    fprintf(output_file, "SynchronizedTime,Value1,Trade1Started,Value2,Trade2Started\n");

    // Write each synchronized data entry to the output file
    for (int i = 0; i < synchronized_data_count; i++) {
        fprintf(output_file, "%.6f,%.2f,%d,%.2f,%d\n",
                synchronized_data_array[i].synchronized_time,
                synchronized_data_array[i].value1,
                synchronized_data_array[i].trade1_started,
                synchronized_data_array[i].value2,
                synchronized_data_array[i].trade2_started);
    }

    fclose(output_file); // Close the output file

    // --- 6. Free all dynamically allocated memory ---
    free(raw_data_array);
    free(sorted_unique_times);
    free(synchronized_data_array);

    printf("Data processed and saved to output.csv successfully.\n");

    return 0; // Indicate successful execution
}
