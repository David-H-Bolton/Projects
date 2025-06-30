#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 10000
#define LINE_LEN 256

typedef struct {
    double value1;
    double time1;
    double value2;
    double time2;
    int tradeStart1;
    int tradeStart2;
} TradeRow;

int compareTimes(const void *a, const void *b) {
    double t1 = *(double *)a;
    double t2 = *(double *)b;
    if (t1 < t2) return -1;
    if (t1 > t2) return 1;
    return 0;
}

int main() {
    FILE *fp = fopen("data.csv", "r");
    if (!fp) {
        printf("Error opening data.csv\n");
        return 1;
    }

    TradeRow rows1[MAX_ROWS], rows2[MAX_ROWS];
    int count1 = 0, count2 = 0;

    char line[LINE_LEN];
    double prevValue1 = -1, prevValue2 = -1;

    while (fgets(line, sizeof(line), fp)) {
        double v1, t1, v2, t2;
        if (sscanf(line, "%lf,%lf,%lf,%lf", &v1, &t1, &v2, &t2) == 4) {
            // Store trade 1
            rows1[count1].value1 = v1;
            rows1[count1].time1 = t1;
            rows1[count1].tradeStart1 = (count1 == 0 || v1 != prevValue1) ? 1 : 0;
            prevValue1 = v1;
            count1++;

            // Store trade 2
            rows2[count2].value2 = v2;
            rows2[count2].time2 = t2;
            rows2[count2].tradeStart2 = (count2 == 0 || v2 != prevValue2) ? 1 : 0;
            prevValue2 = v2;
            count2++;
        }
    }
    fclose(fp);

    // Collect all unique times
    double allTimes[MAX_ROWS * 2];
    int totalTimes = 0;

    for (int i = 0; i < count1; i++) {
        allTimes[totalTimes++] = rows1[i].time1;
    }
    for (int i = 0; i < count2; i++) {
        allTimes[totalTimes++] = rows2[i].time2;
    }

    // Sort times
    qsort(allTimes, totalTimes, sizeof(double), compareTimes);

    // Remove duplicates
    double uniqueTimes[MAX_ROWS * 2];
    int uniqueCount = 0;
    for (int i = 0; i < totalTimes; i++) {
        if (i == 0 || allTimes[i] != allTimes[i - 1]) {
            uniqueTimes[uniqueCount++] = allTimes[i];
        }
    }

    // Open output file
    FILE *out = fopen("output.csv", "w");
    if (!out) {
        printf("Error opening output.csv for writing\n");
        return 1;
    }

    fprintf(out, "Time,Value1,TradeStart1,Value2,TradeStart2\n");

    int idx1 = 0, idx2 = 0;
    double lastValue1 = 0, lastValue2 = 0;
    int lastStart1 = 0, lastStart2 = 0;

    for (int i = 0; i < uniqueCount; i++) {
        double t = uniqueTimes[i];
        // Check for trade1 at this time
        if (idx1 < count1 && rows1[idx1].time1 == t) {
            lastValue1 = rows1[idx1].value1;
            lastStart1 = rows1[idx1].tradeStart1;
            idx1++;
        }
        // Otherwise, keep lastValue1

        // Check for trade2 at this time
        if (idx2 < count2 && rows2[idx2].time2 == t) {
            lastValue2 = rows2[idx2].value2;
            lastStart2 = rows2[idx2].tradeStart2;
            idx2++;
        }
        // Otherwise, keep lastValue2

        fprintf(out, "%.10f,%.2f,%d,%.2f,%d\n", t, lastValue1, lastStart1, lastValue2, lastStart2);

        // Reset trade starts after they are marked
        lastStart1 = 0;
        lastStart2 = 0;
    }

    fclose(out);
    printf("Output written to output.csv\n");
    return 0;
}
