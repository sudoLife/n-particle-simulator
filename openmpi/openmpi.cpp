#include "constants.hpp"
#include "defaults.hpp"
#include "partial_grid.hpp"
#include "particles.hpp"
#include <chrono>
#include <cxxopts.hpp>
#include <fmt/os.h>
#include <mpi.h>
#include <unistd.h>

typedef std::chrono::high_resolution_clock hclock;
using std::chrono::duration_cast;
typedef std::chrono::microseconds us;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

constexpr int root = 0;

int particleNum = defaults::particleNum;
int stepNum = defaults::stepNum;
int saveFreq = defaults::saveFreq;
std::string dumpFilename = "test.dump";
bool save = false;

// sets the above parameters
void parse_cmd(int argc, char **argv);

int main(int argc, char **argv)
{
    MPI::Init(argc, argv);
    parse_cmd(argc, argv);

    int rank = MPI::COMM_WORLD.Get_rank();
    int process_num = MPI::COMM_WORLD.Get_size();

    fmt::ostream dump_file = fmt::output_file(fmt::format("{}.{}", dumpFilename, rank));
    dump_file.print("step x y\n"); // CSV-like
    // ---------- Creating the type for network operations START
    MPI_Datatype MPI_Particle;
    int block_lengths[7] = {1, 1, 1, 1, 1, 1, 1};
    MPI_Aint offsets[7];
    MPI_Datatype types[7] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
    offsets[0] = offsetof(Particle, id);
    offsets[1] = offsetof(Particle, x);
    offsets[2] = offsetof(Particle, y);
    offsets[3] = offsetof(Particle, vx);
    offsets[4] = offsetof(Particle, vy);
    offsets[5] = offsetof(Particle, ax);
    offsets[6] = offsetof(Particle, ay);

    MPI_Type_create_struct(7, block_lengths, offsets, types, &MPI_Particle);
    MPI_Type_commit(&MPI_Particle);
    // ---------- Creating the type for network operations END

    Particles particles;
    particles.particles.resize(particleNum);

    if (rank == root)
    {
        particles.Generate(particleNum);
        fmt::print("{}\n", "Generated particles");
    }

    // making sure eveyrone's on the same page
    // this can be blocking because we haven't started benchmarking yet
    MPI::COMM_WORLD.Bcast(particles.particles.data(), particleNum, MPI_Particle, root);
    // because it's not set on all non-root processes otherwise
    particles.size = std::sqrt(constants::density * particleNum);

    fmt::print("{}\n", "Broadcasted particles");

    int full_grid_size = (particles.size / constants::distanceThreshold) + 1;

    if (rank == 0)
    {
        fmt::print("Full grid size: {} x {}\n", full_grid_size, full_grid_size);
    }

    // this is for gdb
    // delete this when debugged
    // {
    //     volatile int i = 0;
    //     char hostname[256];
    //     gethostname(hostname, sizeof(hostname));
    //     fmt::print("#{} PID {} on {} ready for attach\n", rank, getpid(), hostname);
    //     fflush(stdout);
    //     while (0 == i)
    //         sleep(5);
    // }

    // now we calcualte our rows on the grid
    int rows_per_process = full_grid_size / process_num;

    int row_start = rows_per_process * rank;

    int row_end = row_start + rows_per_process;
    if (rank == process_num - 1)
        row_end = full_grid_size;

    // the last grid may have a slightly bigger number of rows, that's okay
    int partial_row_num = row_end - row_start;

    PartialGrid grid(partial_row_num, full_grid_size, row_start);

    fmt::print("I am #{} and I work from {} to {}.\n", rank, row_start, row_end);

    for (auto &particle : particles)
        grid.Add(particle);

    fmt::print("{}\n", "Added grid");

    // having so many for loops in the same region is just too sad, lambda
    // might make it better
    // grid_x here is relative
    auto computeForParticle = [&](int grid_x, int grid_y, Particle &particle)
    {
        for (int gx = std::max(grid_x - 1, 0); gx <= std::min(grid_x + 1, partial_row_num - 1); gx++)
        {
            for (int gy = std::max(grid_y - 1, 0); gy <= std::min(grid_y + 1, full_grid_size - 1); gy++)
            {
                int cell_index = gx * full_grid_size + gy;
                for (auto it2 = grid[cell_index].begin(); it2 != grid[cell_index].end(); it2++)
                {
                    particles.ApplyForce(particle, **it2);
                }
            }
        }
    };

    std::vector<int> local_ids;
    // we can technically reserve something like particleNum / process_num
    // and that would work very well for a big number of steps
    // because even if there were any reallocations
    // we'd see very few of them after a few steps, because particles tend to be
    // spread out more or less evenly
    local_ids.reserve(particleNum);

    // this is what we use to signify the end of communication
    Particle end_of_communication{};
    end_of_communication.id = -1;

    auto t1 = hclock::now();

    for (int step = 0; step < stepNum; step++)
    {
        local_ids.clear();

        for (int grid_x = 0; grid_x < partial_row_num; grid_x++)
        {
            // all columns are ours because we divided the work by rows
            for (int grid_y = 0; grid_y < full_grid_size; grid_y++)
            {
                int cell_index1 = full_grid_size * grid_x + grid_y;
                for (auto it = grid[cell_index1].begin(); it != grid[cell_index1].end(); it++)
                {
                    computeForParticle(grid_x, grid_y, **it);
                    local_ids.push_back((*it)->id);
                }
            }
        }
        for (auto &id : local_ids)
        {
            auto old_cell_index = grid.getCellIndex(particles[id]);
            // old_cell_index is guaranteed to be valid, because it's in
            // our local_ids
            particles.Move(particles[id]);
            grid.CheckMove(particles[id], old_cell_index);

            auto new_row_abs_index = grid.getGridAbsX(particles[id]);
            auto new_row_rel_index = grid.getGridRelX(particles[id]);

            // if the particle doesn't belong to us anymore
            if (new_row_rel_index == -1)
            {
                // send it to the other guy, we don't care anymore
                // I am using my own rank as a tag
                // cool, right?
                int receiver_rank = new_row_abs_index / rows_per_process;
                if (receiver_rank >= process_num)
                    receiver_rank = process_num - 1;
                // fmt::print("{}, Sending to {}, new index {}, rows per process {}\n", rank, receiver_rank, new_row_abs_index, rows_per_process);

                MPI::COMM_WORLD.Isend(&particles[id], 1, MPI_Particle, receiver_rank, rank);
            }

            // send it to the previous guy
            if (new_row_rel_index == 0 && rank > 0)
            {
                MPI::COMM_WORLD.Isend(&particles[id], 1, MPI_Particle, rank - 1, rank);
            }

            // boundary, send it to next guy if it exists
            if (new_row_rel_index == partial_row_num - 1 && rank < process_num - 1)
            {
                MPI::COMM_WORLD.Isend(&particles[id], 1, MPI_Particle, rank + 1, rank);
            }

            // dump to file
            if ((save) && step % saveFreq == 0)
                dump_file.print("{} {} {}\n", step, particles[id].x, particles[id].y);
        }

        // signify that we are done talking
        if (rank > 0)
            MPI::COMM_WORLD.Isend(&end_of_communication, 1, MPI_Particle, rank - 1, rank);
        if (rank < process_num - 1)
            MPI::COMM_WORLD.Isend(&end_of_communication, 1, MPI_Particle, rank + 1, rank);

        // that done, we can now accept things from others

        Particle recv_particle;

        auto placeReceived = [&]
        {
            auto old_cell_index = grid.getCellIndex(particles[recv_particle.id]);
            particles[recv_particle.id] = recv_particle;

            if (old_cell_index != -1)
                grid.CheckMove(particles[recv_particle.id], old_cell_index);
            else
                grid.Add(particles[recv_particle.id]);
        };

        // receive from the previous guy
        while (rank - 1 >= 0)
        {
            MPI::COMM_WORLD.Recv(&recv_particle, 1, MPI_Particle, rank - 1, rank - 1);
            if (recv_particle.id == -1)
                break;
            placeReceived();
        }

        // receive from the next guy
        while (rank < process_num - 1)
        {
            MPI::COMM_WORLD.Recv(&recv_particle, 1, MPI_Particle, rank + 1, rank + 1);
            if (recv_particle.id == -1)
                break;
            placeReceived();
        }

        // Since the order of sends is the same as the order of receives
        // (https://www.mpi-forum.org/docs/mpi-1.1/mpi-11-html/node41.html),
        // nothing can be shuffled  and therefore the communication on the
        // previous step must be finished before the new one can occur,
        // therefore we don't need an explicit barrier to synchronize our
        // effort, the communication itself is the barrier
    }

    auto t2 = hclock::now();

    auto dur = duration_cast<us>(t2 - t1).count();

    dump_file.close();

    int64_t result = 0;
    MPI::COMM_WORLD.Reduce(&dur, &result, 1, MPI_INT64_T, MPI::SUM, root);

    if (rank == root)
    {
        long double mean = result;
        mean /= process_num;
        fmt::print("Time {}\n", mean);
    }

    MPI::Finalize();

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
        dumpFilename = result["filename"].as<std::string>();
    if (result.count("frequency"))
        saveFreq = result["frequency"].as<int>();

    // we just don't want that much stuff in the terminal
    if (MPI::COMM_WORLD.Get_rank() == 0)
    {
        fmt::print("Resulting settings:\nParticle num: {}\nStep num: {}\nSave frequency: {}\nDump file name: {}\n",
                   particleNum,
                   stepNum,
                   saveFreq,
                   dumpFilename);
    }
}
