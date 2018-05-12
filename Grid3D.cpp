#include "Grid3D.hpp"
#include <cassert>
#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>

bool endsWith(std::string s, std::string ext) {
    if (s.size() < ext.size())
        return false;

    return s.substr(s.size() - ext.size(), ext.size()) == ext;
}

namespace contourtree {

Grid3D::Grid3D(int resx, int resy, int resz) :
    dimx(resx), dimy(resy), dimz(resz)
{
    nv = dimx * dimy * dimz;
    this->updateStars();
}

int Grid3D::getMaxDegree() {
    return 14;
}

int Grid3D::getVertexCount() {
    return nv;
}

int Grid3D::getStar(int64_t v, std::vector<int64_t> &star) {
    int z = v / (dimx * dimy);
    int rem = v % (dimx * dimy);
    int y = rem / dimx;
    int x = rem % dimx;

    int ct = 0;
    for(int i = 0;i < 14;i ++) {
        int _x = x + starin[i][0];
        int _y = y + starin[i][1];
        int _z = z + starin[i][2];
        if(_x < 0 || _x >= dimx ||
           _y < 0 || _y >= dimy ||
           _z < 0 || _z >= dimz) {
            continue;
        }
        star[ct ++] = (v + this->star[i]);
    }
    return ct;
}

bool Grid3D::lessThan(int64_t v1, int64_t v2) {
    if(fnVals[v1] < fnVals[v2]) {
        return true;
    } else if(fnVals[v1] == fnVals[v2]) {
        return (v1 < v2);
    }
    return false;
}

 scalar_t Grid3D::getFunctionValue(int64_t v) {
    return this->fnVals[v];
}
 //
//
void Grid3D::loadGrid(std::string fileName) {
	if (endsWith(fileName, ".raw")) {
		std::ifstream ip(fileName, std::ios::binary);
		this->fnVals.resize(nv);
		ip.read((char *)(void *)fnVals.data(), nv*sizeof(scalar_t));
		ip.close();
	}
	else if (endsWith(fileName, ".csv")) {
		std::ifstream ip(fileName);
		if (!ip.is_open())
			throw std::runtime_error("File could not be opened file=" + fileName);
		std::string fisrtString;
		ip >> fisrtString;

		fnVals.resize(nv);

		for (int i = 0; i < nv; ++i) {
			ip >> fnVals[i];
			if(i < 10)
			std::cout <<fnVals[i] << std::endl;
		}

	}
	else {
		assert(false && "unknown FileFormat");
	}

}

void Grid3D::updateStars() {
    int ordering [][3] = {
        {0,1,2},
        {0,2,1},
        {1,0,2},
        {1,2,0},
        {2,0,1},
        {2,1,0}
    };

    Tet tet;
    tet.v[0] = index(0,0,0);
    for(int i = 0;i < 6;i ++) {
        int64_t loc[] = {0,0,0};
        for(int j = 0;j < 3;j ++) {
            loc[ordering[i][j]] ++;
            tet.v[j + 1] = index(loc[0],loc[1],loc[2]);
        }
        tets.push_back(tet);
    }


    ////
    int x = 1;
    int y = 1;
    int z = 1;
    int v = index(1,1,1);

    std::set<int64_t> unique;
    for(int xx = -1;xx < 1;xx ++) {
        int _x = x + xx;
        for(int yy = -1;yy < 1;yy ++) {
            int _y = y + yy;
            for(int zz = -1;zz < 1;zz ++) {
                int _z = z + zz;
                int vv = index(_x,_y,_z);
                for(int i = 0;i < 6;i ++) {
                    int in = -1;
                    for(int j = 0;j < 4;j ++) {
                        if(tets[i].v[j] + vv == v) {
                            in = j;
                            break;
                        }
                    }
                    if(in != -1)
                    for(int j = 0;j < 4;j ++) {
                        if(j != in) {
                            unique.insert(tets[i].v[j] + vv);
                        }
                    }
                }
            }
        }
    }
    assert(unique.size() == 14);

    int sct = 0;
    for(int64_t adj: unique) {
        z = adj / (dimx * dimy);
        int rem = adj % (dimx * dimy);
        y = rem / dimx;
        x = rem % dimx;

        starin[sct][0] = x - 1;
        starin[sct][1] = y - 1;
        starin[sct][2] = z - 1;
        star[sct] = adj - v;
        sct ++;
    }

}

} // namespace
