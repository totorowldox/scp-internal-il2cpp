#include "il2cpp.h"

namespace il2cpp
{
	// variables
	pointer moduleBase;
	pointer assemblyBase;

	// Patch GC that wants to crash the entire game if suspicious thread found
	// Better option would be to hook up game thread, what ever, this is the fastest option right now, isn't it?
	bool GC_patched;

	// api
	void* unity_string_new;
	void* unity_resolve_icall;
	void* unity_find_objects;
	void* unity_get_transform;
	void* unity_get_main_camera;
	void* unity_get_gameobject;
	void* unity_create_text;
	void* unity_draw_text;
	void* unity_none_style;

	pointer GetModuleBase()
	{
		if (assemblyBase == 0)
		{
			assemblyBase = (pointer)GetModuleHandleA(assemblyName);
		}
		return assemblyBase;
	}

	template<class T>
	T* FindFunction(pointer offset)
	{
		return (T*)(GetModuleBase() + offset);
	}

	void Init() {

		/// This is not needed if you hooked game thread
		// fix GC crash
		/*if (!GC_patched) {

			auto base = GetModuleBase();
			DWORD dwOld;

			VirtualProtect((void*)(base + offset::GC_crash), 1, PAGE_EXECUTE_READWRITE, &dwOld);
			*(byte*)(base + offset::GC_crash) = offset::GC_patch;
			VirtualProtect((void*)(base + offset::GC_crash), 1, dwOld, NULL);

			GC_patched = true;
		}*/

		// il2cpp things
		unity_string_new = FindFunction<il2cpp_string_new>(offset::il2cpp_string_new);
		unity_resolve_icall = FindFunction<il2cpp_resolve_icall>(offset::il2cpp_resolve_icall);

		// unity3D things
		unity_find_objects = ((il2cpp_resolve_icall*)unity_resolve_icall)(fname_find_gameobjects);
		unity_get_transform = ((il2cpp_resolve_icall*)unity_resolve_icall)(fname_get_transform);
		unity_get_main_camera = ((il2cpp_resolve_icall*)unity_resolve_icall)(fname_get_current_camera);
		unity_get_gameobject = ((il2cpp_resolve_icall*)unity_resolve_icall)(fname_get_gameobject);

		// unity3D rendering
		unity_create_text = FindFunction<t_unity_create_gui_text>(0x103BE10);
		unity_draw_text = FindFunction<t_unity_label>(0x1050380);
		unity_none_style = FindFunction<t_unity_no_style>(0x1049CF0);

	}

	void draw_text(Rect position, const char* text) {
		auto il2cpp_string = ((il2cpp_string_new*)unity_string_new)(text);
		auto content = ((t_unity_create_gui_text*)unity_create_text)(il2cpp_string);
		auto style = ((t_unity_no_style*)unity_none_style)();
		((t_unity_label*)unity_draw_text)(position, content, style);
	}

	pointer get_current_camera() {
		auto ptr = ((t_unity_get_main_camera*)unity_get_main_camera)();
		return (pointer)ptr;
	}

	pointer* find_entities(const char* tag) {
		auto il2cpp_string = ((il2cpp_string_new*)unity_string_new)(tag);
		return ((t_unity_find_objects*)unity_find_objects)(il2cpp_string);
	}

	vec3 get_transform(pointer entity, int transform_type) {
		auto transform = ((t_unity_get_transform*)unity_get_transform)(entity);
		if (!transform) return vec3{};
		auto posdata = Read<pointer>((pointer)transform + offset::transform_component);
		if (!posdata) return vec3{};
		if (transform_type == TRANSFORM_IMMOVABLE) {
			return Read<vec3>(posdata + offset::transform_data_vector_structure);
		}
		posdata = Read<pointer>((pointer)posdata + offset::transform_component_data);
		return Read<vec3>(posdata + offset::transform_data_vector);
	}

	pointer get_gameobject(pointer component) {
		auto go = ((t_unity_get_gameobject*)unity_get_gameobject)(component);
		return go; 
	}

	vec3 get_camera_position(pointer camera) {
		if (camera != NULL) {
			camera = Read<pointer>((pointer)camera + offset::camera);
			return Read<vec3>(camera + offset::camera_position);
		}
		return vec3{};
	}

	Matrix get_viewmatrix() {
		auto camera = get_current_camera();
		return get_viewmatrix(camera);
	}
	Matrix get_viewmatrix(pointer camera) {
		if (camera != NULL) {
			camera = Read<pointer>((pointer)camera + offset::camera);
			return Read<Matrix>(camera + offset::matrix);
		}
		return Matrix{};
	}

}
