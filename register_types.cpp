#include "register_types.h"
#include "edition/voxel_tool.h"
#include "edition/voxel_tool_buffer.h"
#include "meshers/blocky/voxel_library.h"
#include "meshers/blocky/voxel_mesher_blocky.h"
#include "meshers/cubes/voxel_mesher_cubes.h"
#include "storage/voxel_buffer.h"
#include "storage/voxel_memory_pool.h"
#include "streams/vox_loader.h"
#include "util/macros.h"
#include "constants/voxel_string_names.h"

#include <core/config/engine.h>

#ifdef TOOLS_ENABLED
#include "editor/editor_plugin.h"
#include "vox_editor_plugin.h"
#endif

#ifdef VOXEL_RUN_TESTS
#include "tests/tests.h"
#endif

void register_vox_types() {
	VoxelMemoryPool::create_singleton();
	VoxelStringNames::create_singleton();

	// Misc
	ClassDB::register_class<Voxel>();
	ClassDB::register_class<VoxelLibrary>();
	ClassDB::register_class<VoxelColorPalette>();

	// Storage
	ClassDB::register_class<VoxelBuffer>();

	// Utilities
	ClassDB::register_class<VoxelRaycastResult>();
	ClassDB::register_class<VoxelTool>();
	// I had to bind this one despite it being useless as-is because otherwise Godot lazily initializes its class.
	// And this can happen in a thread, causing crashes due to the concurrent access
	ClassDB::register_class<VoxelToolBuffer>();
	ClassDB::register_class<VoxelVoxLoader>();

	// Meshers
	ClassDB::register_virtual_class<VoxelMesher>();
	ClassDB::register_class<VoxelMesherBlocky>();
	ClassDB::register_class<VoxelMesherCubes>();

	// Reminder: how to create a singleton accessible from scripts:
	// Engine::get_singleton()->add_singleton(Engine::Singleton("SingletonName",singleton_instance));

	PRINT_VERBOSE(String("Size of Object: {0}").format(varray((int)sizeof(Object))));
	PRINT_VERBOSE(String("Size of RefCounted: {0}").format(varray((int)sizeof(RefCounted))));
	PRINT_VERBOSE(String("Size of Node: {0}").format(varray((int)sizeof(Node))));
	PRINT_VERBOSE(String("Size of VoxelBuffer: {0}").format(varray((int)sizeof(VoxelBuffer))));

	EditorPlugins::add_by_type<VoxEditorPlugin>();

#ifdef VOXEL_RUN_TESTS
	run_voxel_tests();
#endif
}

void unregister_vox_types() {
	// At this point, the GDScript module has nullified GDScriptLanguage::singleton!!
	// That means it's impossible to free scripts still referenced by VoxelServer. And that can happen, because
	// users can write custom generators, which run inside threads, and these threads are hosted in the server...
	// See https://github.com/Zylann/godot_voxel/issues/189

	VoxelStringNames::destroy_singleton();

	// Do this last as VoxelServer might still be holding some refs to voxel blocks
	unsigned int used_blocks = VoxelMemoryPool::get_singleton()->debug_get_used_blocks();
	if (used_blocks > 0) {
		ERR_PRINT(String("VoxelMemoryPool: "
						 "{0} memory blocks are still used when unregistering the module. Recycling leak?")
						  .format(varray(used_blocks)));
	}
	VoxelMemoryPool::destroy_singleton();
}
