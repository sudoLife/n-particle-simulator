#include "constants.hpp"
#include "defaults.hpp"
#include "grid.hpp"
#include "particles.hpp"
#include <chrono>
#include <cxxopts.hpp>
#include <fmt/os.h>

typedef std::chrono::high_resolution_clock hclock;
using std::chrono::duration_cast;
typedef std::chrono::microseconds us;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

int particleNum = defaults::particleNum;
int stepNum = defaults::stepNum;
int saveFreq = defaults::saveFreq;
std::string dump_file_name = "test.dump";

// sets the above parameters
void parse_cmd(int argc, char **argv);

int main(int argc, char **argv)
{
    // TODO: replace this with an option?
    parse_cmd(argc, argv);
    bool save = true;

    Particles particles(particleNum);

    // +1 is to ensure rounding
    int grid_size = (particles.size / constants::distanceThreshold) + 1;

    Grid grid(grid_size);

    for (auto &particle : particles)
        grid.Add(particle);

    fmt::ostream dump_file = fmt::output_file(dump_file_name);

    auto t1 = hclock::now();

    for (int step = 0; step < stepNum; step++)
    {

        for (auto &particle : particles)
        {
            auto grid_x = grid.getGridX(particle);
            auto grid_y = grid.getGridY(particle);

            // The min and max functions are added to ensure we don't run out of
            // bounds while checking neighbors
            for (int gx = std::max(grid_x - 1, 0); gx <= std::min(grid_x + 1, grid_size - 1); gx++)
            {
                for (int gy = std::max(grid_y - 1, 0); gy <= std::min(grid_y + 1, grid_size - 1); gy++)
                {
                    int cell_index = gx * grid_size + gy;
                    for (auto it = grid.grid[cell_index].begin(); it != grid.grid[cell_index].end(); it++)
                    {
                        particles.ApplyForce(particle, **it);
                    }
                }
            }
        }

        for (auto &particle : particles)
        {
            auto old_cell_index = grid.getCellIndex(particle);

            particles.Move(particle);
            grid.CheckMove(particle, old_cell_index);
        }

        if (save && step % saveFreq == 0)
        {
            particles.Save(dump_file);
        }
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
        dump_file_name = result["filename"].as<std::string>();
    if (result.count("frequency"))
        saveFreq = result["frequency"].as<int>();

    fmt::print("Resulting settings:\nParticle num: {}\nStep num: {}\nSave frequency: {}\nDump file name: {}\n",
               particleNum,
               stepNum,
               saveFreq,
               dump_file_name);
}