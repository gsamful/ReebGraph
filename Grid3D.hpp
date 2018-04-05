#ifndef GRID3D_H
#define GRID3D_H

#include "ScalarFunction.hpp"
#include <set>
#include <stdint.h>
#include <vector>

namespace contourtree {

struct Tet {
    int64_t v[4];
};

class Grid3D : public ScalarFunction
{
public:
    Grid3D (int resx, int resy, int resz);

public:
    int getMaxDegree();
    int getVertexCount();
    int getStar(int64_t v, std::vector<int64_t> &star);
    bool lessThan(int64_t v1, int64_t v2);
    unsigned char getFunctionValue(int64_t v);

public:
    void loadGrid(std::string fileName);

protected:
    void updateStars();

public:
    int dimx, dimy, dimz;
    int nv;
    std::vector<Tet> tets;
    int starin[14][3];
    int64_t star[14];
    std::vector<unsigned char> fnVals;

protected:
    inline int64_t index(int64_t x, int64_t y, int64_t z) {
        return (x + y * dimx + z * dimx * dimy);
    }
};

} // namespace 

#endif // GRID3D_H
