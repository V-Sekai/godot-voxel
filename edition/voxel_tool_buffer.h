#ifndef VOXEL_TOOL_BUFFER_H
#define VOXEL_TOOL_BUFFER_H

#include "voxel_tool.h"

class VoxelBuffer;

class VoxelToolBuffer : public VoxelTool {
	GDCLASS(VoxelToolBuffer, VoxelTool)
public:
	VoxelToolBuffer() {}
	VoxelToolBuffer(Ref<VoxelBuffer> vb);

	bool is_area_editable(const Box3i &box) const override;
	void paste(VoxelVector3i p_pos, Ref<VoxelBuffer> p_voxels,
			uint8_t channels_mask, uint64_t mask_value) override;

	void set_voxel_metadata(VoxelVector3i pos, Variant meta) override;
	Variant get_voxel_metadata(VoxelVector3i pos) override;

	void do_sphere(Vector3 center, float radius) override;

protected:
	uint64_t _get_voxel(VoxelVector3i pos) const override;
	float _get_voxel_f(VoxelVector3i pos) const override;
	void _set_voxel(VoxelVector3i pos, uint64_t v) override;
	void _set_voxel_f(VoxelVector3i pos, float v) override;
	void _post_edit(const Box3i &box) override;

private:
	Ref<VoxelBuffer> _buffer;
};

#endif // VOXEL_TOOL_BUFFER_H
