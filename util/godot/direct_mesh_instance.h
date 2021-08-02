#ifndef DIRECT_MESH_INSTANCE_H
#define DIRECT_MESH_INSTANCE_H

#include <core/templates/rid.h>
#include <scene/resources/mesh.h>
#include <scene/resources/world_3d.h>

class World;

// Thin wrapper around VisualServer mesh instance API
class DirectMeshInstance {
public:
	DirectMeshInstance();
	~DirectMeshInstance();

	bool is_valid() const;
	void create();
	void destroy();
	void set_world(World3D *world);
	void set_transform(Transform3D world_transform);
	void set_mesh(Ref<Mesh> mesh);
	void set_material_override(Ref<Material> material);
	void set_visible(bool visible);
	void set_cast_shadows_setting(RenderingServer::ShadowCastingSetting mode);

	Ref<Mesh> get_mesh() const;

private:
	RID _mesh_instance;
	Ref<Mesh> _mesh;
};

#endif // DIRECT_MESH_INSTANCE_H
