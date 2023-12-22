#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <complex.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>

#define DEFAULT_SAMPLES_COUNT   1024
#define DEFAULT_SQUELCH_LEVEL   0.01

typedef enum {
    SQUELCH_STATUS_CLOSE,
    SQUELCH_STATUS_OPEN
} t_squelch_status;

int main(int argc, char *argv[])
{
    float complex      *iq_in = NULL;
    float complex      *iq_zero = NULL;
    int                 sample_count = DEFAULT_SAMPLES_COUNT;
    float               squelch_level = DEFAULT_SQUELCH_LEVEL;
    bool                blocking = false;
    bool                dump = false;
    t_squelch_status    current_squelch_status = SQUELCH_STATUS_CLOSE;
    t_squelch_status    previous_squelch_status = SQUELCH_STATUS_CLOSE;
    struct timeval      tv_open;

    fprintf(stderr, "[SQUELCH] sizeof(float): %ld\n", sizeof(float));
    fprintf(stderr, "[SQUELCH] sizeof(float complex): %ld\n", sizeof(float complex));

    /*
     * Read args
     */

    while (1) {

        int c;
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
    iq_zero = malloc(sample_count * sizeof(float complex));
    memset(iq_zero, 0, sample_count * sizeof(float complex));

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
        if (mean_power_level > squelch_level) {

            current_squelch_status = SQUELCH_STATUS_OPEN;
            fwrite(iq_in, sizeof(float complex), sample_count, stdout);
        }
        else {

            /* Flush last samples before mute */
            if (current_squelch_status == SQUELCH_STATUS_OPEN)
                fwrite(iq_in, sizeof(float complex), sample_count, stdout);

            /* Mute */
            if (blocking == false)
                fwrite(iq_zero, sizeof(float complex), sample_count, stdout);

            current_squelch_status = SQUELCH_STATUS_CLOSE;
        }

        /* Dump gate status */
        if (previous_squelch_status != current_squelch_status) {

            if (current_squelch_status == SQUELCH_STATUS_OPEN) {

                struct tm  *timeinfo;
                char        time_str[32];

                gettimeofday(&tv_open, NULL);
                timeinfo = localtime(&tv_open.tv_sec);
                strftime(time_str, 32, "%Y-%m-%d %H:%M:%S", timeinfo);
                fprintf(stderr, "[SQUELCH] Opened at %s\n", time_str);
            }
            else {

                struct timeval  tv_close;
                struct timeval  tv_diff;

                gettimeofday(&tv_close, NULL);
                timersub(&tv_close, &tv_open, &tv_diff);
                fprintf(stderr, "[SQUELCH] Closed after %ld.%06lds\n", tv_diff.tv_sec, tv_diff.tv_usec);
            }
        }

        previous_squelch_status = current_squelch_status;
    }

    /*
     * Clean
     */

    free(iq_in);
    free(iq_zero);

    return EXIT_SUCCESS;
}