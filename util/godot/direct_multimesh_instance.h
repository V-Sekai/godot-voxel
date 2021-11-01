#ifndef DIRECT_MULTIMESH_INSTANCE_H
#define DIRECT_MULTIMESH_INSTANCE_H

#include "../span.h"

#include "core/variant/variant.h"
#include "scene/3d/multimesh_instance_3d.h"
#include <core/templates/rid.h>

class World;

// Thin wrapper around VisualServer multimesh instance API
class DirectMultiMeshInstance {
public:
  DirectMultiMeshInstance();
  ~DirectMultiMeshInstance();

  void create();
  void destroy();
  bool is_valid() const;
  void set_world(World3D *world);
  void set_multimesh(Ref<MultiMesh> multimesh);
  Ref<MultiMesh> get_multimesh() const;
  void set_transform(Transform3D world_transform);
  void set_visible(bool visible);
  void set_material_override(Ref<BaseMaterial3D> material);
  void set_cast_shadows_setting(RenderingServer::ShadowCastingSetting mode);

  static PackedFloat32Array
  make_transform_3d_bulk_array(Span<const Transform3D> transforms);

private:
  RID _multimesh_instance;
  Ref<MultiMesh> _multimesh;
};

#endif // DIRECT_MULTIMESH_INSTANCE_H
