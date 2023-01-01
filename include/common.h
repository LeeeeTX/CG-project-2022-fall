//
// Created by Renhui on 2022/12/19.
//

#ifndef CG_PROJECT_2022_FALL_COMMON_H
#define CG_PROJECT_2022_FALL_COMMON_H
#include <openvdb/openvdb.h>
#include <openvdb/tree/NodeManager.h>
#include <openvdb/version.h>
#include <openvdb/tools/SignedFloodFill.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include "Cell.h"

using Vec4f=openvdb::Vec4f;
using Vec3f=openvdb::Vec3f;
using Vec3i=openvdb::Vec3i;
using Vec2f=openvdb::Vec2f;
using Mat4f=openvdb::Mat4s;
using Mat3f=openvdb::Mat3s;
using Coord=openvdb::Coord;

using std::cin;
using std::cout;
using std::endl;


#endif //CG_PROJECT_2022_FALL_COMMON_H
