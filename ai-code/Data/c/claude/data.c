#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_ROWS 10000
#define MAX_LINE_LENGTH 1024
#define EPSILON 1e-9

typedef struct {
    double value1;
    double time1;
    double value2;
    double time2;
    int trade1_start;
    int trade2_start;
} TradeData;

typedef struct {
    double time;
    double value1;
    double value2;
    int trade1_start;
    int trade2_start;
    int is_interpolated;
} SyncedData;

// Function to trim whitespace from a string
char* trim(char* str) {
    char* end;
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    end[1] = '\0';
    return str;
}

// Function to parse CSV line
int parse_csv_line(char* line, TradeData* data) {
    char* token;
    char* line_copy = strdup(line);
    int field = 0;
    
    token = strtok(line_copy, ",");
    while (token != NULL && field < 4) {
        token = trim(token);
        switch (field) {
            case 0: data->value1 = atof(token); break;
            case 1: data->time1 = atof(token); break;
            case 2: data->value2 = atof(token); break;
            case 3: data->time2 = atof(token); break;
        }
        token = strtok(NULL, ",");
        field++;
    }
    
    free(line_copy);
    return (field == 4);
}

// Function to interpolate value at a given time
double interpolate_value(double t, double t1, double v1, double t2, double v2) {
    if (fabs(t2 - t1) < EPSILON) return v1;
    return v1 + (v2 - v1) * (t - t1) / (t2 - t1);
}

// Comparison function for qsort
int compare_synced_data(const void* a, const void* b) {
    SyncedData* da = (SyncedData*)a;
    SyncedData* db = (SyncedData*)b;
    if (da->time < db->time) return -1;
    if (da->time > db->time) return 1;
    return 0;
}

int main() {
    FILE* input_file;
    FILE* output_file;
    char line[MAX_LINE_LENGTH];
    TradeData data[MAX_ROWS];
    SyncedData synced[MAX_ROWS * 4]; // Allocate more space for interpolated points
    int row_count = 0;
    int synced_count = 0;
    
    // Open input file
    input_file = fopen("data.csv", "r");
    if (!input_file) {
        printf("Error: Could not open data.csv\n");
        return 1;
    }
    
    // Skip header line if present
    if (fgets(line, sizeof(line), input_file)) {
        // Check if first line looks like header
        if (strstr(line, "Value1") || strstr(line, "value1")) {
            // Skip header
        } else {
            // First line is data, rewind
            rewind(input_file);
        }
    }
    
    // Read data
    while (fgets(line, sizeof(line), input_file) && row_count < MAX_ROWS) {
        if (parse_csv_line(line, &data[row_count])) {
            row_count++;
        }
    }
    fclose(input_file);
    
    if (row_count == 0) {
        printf("Error: No valid data found\n");
        return 1;
    }
    
    // Detect trade starts
    data[0].trade1_start = 1; // First row is always a trade start
    data[0].trade2_start = 1;
    
    for (int i = 1; i < row_count; i++) {
        // Trade 1 start detection
        data[i].trade1_start = (fabs(data[i].value1 - data[i-1].value1) > EPSILON) ? 1 : 0;
        
        // Trade 2 start detection
        data[i].trade2_start = (fabs(data[i].value2 - data[i-1].value2) > EPSILON) ? 1 : 0;
    }
    
    // Create synchronized time series
    // First, add all original time points
    for (int i = 0; i < row_count; i++) {
        // Add time1 point
        synced[synced_count].time = data[i].time1;
        synced[synced_count].value1 = data[i].value1;
        synced[synced_count].trade1_start = data[i].trade1_start;
        synced[synced_count].is_interpolated = 0;
        
        // Find corresponding value2 and trade2_start for this time
        synced[synced_count].value2 = data[i].value2;
        synced[synced_count].trade2_start = 0;
        
        // Interpolate value2 if needed
        for (int j = 1; j < row_count; j++) {
            if (data[j].time2 >= data[i].time1) {
                if (fabs(data[j].time2 - data[i].time1) < EPSILON) {
                    synced[synced_count].value2 = data[j].value2;
                    synced[synced_count].trade2_start = data[j].trade2_start;
                } else if (j > 0) {
                    synced[synced_count].value2 = interpolate_value(data[i].time1, 
                        data[j-1].time2, data[j-1].value2, 
                        data[j].time2, data[j].value2);
                }
                break;
            }
        }
        synced_count++;
        
        // Add time2 point if different from time1
        if (fabs(data[i].time2 - data[i].time1) > EPSILON) {
            synced[synced_count].time = data[i].time2;
            synced[synced_count].value2 = data[i].value2;
            synced[synced_count].trade2_start = data[i].trade2_start;
            synced[synced_count].is_interpolated = 0;
            
            // Find corresponding value1 and trade1_start for this time
            synced[synced_count].value1 = data[i].value1;
            synced[synced_count].trade1_start = 0;
            
            // Interpolate value1 if needed
            for (int j = 1; j < row_count; j++) {
                if (data[j].time1 >= data[i].time2) {
                    if (fabs(data[j].time1 - data[i].time2) < EPSILON) {
                        synced[synced_count].value1 = data[j].value1;
                        synced[synced_count].trade1_start = data[j].trade1_start;
                    } else if (j > 0) {
                        synced[synced_count].value1 = interpolate_value(data[i].time2, 
                            data[j-1].time1, data[j-1].value1, 
                            data[j].time1, data[j].value1);
                    }
                    break;
                }
            }
            synced_count++;
        }
    }
    
    // Sort by time
    qsort(synced, synced_count, sizeof(SyncedData), compare_synced_data);
    
    // Remove duplicates
    int unique_count = 1;
    for (int i = 1; i < synced_count; i++) {
        if (fabs(synced[i].time - synced[unique_count-1].time) > EPSILON) {
            if (i != unique_count) {
                synced[unique_count] = synced[i];
            }
            unique_count++;
        } else {
            // Merge duplicate time points
            if (synced[i].trade1_start) synced[unique_count-1].trade1_start = 1;
            if (synced[i].trade2_start) synced[unique_count-1].trade2_start = 1;
        }
    }
    synced_count = unique_count;
    
    // Write output
    output_file = fopen("output.csv", "w");
    if (!output_file) {
        printf("Error: Could not create output.csv\n");
        return 1;
    }
    
    // Write header
    fprintf(output_file, "Time,Value1,Value2,Trade1_Start,Trade2_Start\n");
    
    // Write data
    for (int i = 0; i < synced_count; i++) {
        fprintf(output_file, "%.10f,%.6f,%.6f,%d,%d\n", 
            synced[i].time, 
            synced[i].value1, 
            synced[i].value2,
            synced[i].trade1_start,
            synced[i].trade2_start);
    }
    
    fclose(output_file);
    
    printf("Processing complete!\n");
    printf("Input rows: %d\n", row_count);
    printf("Output synchronized rows: %d\n", synced_count);
    printf("Results written to output.csv\n");
    
    return 0;
}