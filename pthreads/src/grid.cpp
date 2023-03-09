#include "grid.hpp"
#include "constants.hpp"
#include <cmath>
#include <vector>

Grid::Grid(int grid_dim)
{
    grid_dim_ = grid_dim;
    grid.resize(grid_dim * grid_dim);
    // we cannot resize this vector because that needs the objects to be mutable;
    locks = std::vector<std::mutex>(grid.size());
}

void Grid::Add(Particle &particle)
{
    auto cell_index = getCellIndex(particle);

    // no need for scoped_lock here, really
    locks[cell_index].lock();
    grid[cell_index].push_front(&particle);
    locks[cell_index].unlock();
}

void Grid::Remove(Particle &particle, int cell_index)
{
    // locking only access to the current cell
    std::scoped_lock<std::mutex> lock(locks[cell_index]);

    auto it = grid[cell_index].begin();
    auto prev = grid[cell_index].before_begin();

    for (; it != grid[cell_index].end(); it++, prev++)
    {
        if (*it == &particle)
        {
            grid[cell_index].erase_after(prev);
            return;
        }
    }
}

void Grid::CheckMove(Particle &particle, int old_cell_index)
{

    if (getCellIndex(particle) != old_cell_index)
    {
        // first, remove
        Remove(particle, old_cell_index);
        // then, add to another cell
        Add(particle);
    }
    // else do nothing
}

int Grid::getCellIndex(const Particle &particle) const
{
    return (particle.x * grid_dim_ + particle.y);
}

int Grid::getGridCoordinate(double coordinate) const
{
    return (int)std::floor(coordinate / constants::distanceThreshold);
}

std::forward_list<Particle *> &Grid::operator[](int index)
{
    return grid[index];
}
