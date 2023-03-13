#if !defined(PARTIAL_GRID_HPP)
#define PARTIAL_GRID_HPP

#include "constants.hpp"
#include "particle.hpp"
#include <cmath>
#include <forward_list>
#include <vector>

class PartialGrid
{
public:
    explicit PartialGrid(int grid_rows, int grid_cols, int offset_rows);
    void Add(Particle &particle);
    void Remove(Particle &particle, int cell_index);
    void CheckMove(Particle &particle, int old_cell_index);

    int getCellIndex(const Particle &particle);

    int getGridRelX(const Particle &particle);
    int getGridAbsX(const Particle &particle);

    inline int getGridY(const Particle &particle) { return (int)std::floor(particle.y / constants::distanceThreshold); }

    std::forward_list<Particle *> &operator[](int index);

public:
    std::vector<std::forward_list<Particle *>> grid;

private:
    int grid_rows_;
    int grid_cols_;
    int offset_rows_;
};

#endif // PARTIAL_GRID_HPP
