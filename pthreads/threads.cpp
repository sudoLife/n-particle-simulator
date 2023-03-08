#include "constants.hpp"
#include "defaults.hpp"
#include "grid.hpp"
#include "particles.hpp"
#include <barrier>
#include <chrono>
#include <cxxopts.hpp>
#include <fmt/os.h>
#include <thread>

typedef std::chrono::high_resolution_clock hclock;
using std::chrono::duration_cast;
typedef std::chrono::microseconds us;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

int particleNum = defaults::particleNum;
int stepNum = defaults::stepNum;
int saveFreq = defaults::saveFreq;
int threadNum = defaults::threadNum;
bool save = true;

std::string dump_file_name = "test.dump";

// sets the above parameters
void parse_cmd(int argc, char **argv);

int main(int argc, char **argv)
{
    // TODO: replace this with an option?
    parse_cmd(argc, argv);

    Particles particles(particleNum);

    // +1 is to ensure rounding
    int grid_size = (particles.size / constants::distanceThreshold) + 1;

    Grid grid(grid_size);

    for (auto &particle : particles)
        grid.Add(particle);

    fmt::ostream dump_file = fmt::output_file(dump_file_name);

    // if we wanted a callback we could provide it.
    // std::barrier barrier(threadNum, []
    //                      { fmt::print("Step finished\n"); });
    std::barrier barrier(threadNum);
    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    int chunkSize = particles.size / threadNum;

    auto simulatorThread = [&](int start, int stop, bool main = false)
    {
        for (int step = 0; step < stepNum; step++)
        {

            for (int i = start; i < stop; i++)
            {
                auto grid_x = grid.getGridX(particles[i]);
                auto grid_y = grid.getGridY(particles[i]);

                // The min and max functions are added to ensure we don't run out of
                // bounds while checking neighbors
                // would it be acceptable to parallelize this place?
                for (int gx = std::max(grid_x - 1, 0); gx <= std::min(grid_x + 1, grid_size - 1); gx++)
                {
                    for (int gy = std::max(grid_y - 1, 0); gy <= std::min(grid_y + 1, grid_size - 1); gy++)
                    {
                        int cell_index = gx * grid_size + gy;
                        for (auto it = grid[cell_index].begin(); it != grid[cell_index].end(); it++)
                        {
                            particles.ApplyForce(particles[i], **it);
                        }
                    }
                }
            }

            // this is an obvious optimization step
            for (int i = start; i < stop; i++)
            {
                auto old_cell_index = grid.getCellIndex(particles[i]);

                particles.Move(particles[i]);
                grid.CheckMove(particles[i], old_cell_index);
            }

            // we must not be jumping in time
            barrier.arrive_and_wait();

            if (main && save && step % saveFreq == 0)
            {
                particles.Save(dump_file);
            }

            barrier.arrive_and_wait();
        }
    };

    auto t1 = hclock::now();

    // now we have to assign the code;
    bool main = true;
    for (int chunk = 0; chunk < threadNum; chunk++)
    {
        auto start = chunkSize * chunk;
        auto end = start + chunkSize;
        if (chunk == threadNum - 1)
            end = particles.size;

        threads.emplace_back(simulatorThread, start, end, main);
        main = false;
    }

    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }

    auto t2 = hclock::now();

    auto dur = duration_cast<ms>(t2 - t1).count();

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
        /**/ ("t,threads", "Thread num", cxxopts::value<int>())
        /**/ ("f,frequency", "Dump frequency", cxxopts::value<int>());

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        fmt::print("{}", options.help());
        exit(0);
    }
    if (result.count("particle-num"))
        particleNum = result["particle-num"].as<int>();
    if (result.count("steps"))
        stepNum = result["steps"].as<int>();
    if (result.count("filename"))
        dump_file_name = result["filename"].as<std::string>();
    if (result.count("frequency"))
        saveFreq = result["frequency"].as<int>();
    if (result.count("thread-num"))
        threadNum = result["thread-num"].as<int>();

    fmt::print("Resulting settings:\nParticle num: {}\nStep num: {}\nSave frequency: {}\nDump file name: {}\nThread num: {}\n",
               particleNum,
               stepNum,
               saveFreq,
               dump_file_name,
               threadNum);
}