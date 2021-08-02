#include "register_types.h"
#include "edition/voxel_tool.h"
#include "edition/voxel_tool_buffer.h"
#include "edition/voxel_tool_terrain.h"
#include "generators/voxel_generator_script.h"
#include "meshers/blocky/voxel_library.h"
#include "meshers/blocky/voxel_mesher_blocky.h"
#include "meshers/cubes/voxel_mesher_cubes.h"
#include "storage/voxel_buffer.h"
#include "storage/voxel_memory_pool.h"
#include "streams/vox_loader.h"
#include "streams/voxel_stream_script.h"
#include "terrain/voxel_box_mover.h"
#include "terrain/voxel_mesh_block.h"
#include "terrain/voxel_terrain.h"
#include "terrain/voxel_viewer.h"
#include "util/macros.h"
#include "constants/voxel_string_names.h"
#include "util/noise/fast_noise_lite.h"
#include "util/noise/fast_noise_lite_gradient.h"

#include <core/engine.h>

#ifdef TOOLS_ENABLED
#include "editor/editor_plugin.h"
#include "vox_editor_plugin.h"
#include "editor/voxel_debug.h"
#endif

#ifdef VOXEL_RUN_TESTS
#include "tests/tests.h"
#endif

void register_voxel_types() {
	VoxelMemoryPool::create_singleton();
	VoxelStringNames::create_singleton();
	VoxelServer::create_singleton();

	Engine::get_singleton()->add_singleton(Engine::Singleton("VoxelServer", VoxelServer::get_singleton()));

	// TODO Can I prevent users from instancing it? is "register_virtual_class" correct for a class that's not abstract?
	ClassDB::register_class<VoxelServer>();

	// Misc
	ClassDB::register_class<Voxel>();
	ClassDB::register_class<VoxelLibrary>();
	ClassDB::register_class<VoxelColorPalette>();

	// Storage
	ClassDB::register_class<VoxelBuffer>();

	// Nodes
	ClassDB::register_virtual_class<VoxelNode>();
	ClassDB::register_class<VoxelTerrain>();
	ClassDB::register_class<VoxelViewer>();
	// Streams
	ClassDB::register_virtual_class<VoxelStream>();
	ClassDB::register_class<VoxelStreamScript>();

	// Generators
	ClassDB::register_virtual_class<VoxelGenerator>();
	ClassDB::register_class<VoxelGeneratorScript>();

	// Utilities
	ClassDB::register_class<VoxelBoxMover>();
	ClassDB::register_class<VoxelRaycastResult>();
	ClassDB::register_class<VoxelTool>();
	ClassDB::register_class<VoxelToolTerrain>();
	// I had to bind this one despite it being useless as-is because otherwise Godot lazily initializes its class.
	// And this can happen in a thread, causing crashes due to the concurrent access
	ClassDB::register_class<VoxelToolBuffer>();
	ClassDB::register_class<VoxelVoxLoader>();
	ClassDB::register_class<FastNoiseLite>();
	ClassDB::register_class<FastNoiseLiteGradient>();

	// Meshers
	ClassDB::register_virtual_class<VoxelMesher>();
	ClassDB::register_class<VoxelMesherBlocky>();
	ClassDB::register_class<VoxelMesherCubes>();

	// Reminder: how to create a singleton accessible from scripts:
	// Engine::get_singleton()->add_singleton(Engine::Singleton("SingletonName",singleton_instance));

	PRINT_VERBOSE(String("Size of Object: {0}").format(varray((int)sizeof(Object))));
	PRINT_VERBOSE(String("Size of Reference: {0}").format(varray((int)sizeof(Reference))));
	PRINT_VERBOSE(String("Size of Node: {0}").format(varray((int)sizeof(Node))));
	PRINT_VERBOSE(String("Size of VoxelBuffer: {0}").format(varray((int)sizeof(VoxelBuffer))));
	PRINT_VERBOSE(String("Size of VoxelMeshBlock: {0}").format(varray((int)sizeof(VoxelMeshBlock))));
	PRINT_VERBOSE(String("Size of VoxelTerrain: {0}").format(varray((int)sizeof(VoxelTerrain))));

	EditorPlugins::add_by_type<VoxEditorPlugin>();

#ifdef VOXEL_RUN_TESTS
	run_voxel_tests();
#endif
}

void unregister_voxel_types() {
	// At this point, the GDScript module has nullified GDScriptLanguage::singleton!!
	// That means it's impossible to free scripts still referenced by VoxelServer. And that can happen, because
	// users can write custom generators, which run inside threads, and these threads are hosted in the server...
	// See https://github.com/Zylann/godot_voxel/issues/189

	VoxelStringNames::destroy_singleton();
	VoxelServer::destroy_singleton();

	// Do this last as VoxelServer might still be holding some refs to voxel blocks
	unsigned int used_blocks = VoxelMemoryPool::get_singleton()->debug_get_used_blocks();
	if (used_blocks > 0) {
		ERR_PRINT(String("VoxelMemoryPool: "
						 "{0} memory blocks are still used when unregistering the module. Recycling leak?")
						  .format(varray(used_blocks)));
	}
	VoxelMemoryPool::destroy_singleton();
	// TODO No remove?

	VoxelDebug::free_resources();

	// TODO Seriously, no remove?
	//EditorPlugins::remove_by_type<VoxelGraphEditorPlugin>();
}
