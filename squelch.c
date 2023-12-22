#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <complex.h>
#include <getopt.h>

#define DEFAULT_SAMPLES_COUNT   1024
#define DEFAULT_SQUELCH_LEVEL   0.01


int main(int argc, char *argv[])
{
    float complex  *iq_in = NULL;
    float complex  *iq_zero = NULL;
    int             sample_count = DEFAULT_SAMPLES_COUNT;
    float           squelch_level = DEFAULT_SQUELCH_LEVEL;
    int             c;
    bool            blocking = false;
    bool            dump = false;

    fprintf(stderr, "[SQUELCH] sizeof(float): %ld\n", sizeof(float));
    fprintf(stderr, "[SQUELCH] sizeof(float complex): %ld\n", sizeof(float complex));

    /*
     * Read args
     */

    while (1) {

        int option_index = 0;
        static struct option long_options[] = {
            {"blocking", no_argument,       0,  'b' },
            {"count",    required_argument, 0,  'c' },
            {"dump",     no_argument,       0,  'd' },
            {"level",    required_argument, 0,  'l' },
            {0,          0,                 0,  0   }
        };

        c = getopt_long(argc, argv, "bc:dl:",
                long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            fprintf(stderr, "[SQUELCH] option %s", long_options[option_index].name);
            if (optarg)
                fprintf(stderr, " with arg %s", optarg);
            fprintf(stderr, "\n");
            break;

        case 'b':
            blocking = true;
            break;

        case 'c':
            sample_count = atoi(optarg);
            break;

        case 'd':
            dump = true;
            break;

        case 'l':
            squelch_level = atof(optarg);
            break;

        default:
            return EXIT_FAILURE;
        }
    }

    fprintf(stderr, "[SQUELCH] Sample count:  %d\n", sample_count);
    fprintf(stderr, "[SQUELCH] Squelch level: %f\n", squelch_level);

    /*
     * Allocate buffer
     */

    iq_in = malloc(sample_count * sizeof(float complex));
    if (blocking == false) {

        iq_zero = malloc(sample_count * sizeof(float complex));
        memset(iq_zero, 0, sample_count * sizeof(float complex));
    }

    /*
     * Read complex samples
     */

    while (42) {

        float mean_power_level = 0;

        /* Read stdin */
        fread(iq_in, sizeof(float complex), sample_count, stdin);

        /* Compute mean power level */
        for (int n = 0; n < sample_count; n++)
            mean_power_level += cabsf(iq_in[n]);
        mean_power_level /= sample_count;

        if (dump == true)
            fprintf(stderr, "[SQUELCH] mean_power_level: %f\n", mean_power_level);

        /* stdout gate */
        if (mean_power_level > DEFAULT_SQUELCH_LEVEL)
            fwrite(iq_in, sizeof(float complex), sample_count, stdout);
        else if (blocking == false)
            fwrite(iq_zero, sizeof(float complex), sample_count, stdout);
    }

    /*
     * Clean
     */

    free(iq_in);
    if (iq_zero != NULL)
        free(iq_zero);

    return EXIT_SUCCESS;
}