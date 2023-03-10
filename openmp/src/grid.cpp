#include "grid.hpp"
#include "constants.hpp"
#include <cmath>
#include <omp.h>
#include <vector>

Grid::Grid(int grid_dim)
{
    grid_dim_ = grid_dim;
    grid.resize(grid_dim * grid_dim);
    locks.resize(grid.size());

    for (auto &lock : locks)
    {
        omp_init_lock(&lock);
    }
}

Grid::~Grid()
{
    for (auto &lock : locks)
    {
        omp_destroy_lock(&lock);
    }
}

void Grid::Add(Particle &particle)
{
    auto cell_index = getCellIndex(particle);
    omp_set_lock(&locks[cell_index]);
    grid[cell_index].push_front(&particle);
    omp_unset_lock(&locks[cell_index]);
}

void Grid::Remove(Particle &particle, int cell_index)
{
    omp_set_lock(&locks[cell_index]);
    auto it = grid[cell_index].begin();
    auto prev = grid[cell_index].before_begin();

    for (; it != grid[cell_index].end(); it++, prev++)
    {
        if (*it == &particle)
        {
            grid[cell_index].erase_after(prev);
            omp_unset_lock(&locks[cell_index]);
            return;
        }
    }
    omp_unset_lock(&locks[cell_index]);
}

void Grid::CheckMove(Particle &particle, int old_cell_index)
{

    if (getCellIndex(particle) != old_cell_index)
    {
        // move the element
        // Okay I have to use my own implementation damn it
        // because I need to be able to remove elements without
        // making it a wack job (destroying them)

        // firest, remove
        Remove(particle, old_cell_index);
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
