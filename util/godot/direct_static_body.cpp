/**************************************************************************/
/*  direct_static_body.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "direct_static_body.h"
#include "../profiling.h"

#include "servers/physics_server_3d.h"
#include <scene/resources/world_3d.h>

DirectStaticBody::DirectStaticBody() {}

DirectStaticBody::~DirectStaticBody() {
	destroy();
}

void DirectStaticBody::create() {
	ERR_FAIL_COND(_body.is_valid());
	PhysicsServer3D &ps = *PhysicsServer3D::get_singleton();
	_body = ps.body_create();
	ps.body_set_ray_pickable(_body, false);
}

void DirectStaticBody::destroy() {
	if (_body.is_valid()) {
		PhysicsServer3D &ps = *PhysicsServer3D::get_singleton();
		ps.free(_body);
		_body = RID();
		// The shape need to be destroyed after the body
		_shape.unref();
	}
	if (_debug_mesh_instance.is_valid()) {
		_debug_mesh_instance.destroy();
	}
}

bool DirectStaticBody::is_valid() const {
	return _body.is_valid();
}

void DirectStaticBody::set_transform(Transform3D transform) {
	VOXEL_PROFILE_SCOPE();
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D::get_singleton()->body_set_state(
			_body, PhysicsServer3D::BODY_STATE_TRANSFORM, transform);

	if (_debug_mesh_instance.is_valid()) {
		_debug_mesh_instance.set_transform(transform);
	}
}

void DirectStaticBody::add_shape(Ref<Shape3D> shape) {
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D::get_singleton()->body_add_shape(_body, shape->get_rid(),
			Transform3D(), false);
	// No use case for multishape yet
	_shape = shape;

	if (_debug_mesh_instance.is_valid()) {
		Ref<Mesh> mesh = _shape->get_debug_mesh();
		_debug_mesh_instance.set_mesh(mesh);
	}
}

void DirectStaticBody::remove_shape(int shape_index) {
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D::get_singleton()->body_remove_shape(_body, shape_index);
	_shape.unref();

	if (_debug_mesh_instance.is_valid()) {
		_debug_mesh_instance.set_mesh(Ref<Mesh>());
	}
}

Ref<Shape3D> DirectStaticBody::get_shape(int shape_index) {
	ERR_FAIL_COND_V(shape_index < 0 || shape_index > 1, Ref<Shape3D>());
	return _shape;
}

void DirectStaticBody::set_world(World3D *world) {
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D &ps = *PhysicsServer3D::get_singleton();
	ps.body_set_space(_body, world != nullptr ? world->get_space() : RID());

	if (_debug_mesh_instance.is_valid()) {
		_debug_mesh_instance.set_world(world);
	}
}

void DirectStaticBody::set_shape_enabled(int shape_index, bool enabled) {
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D &ps = *PhysicsServer3D::get_singleton();
	ps.body_set_shape_disabled(_body, shape_index, !enabled);

	if (_debug_mesh_instance.is_valid()) {
		_debug_mesh_instance.set_visible(enabled);
	}
}

void DirectStaticBody::set_attached_object(Object *obj) {
	// Serves in high-level collision query results, `collider` will contain the
	// attached object
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D::get_singleton()->body_attach_object_instance_id(
			_body, obj != nullptr ? obj->get_instance_id() : ObjectID());
}

void DirectStaticBody::set_collision_layer(int layer) {
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D::get_singleton()->body_set_collision_layer(_body, layer);
}

void DirectStaticBody::set_collision_mask(int mask) {
	ERR_FAIL_COND(!_body.is_valid());
	PhysicsServer3D::get_singleton()->body_set_collision_mask(_body, mask);
}

void DirectStaticBody::set_debug(bool enabled, World3D *world) {
	ERR_FAIL_COND(world == nullptr);

	if (enabled && !_debug_mesh_instance.is_valid()) {
		_debug_mesh_instance.create();
		_debug_mesh_instance.set_world(world);

		Transform3D transform = PhysicsServer3D::get_singleton()->body_get_state(
				_body, PhysicsServer3D::BODY_STATE_TRANSFORM);
		_debug_mesh_instance.set_transform(transform);

		if (_shape.is_valid()) {
			Ref<Mesh> mesh = _shape->get_debug_mesh();
			_debug_mesh_instance.set_mesh(mesh);
		}

	} else if (!enabled && _debug_mesh_instance.is_valid()) {
		_debug_mesh_instance.destroy();
	}
}
