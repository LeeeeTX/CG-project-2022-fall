#ifndef VDB_VOLUME_LOADER_H_
#define VDB_VOLUME_LOADER_H_

#include <memory>
#include "common.h"
//#include <openvdb/io/Stream.h>
#include <algorithm>


//template<typename GridType>
class VDBLoader {
public:
//    using TreeType = typename GridType::TreeType;
    VDBLoader() = default;

    void load(const std::string &filename);

    explicit VDBLoader(const std::string &filename);

    ~VDBLoader();

    std::vector<std::string> &getGridNames();

    Grids_data grids;
    openvdb::io::File *file;
    std::vector<std::string> gridNames;
    std::string filename;

    static std::string getGridType(const openvdb::GridBase::Ptr &grid);

    static float q_criterion(const Vec3sGrid &grid, const Coord &coord, const iBBox &ibbox);

private:
};


#endif /* VDB_VOLUME_LOADER_H_ */
