#include "direct_mesh_instance.h"
#include "../profiling.h"
#include <scene/resources/world_3d.h>

DirectMeshInstance::DirectMeshInstance() {
}

DirectMeshInstance::~DirectMeshInstance() {
	destroy();
}

bool DirectMeshInstance::is_valid() const {
	return _mesh_instance.is_valid();
}

void DirectMeshInstance::create() {
	ERR_FAIL_COND(_mesh_instance.is_valid());
	RenderingServer &rs = *RenderingServer::get_singleton();
	_mesh_instance = rs.instance_create();
	rs.instance_set_visible(_mesh_instance, true); // TODO Is it needed?
}

void DirectMeshInstance::destroy() {
	if (_mesh_instance.is_valid()) {
		RenderingServer &rs = *RenderingServer::get_singleton();
		rs.free(_mesh_instance);
		_mesh_instance = RID();
		_mesh.unref();
	}
}

void DirectMeshInstance::set_world(World3D *world) {
	ERR_FAIL_COND(!_mesh_instance.is_valid());
	RenderingServer &rs = *RenderingServer::get_singleton();
	if (world != nullptr) {
		rs.instance_set_scenario(_mesh_instance, world->get_scenario());
	} else {
		rs.instance_set_scenario(_mesh_instance, RID());
	}
}

void DirectMeshInstance::set_transform(Transform3D world_transform) {
	VOXEL_PROFILE_SCOPE();
	ERR_FAIL_COND(!_mesh_instance.is_valid());
	RenderingServer &rs = *RenderingServer::get_singleton();
	rs.instance_set_transform(_mesh_instance, world_transform);
}

void DirectMeshInstance::set_mesh(Ref<Mesh> mesh) {
	ERR_FAIL_COND(!_mesh_instance.is_valid());
	RenderingServer &rs = *RenderingServer::get_singleton();
	if (mesh.is_valid()) {
		if (_mesh != mesh) {
			rs.instance_set_base(_mesh_instance, mesh->get_rid());
		}
	} else {
		rs.instance_set_base(_mesh_instance, RID());
	}
	_mesh = mesh;
}

void DirectMeshInstance::set_material_override(Ref<Material> material) {
	ERR_FAIL_COND(!_mesh_instance.is_valid());
	RenderingServer &rs = *RenderingServer::get_singleton();
	if (material.is_valid()) {
		rs.instance_geometry_set_material_override(_mesh_instance, material->get_rid());
	} else {
		rs.instance_geometry_set_material_override(_mesh_instance, RID());
	}
}

void DirectMeshInstance::set_visible(bool visible) {
	ERR_FAIL_COND(!_mesh_instance.is_valid());
	RenderingServer &rs = *RenderingServer::get_singleton();
	rs.instance_set_visible(_mesh_instance, visible);
}

void DirectMeshInstance::set_cast_shadows_setting(RenderingServer::ShadowCastingSetting mode) {
	ERR_FAIL_COND(!_mesh_instance.is_valid());
	RenderingServer &rs = *RenderingServer::get_singleton();
	rs.instance_geometry_set_cast_shadows_setting(_mesh_instance, mode);
}

Ref<Mesh> DirectMeshInstance::get_mesh() const {
	return _mesh;
}
