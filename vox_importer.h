#ifndef VOX_IMPORTER_H
#define VOX_IMPORTER_H

#include "editor/import/resource_importer_scene.h"
#include "scene/3d/importer_mesh_instance_3d.h"
#include "scene/resources/importer_mesh.h"
#include <editor/import/editor_import_plugin.h>

#include "streams/vox_data.h"

class VoxelBuffer;
class VoxelMesher;
class VoxelVoxImporter : public EditorSceneFormatImporter {
	GDCLASS(VoxelVoxImporter, EditorSceneFormatImporter);

	struct VoxMesh {
		Ref<ImporterMesh> mesh;
		Vector3 pivot;
	};

	static Error process_scene_node_recursively(const vox::Data &data, int node_id,
			Node3D *parent_node,
			Node3D *&root_node, int depth,
			const Vector<VoxMesh> &meshes);
	static void add_mesh_instance(Ref<ImporterMesh> mesh, Node *parent, Node *owner,
			Vector3 offset);
	static Ref<ImporterMesh>
	build_mesh(VoxelBuffer &voxels, VoxelMesher &mesher,
			Ref<Image> &out_atlas);

public:
	virtual uint32_t get_import_flags() const override {
		return EditorSceneFormatImporter::IMPORT_SCENE;
	}
	virtual void get_extensions(List<String> *r_extensions) const override {
		ERR_FAIL_NULL(r_extensions);
		r_extensions->push_back("vox");
	}
	virtual Node *import_scene(const String &p_path, uint32_t p_flags, const HashMap<StringName, Variant> &p_options, List<String> *r_missing_deps, Error *r_err) override;
	VoxelVoxImporter() {}
};

#endif // VOX_IMPORTER_H
