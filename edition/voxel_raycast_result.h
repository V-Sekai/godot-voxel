#ifndef VOXEL_RAYCAST_RESULT_H
#define VOXEL_RAYCAST_RESULT_H

#include "../util/math/vector3i.h"
#include "core/object/ref_counted.h"

// This class exists only to make the script API nicer.
class VoxelRaycastResult : public RefCounted {
	GDCLASS(VoxelRaycastResult, RefCounted);

public:
	VoxelVector3i position;
	VoxelVector3i previous_position;
	float distance_along_ray = 0.0f;

private:
	Vector3 _b_get_position() const;
	Vector3 _b_get_previous_position() const;
	float _b_get_distance() const;

	static void _bind_methods();
};

#endif // VOXEL_RAYCAST_RESULT_H
