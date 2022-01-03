#ifndef VOX_IMPORTER_H
#define VOX_IMPORTER_H

#include "editor/import/resource_importer_scene.h"
#include <editor/import/editor_import_plugin.h>

#include "streams/vox_data.h"

class VoxelBuffer;
class VoxelMesher;
class VoxelVoxImporter : public EditorSceneFormatImporter
{
  GDCLASS(VoxelVoxImporter, EditorSceneFormatImporter);

struct VoxMesh
{
  Ref<Mesh> mesh;
  Vector3 pivot;
};

  static Error process_scene_node_recursively(const vox::Data &data, int node_id,
                                                       Node3D *parent_node,
                                                       Node3D *&root_node, int depth,
                                                       const Vector<VoxMesh> &meshes);
  static void add_mesh_instance(Ref<Mesh> mesh, Node *parent, Node *owner,
                                Vector3 offset);
  static Ref<Mesh>
  build_mesh(VoxelBuffer &voxels, VoxelMesher &mesher,
             std::vector<unsigned int> &surface_index_to_material,
             Ref<Image> &out_atlas);

public:
  virtual uint32_t get_import_flags() const override
  {
    return IMPORT_SCENE;
  }
  virtual void get_extensions(List<String> *r_extensions) const override
  {
    ERR_FAIL_NULL(r_extensions);
    r_extensions->push_back("vox");
  }
  virtual Node *import_scene(const String &p_path, uint32_t p_flags, int p_bake_fps, List<String> *r_missing_deps = nullptr, Error *r_err = nullptr) override;
  virtual Ref<Animation> import_animation(const String &p_path, uint32_t p_flags, int p_bake_fps) override { return Ref<Animation>(); }
  VoxelVoxImporter() {}
};

#endif // VOX_IMPORTER_H
