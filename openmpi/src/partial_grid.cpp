#include "partial_grid.hpp"
#include "constants.hpp"

PartialGrid::PartialGrid(int grid_rows, int grid_cols, int offset_rows)
{
    grid_rows_ = grid_rows;
    grid_cols_ = grid_cols;
    offset_rows_ = offset_rows;

    grid.resize(grid_rows * grid_cols);
}

void PartialGrid::Add(Particle &particle)
{
    auto cell_index = getCellIndex(particle);

    // otherwise we just ignore it
    if (cell_index != -1)
    {
        grid[cell_index].push_front(&particle);
    }
}

void PartialGrid::Remove(Particle &particle, int cell_index)
{
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

void PartialGrid::CheckMove(Particle &particle, int old_cell_index)
{

    int new_cell_index = getCellIndex(particle);
    if (new_cell_index != old_cell_index)
    {
        Remove(particle, old_cell_index);

        // cherry on top
        if (new_cell_index != -1)
        {
            Add(particle);
        }
    }
}

int PartialGrid::getCellIndex(const Particle &particle)
{
    auto rel_x = getGridRelX(particle);

    if (rel_x == -1)
        return -1;

    return (rel_x * grid_cols_ + getGridY(particle));
}

int PartialGrid::getGridRelX(const Particle &particle)
{
    auto result = getGridAbsX(particle) - offset_rows_;
    // returning -1 in both cases for consistency, we could have returned an
    // arbitrary negative number when the particle is in a row preceding our
    // part of the grid
    if (result >= grid_rows_ || result < 0)
        return -1;

    return result;
}

int PartialGrid::getGridAbsX(const Particle &particle)
{
    return (int)std::floor(particle.x / constants::distanceThreshold);
}

std::forward_list<Particle *> &PartialGrid::operator[](int index)
{
    return grid[index];
}
