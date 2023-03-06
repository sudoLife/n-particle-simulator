#include "constants.hpp"
#include "grid.hpp"
#include "particles.hpp"
#include <chrono>
#include <fmt/os.h>

typedef std::chrono::high_resolution_clock hclock;
using std::chrono::duration_cast;
typedef std::chrono::microseconds us;
typedef std::chrono::nanoseconds ns;

const int kStepNum = 500;
const int kSaveFreq = 10;

int main()
{
    int n = 300;
    bool save = true;

    Particles particles(n);

    // TODO: do I really need the +1?
    int grid_size = (particles.size / constants::distanceThreshold) + 1;

    Grid grid(grid_size);

    for (auto &particle : particles)
        grid.Add(particle);

    fmt::ostream dump_file = fmt::output_file("test.dump");

    for (int step = 0; step < kStepNum; step++)
    {

        for (auto &particle : particles)
        {
            auto grid_x = grid.getGridX(particle);
            auto grid_y = grid.getGridY(particle);

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

        if (save)
        {
            particles.Save(dump_file);
        }
    }

    return 0;
}
