#include "constants.hpp"
#include "defaults.hpp"
#include "grid.hpp"
#include "particles.hpp"
#include <cassert>
#include <chrono>
#include <cxxopts.hpp>
#include <fmt/os.h>
#include <omp.h>

typedef std::chrono::high_resolution_clock hclock;
using std::chrono::duration_cast;
typedef std::chrono::microseconds us;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

int particleNum = defaults::particleNum;
int stepNum = defaults::stepNum;
int saveFreq = defaults::saveFreq;
int threadNum = defaults::threadNum;

std::string dumpFilename = "test.dump";

// sets the above parameters
void parse_cmd(int argc, char **argv);

int main(int argc, char **argv)
{
    // TODO: replace this with an option?
    parse_cmd(argc, argv);

    omp_set_num_threads(threadNum);

    bool save = false;

    Particles particles(particleNum);

    // +1 is to ensure rounding
    int grid_size = (particles.size / constants::distanceThreshold) + 1;

    Grid grid(grid_size);

    for (auto &particle : particles)
        grid.Add(particle);

    fmt::ostream dump_file = fmt::output_file(dumpFilename);

    auto t1 = hclock::now();

#pragma omp parallel
    {
        auto thread_id = omp_get_thread_num();
        auto thread_num = omp_get_num_threads();
        // i'm assuming they are the same but OpenMP might decide differently
        assert(thread_num == threadNum);

        int chunkSize = particleNum / thread_num;
        int start = thread_id * chunkSize;
        int stop = start + chunkSize;
        if (thread_id == thread_num - 1)
            stop = particleNum;

        // all preparations are complete
        for (int step = 0; step < stepNum; step++)
        {
            for (int i = start; i < stop; i++)
            {
                auto grid_x = grid.getGridX(particles[i]);
                auto grid_y = grid.getGridY(particles[i]);

                // The min and max functions are added to ensure we don't run out of
                // bounds while checking neighbors
                for (int gx = std::max(grid_x - 1, 0); gx <= std::min(grid_x + 1, grid_size - 1); gx++)
                {
                    for (int gy = std::max(grid_y - 1, 0); gy <= std::min(grid_y + 1, grid_size - 1); gy++)
                    {
                        int cell_index = gx * grid_size + gy;

                        omp_set_lock(&grid.locks[cell_index]);
                        for (auto it = grid[cell_index].begin(); it != grid[cell_index].end(); it++)
                        {
                            particles.ApplyForce(particles[i], **it);
                        }
                        omp_unset_lock(&grid.locks[cell_index]);
                    }
                }
            }

#pragma omp barrier // waiting for everyone to be ready to move

            for (int i = start; i < stop; i++)
            {
                auto old_cell_index = grid.getCellIndex(particles[i]);

                particles.Move(particles[i]);
                grid.CheckMove(particles[i], old_cell_index);
            }
#pragma omp barrier // we need to make sure everyone moved their particles

            // as always, only one thread should do the dumping
            if (save && thread_id == 0 && step % saveFreq == 0)
            {
                particles.Save(dump_file);
            }
        }
    }

    auto t2 = hclock::now();

    auto dur = duration_cast<us>(t2 - t1).count();

    fmt::print("Time: {}\n", dur);

    return 0;
}

void parse_cmd(int argc, char **argv)
{
    cxxopts::Options options("particle-simulator", "N-particle simulator");

    options.add_options()("h,help", "See option list")
        /**/ ("n,particle-num", "Num of particles", cxxopts::value<int>())
        /**/ ("s,steps", "Num of steps", cxxopts::value<int>())
        /**/ ("o,filename", "Dump file name", cxxopts::value<std::string>())
        /**/ ("t,threads", "Number of threads", cxxopts::value<int>())
        /**/ ("f,frequency", "Dump frequency", cxxopts::value<int>());

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        fmt::print(options.help());
        exit(0);
    }
    if (result.count("particle-num"))
        particleNum = result["particle-num"].as<int>();
    if (result.count("steps"))
        stepNum = result["steps"].as<int>();
    if (result.count("filename"))
        dumpFilename = result["filename"].as<std::string>();
    if (result.count("frequency"))
        saveFreq = result["frequency"].as<int>();
    if (result.count("threads"))
        threadNum = result["threads"].as<int>();

    fmt::print("Resulting settings:\nParticle num: {}\nStep num: {}\nSave frequency: {}\nDump file name: {}\nThread num: {}\n",
               particleNum,
               stepNum,
               saveFreq,
               dumpFilename,
               threadNum);
}
