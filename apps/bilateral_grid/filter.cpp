#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "bilateral_grid.h"
#ifndef NO_AUTO_SCHEDULE
#include "bilateral_grid_auto_schedule_old.h"
#include "bilateral_grid_auto_schedule.h"
#endif

#include "halide_benchmark.h"
#include "HalideBuffer.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("Usage: ./filter input.png output.png range_sigma timing_iterations\n"
               "e.g. ./filter input.png output.png 0.1 10\n");
        return 0;
    }

    float r_sigma = (float) atof(argv[3]);
    int timing_iterations = atoi(argv[4]);

    Buffer<float> input = load_and_convert_image(argv[1]);
    Buffer<float> output(input.width(), input.height(), 1);

    bilateral_grid(input, r_sigma, output);

    // Timing code. Timing doesn't include copying the input data to
    // the gpu or copying the output back.

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        bilateral_grid(input, r_sigma, output);
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    #ifndef NO_AUTO_SCHEDULE
    // Old auto-scheduler version
    double min_t_auto_old = benchmark(timing_iterations, 10, [&]() {
        bilateral_grid_auto_schedule_old(input, r_sigma, output);
    });
    printf("Old auto-scheduler time: %gms\n", min_t_auto_old * 1e3);

    // New auto-scheduler version
    double min_t_auto = benchmark(timing_iterations, 10, [&]() {
        bilateral_grid_auto_schedule(input, r_sigma, output);
    });
    printf("New auto-scheduler time: %gms\n", min_t_auto * 1e3);
    #endif

    convert_and_save_image(output, argv[2]);

    return 0;
}
