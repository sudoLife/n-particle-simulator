#if !defined(GRID_HPP)
#define GRID_HPP

#include "particle.hpp"
#include <forward_list>
#include <mutex>
#include <vector>

class Grid
{
public:
    explicit Grid(int grid_size);
    void Add(Particle &particle);
    void Remove(Particle &particle, int cell_index);
    void CheckMove(Particle &particle, int old_cell_index);

    int getCellIndex(const Particle &particle) const;
    int getGridCoordinate(double coordinate) const;
    inline int getGridX(const Particle &particle) { return getGridCoordinate(particle.x); }
    inline int getGridY(const Particle &particle) { return getGridCoordinate(particle.y); }

    std::forward_list<Particle *> &operator[](int index);

public:
    std::vector<std::forward_list<Particle *>> grid;
    std::vector<std::mutex> locks;

private:
    int grid_dim_;
};

#endif // GRID_HPP
