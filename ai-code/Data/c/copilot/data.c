#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 10000

typedef struct {
    double value1, time1, value2, time2;
    int start1, start2;
} TradeRow;

int cmp_time(const void* a, const void* b) {
    double t1 = *((double*)a);
    double t2 = *((double*)b);
    return (t1 > t2) - (t1 < t2);
}

int unique_sorted_times(double* times, int n, double* unique_out) {
    qsort(times, n, sizeof(double), cmp_time);
    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (count == 0 || times[i] != unique_out[count-1]) {
            unique_out[count++] = times[i];
        }
    }
    return count;
}

int main() {
    FILE* f = fopen("data.csv", "r");
    if (!f) {
        perror("Cannot open data.csv");
        return 1;
    }

    TradeRow rows[MAX_ROWS];
    int row_count = 0;
    double prev_v1 = -1, prev_v2 = -1;

    while (fscanf(f, "%lf,%lf,%lf,%lf", &rows[row_count].value1, &rows[row_count].time1,
                                         &rows[row_count].value2, &rows[row_count].time2) == 4) {
        rows[row_count].start1 = (row_count == 0 || rows[row_count].value1 != prev_v1) ? 1 : 0;
        rows[row_count].start2 = (row_count == 0 || rows[row_count].value2 != prev_v2) ? 1 : 0;
        prev_v1 = rows[row_count].value1;
        prev_v2 = rows[row_count].value2;
        row_count++;
    }
    fclose(f);

    double all_times[2 * MAX_ROWS];
    for (int i = 0; i < row_count; ++i) {
        all_times[i] = rows[i].time1;
        all_times[row_count + i] = rows[i].time2;
    }

    double unique_times[2 * MAX_ROWS];
    int time_count = unique_sorted_times(all_times, 2 * row_count, unique_times);

    FILE* out = fopen("output.csv", "w");
    if (!out) {
        perror("Cannot open output.csv");
        return 1;
    }

    fprintf(out, "time,value1,start1,value2,start2\n");

    int idx1 = 0, idx2 = 0;
    double cur_v1 = 0, cur_v2 = 0;
    int cur_s1 = 0, cur_s2 = 0;

    for (int i = 0; i < time_count; ++i) {
        double t = unique_times[i];

        while (idx1 < row_count && rows[idx1].time1 == t) {
            cur_v1 = rows[idx1].value1;
            cur_s1 = rows[idx1].start1;
            idx1++;
        }

        while (idx2 < row_count && rows[idx2].time2 == t) {
            cur_v2 = rows[idx2].value2;
            cur_s2 = rows[idx2].start2;
            idx2++;
        }

        fprintf(out, "%f,%f,%d,%f,%d\n", t, cur_v1, cur_s1, cur_v2, cur_s2);
        cur_s1 = 0;
        cur_s2 = 0;
    }

    fclose(out);
    printf("Output written to output.csv\n");
    return 0;
}
