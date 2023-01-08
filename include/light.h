#ifndef LIGHT_H_
#define LIGHT_H_

#include <vector>

//#include "core.h"
#include "ray.h"
#include "geometry.h"

class Light {
 public:
  Light() = default;
  explicit Light(const Vec3f &pos, const Vec3f &color);
  virtual ~Light() = default;
  [[nodiscard]] virtual Vec3f emission(const Vec3f &pos, const Vec3f &dir) const = 0;
//  virtual float pdf(const Interaction &interaction, Vec3f pos)const = 0;
//  [[nodiscard]] virtual Vec3f sample(Interaction &interaction, float *pdf, Sampler &sampler) const = 0;
//  virtual bool intersect(Ray &ray, Interaction &interaction) const = 0;


  /// position of light in world space
  Vec3f position;
 protected:
  /// RGB color of the light
  Vec3f radiance;
};

class PointLight: public Light{
    using Light::Light;
    Vec3f emission(const Vec3f &pos, const Vec3f &dir) const override;
};
// Square area light, consist of two right triangles, normal always facing (0,-1,0)
//class SquareAreaLight : public Light {
// public:
//  explicit SquareAreaLight(const Vec3f &pos,
//                           const Vec3f &color,
//                           const Vec2f &size);
//  [[nodiscard]] Vec3f emission(const Vec3f &pos, const Vec3f &dir) const override;
//  float pdf(const Interaction &interaction, Vec3f pos) const override;
//  [[nodiscard]] Vec3f sample(Interaction &interaction, float *pdf, Sampler &sampler) const override;
//  bool intersect(Ray &ray, Interaction &interaction) const override;
// protected:
//  // build light mesh from position and size. position locates at the center of rectangle.
//  TriangleMesh light_mesh;
//  Vec2f size;
//  Vec3f left_behind_pos;
//};

#endif //LIGHT_H_
