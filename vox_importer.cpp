#include "vox_importer.h"
#include "constants/voxel_string_names.h"
#include "meshers/cubes/voxel_mesher_cubes.h"
#include "storage/voxel_buffer.h"
#include "streams/vox_data.h"
#include "util/godot/funcs.h"

#include <core/io/file_access.h>
#include <scene/3d/mesh_instance_3d.h>
#include <scene/3d/node_3d.h>
#include <scene/resources/mesh.h>
#include <scene/resources/packed_scene.h>

Error VoxelVoxImporter::process_scene_node_recursively(const vox::Data &data, int node_id,
		Node3D *parent_node,
		Node3D *&root_node, int depth,
		const Vector<VoxMesh> &meshes) {
	ERR_FAIL_COND_V(depth > 10, ERR_INVALID_DATA);
	const vox::Node *vox_node = data.get_node(node_id);

	switch (vox_node->type) {
		case vox::Node::TYPE_TRANSFORM: {
			Node3D *node = memnew(Node3D);
			if (root_node == nullptr) {
				root_node = node;
			} else {
				ERR_FAIL_COND_V(parent_node == nullptr, ERR_BUG);
				parent_node->add_child(node);
				node->set_owner(root_node);
			}
			const vox::TransformNode *vox_transform_node =
					reinterpret_cast<const vox::TransformNode *>(vox_node);
			node->set_transform(Transform3D(vox_transform_node->rotation.basis,
					vox_transform_node->position.to_vec3()));
			process_scene_node_recursively(data, vox_transform_node->child_node_id,
					node, root_node, depth + 1, meshes);
		} break;

		case vox::Node::TYPE_GROUP: {
			const vox::GroupNode *vox_group_node =
					reinterpret_cast<const vox::GroupNode *>(vox_node);
			for (unsigned int i = 0; i < vox_group_node->child_node_ids.size(); ++i) {
				const int child_node_id = vox_group_node->child_node_ids[i];
				process_scene_node_recursively(data, child_node_id, parent_node,
						root_node, depth + 1, meshes);
			}
		} break;

		case vox::Node::TYPE_SHAPE: {
			ERR_FAIL_COND_V(parent_node == nullptr, ERR_BUG);
			ERR_FAIL_COND_V(root_node == nullptr, ERR_BUG);
			const vox::ShapeNode *vox_shape_node =
					reinterpret_cast<const vox::ShapeNode *>(vox_node);
			const VoxMesh &mesh_data = meshes[vox_shape_node->model_id];
			ERR_FAIL_COND_V(mesh_data.mesh.is_null(), ERR_BUG);
			const Vector3 offset = -mesh_data.pivot;
			add_mesh_instance(mesh_data.mesh, parent_node, root_node, offset);
		} break;

		default:
			ERR_FAIL_V(ERR_INVALID_DATA);
			break;
	}

	return OK;
}

Ref<Mesh>
VoxelVoxImporter::build_mesh(VoxelBuffer &voxels, VoxelMesher &mesher,
		std::vector<unsigned int> &surface_index_to_material,
		Ref<Image> &out_atlas) {
	//
	VoxelMesher::Output output;
	VoxelMesher::Input input = { voxels, 0 };
	mesher.build(output, input);

	if (output.surfaces.is_empty()) {
		return Ref<ArrayMesh>();
	}

	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	int surface_index = 0;
	for (int i = 0; i < output.surfaces.size(); ++i) {
		Array surface = output.surfaces[i];

		if (surface.is_empty()) {
			continue;
		}

		CRASH_COND(surface.size() != Mesh::ARRAY_MAX);
		if (!is_surface_triangulated(surface)) {
			continue;
		}

		mesh->add_surface_from_arrays(output.primitive_type, surface, Array());
		surface_index_to_material.push_back(i);
		++surface_index;
	}

	if (output.atlas_image.is_valid()) {
		out_atlas = output.atlas_image;
	}

	return mesh;
}
void VoxelVoxImporter::add_mesh_instance(Ref<Mesh> mesh, Node *parent, Node *owner,
		Vector3 offset) {
	MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
	mesh_instance->set_mesh(mesh);
	parent->add_child(mesh_instance);
	mesh_instance->set_owner(owner);
	mesh_instance->set_position(offset);
}

Node *VoxelVoxImporter::import_scene(const String &p_path, uint32_t p_flags, int p_bake_fps, List<String> *r_missing_deps, Error *r_err) {
	vox::Data data;
	const Error load_err = data.load_from_file(p_path);
	if (load_err != OK && r_err) {
		*r_err = load_err;
		return nullptr;
	}

	Vector<VoxMesh> meshes;
	meshes.resize(data.get_model_count());

	// Get color palette
	Ref<VoxelColorPalette> palette;
	palette.instantiate();
	for (unsigned int i = 0; i < data.get_palette().size(); ++i) {
		Color8 color = data.get_palette()[i];
		palette->set_color8(i, color);
	}

	Ref<VoxelMesherCubes> mesher;
	mesher.instantiate();
	mesher->set_color_mode(VoxelMesherCubes::COLOR_MESHER_PALETTE);
	mesher->set_palette(palette);
	mesher->set_greedy_meshing_enabled(false);
	mesher->set_store_colors_in_texture(true);

	FixedArray<Ref<StandardMaterial3D>, 2> materials;
	for (unsigned int i = 0; i < materials.size(); ++i) {
		Ref<StandardMaterial3D> &mat = materials[i];
		mat.instantiate();
		mat->set_roughness(1.f);
		mat->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC);
	}
	materials[1]->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);

	// Build meshes from voxel models
	for (unsigned int model_index = 0; model_index < data.get_model_count();
			++model_index) {
		const vox::Model &model = data.get_model(model_index);

		Ref<VoxelBuffer> voxels;
		voxels.instantiate();
		voxels->create(model.size + VoxelVector3i(VoxelMesherCubes::PADDING * 2));
		voxels->decompress_channel(VoxelBuffer::CHANNEL_COLOR);

		Span<uint8_t> dst_color_indices;
		if (!voxels->get_channel_raw(VoxelBuffer::CHANNEL_COLOR, dst_color_indices)) {
			if (r_err) {
				*r_err = ERR_BUG;
			}
			return nullptr;
		}
		Span<const uint8_t> src_color_indices = to_span_const(model.color_indexes);
		copy_3d_region_zxy(dst_color_indices, voxels->get_size(),
				VoxelVector3i(VoxelMesherCubes::PADDING),
				src_color_indices, model.size, VoxelVector3i(),
				model.size);

		std::vector<unsigned int> surface_index_to_material;
		Ref<Image> atlas;
		Ref<Mesh> mesh =
				build_mesh(**voxels, **mesher, surface_index_to_material, atlas);

		if (mesh.is_null()) {
			continue;
		}

		// Assign materials
		for (unsigned int surface_index = 0;
				surface_index < surface_index_to_material.size(); ++surface_index) {
			const unsigned int material_index =
					surface_index_to_material[surface_index];
			if (material_index >= materials.size()) {
				if (r_err) {
					*r_err = ERR_BUG;
					return nullptr;
				}
			}
			Ref<BaseMaterial3D> material = materials[material_index]->duplicate();
			if (atlas.is_valid()) {
				// TODO Do I absolutely HAVE to load this texture back to memory AND
				// renderer just so import works??
				// Ref<Texture> texture = ResourceLoader::load(atlas_path);
				// TODO THIS IS A WORKAROUND, it is not supposed to be an
				// ImageTexture... See earlier code, I could not find any way to
				// reference a separate StreamTexture.
				Ref<ImageTexture> texture;
				texture.instantiate();
				texture->create_from_image(atlas);
				material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, texture);
			}
			mesh->surface_set_material(surface_index, material);
		}

		VoxMesh mesh_info;
		mesh_info.mesh = mesh;
		// In MagicaVoxel scene graph, pivots are at the center of models, not at
		// the lower corner.
		mesh_info.pivot = (voxels->get_size() / 2 - VoxelVector3i(1)).to_vec3();
		meshes.write[model_index] = mesh_info;
	}

	Node3D *root_node = memnew(Node3D);
	if (data.get_root_node_id() != -1) {
		// Convert scene graph into a node tree
		process_scene_node_recursively(data, data.get_root_node_id(), root_node,
				root_node, 0, meshes);
	} else if (meshes.size() > 0) {
		// Some vox files don't have a scene graph
		const VoxMesh &mesh0 = meshes[0];
		add_mesh_instance(mesh0.mesh, root_node, root_node, Vector3());
	}
	return root_node;
}
