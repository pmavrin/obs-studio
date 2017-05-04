#include <obs-module.h>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("win-overlaymaster", "en-US")

struct overlay_source {
	obs_source_t *source;
    
	char *file;
	int width;
	int height;
	gs_texture* texture = nullptr;
	uint8_t* buf;
};

static const char *overlay_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Overlay Master";
}


void release_file(char* file)
{
	
}

static void overlay_update(void *data, obs_data_t *settings)
{
	struct overlay_source *overlay = static_cast<struct overlay_source*>(data);
	const char *file = obs_data_get_string(settings, "file");
	const int width = obs_data_get_int(settings, "width");
	const int height = obs_data_get_int(settings, "height");

	if (overlay->file)
		release_file(overlay->file);
	overlay->file = bstrdup(file);
	overlay->height = height;
	overlay->width = width;
	
	int size = overlay->width * overlay->height * 4;
	
	
	if (overlay->texture)
	{
		gs_texture_destroy(overlay->texture);
	}
	overlay->texture = gs_texture_create(
		overlay->width, overlay->height, GS_BGRA, 1, (const uint8_t**)&(overlay->buf), 0);
		
}

boost::interprocess::file_mapping m_file;
boost::interprocess::mapped_region region;

static void *overlay_create(obs_data_t *settings, obs_source_t *source)
{
	struct overlay_source *overlay = static_cast<struct overlay_source*>(bzalloc(sizeof(struct overlay_source)));
	overlay->source = source;
	overlay_update(overlay, settings);

	overlay->buf = new uint8_t[1920 * 1080 * 4];
	
	for (int i = 0; i < 1920 * 1080 * 4; i++) {
		overlay->buf[i] = (i & 255);
	}

	obs_enter_graphics();
	overlay->texture = gs_texture_create(overlay->width, overlay->height, GS_RGBA, 1, NULL, GS_DYNAMIC);
	gs_texture_set_image(overlay->texture,
		(const uint8_t*)overlay->buf, overlay->width * 4, false);
	obs_leave_graphics();
	//gs_texture_set_image(overlay->texture, )
	obs_leave_graphics();

	m_file = boost::interprocess::file_mapping("c:\\work\\image.bin", boost::interprocess::read_only);
	region = boost::interprocess::mapped_region(m_file, boost::interprocess::read_only);

	return overlay;
}

static void draw_solid(overlay_source* overlay, int color)
{
	gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t    *color_param = gs_effect_get_param_by_name(solid, "color");
	gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

	struct vec4 colorVal;
	vec4_from_rgba(&colorVal, color);
	gs_effect_set_vec4(color_param, &colorVal);

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);

	gs_draw_sprite(0, 0, overlay->width, overlay->height);

	gs_technique_end_pass(tech);
	gs_technique_end(tech);
}

int c = 0;

static void overlay_render(void *data, gs_effect_t *effect)
{
	struct overlay_source *overlay = static_cast<struct overlay_source*>(data);
	if (!overlay->texture)
	{

		draw_solid(overlay, 0x88000088);
		return;
	}
	
	obs_enter_graphics();
	
	void* addr = region.get_address();
	const char* mem = static_cast<char*>(addr);
	std::size_t len = region.get_size();
	
	memcpy(overlay->buf, mem, len);
	
	gs_texture_set_image(overlay->texture,
		(const uint8_t*)overlay->buf, overlay->width * 4, false);

	while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_DEFAULT), "Draw"))
		obs_source_draw(overlay->texture, 0, 0, 0, 0, false);

//	draw_solid(overlay, 0x22ffffff);
	obs_leave_graphics();
}

static void overlay_destroy(void *data)
{
	struct overlay_source *overlay = static_cast<struct overlay_source*>(data);
}

static uint32_t overlay_width(void *data)
{
	struct overlay_source *overlay = static_cast<struct overlay_source*>(data);
	return overlay->width;
}

static uint32_t overlay_height(void *data)
{
	struct overlay_source *overlay = static_cast<struct overlay_source*>(data);
	return overlay->height;
}

static void overlay_defaults(obs_data_t *defaults)
{
	obs_data_set_default_int(defaults, "width", 1920);
	obs_data_set_default_int(defaults, "height", 1080);
	obs_data_set_default_string(defaults, "file", "c:\\work\\image.bin");
}

static obs_properties_t *overlay_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *ppts = obs_properties_create();
	obs_property_t *p;

	obs_properties_add_text(ppts,
		"file", obs_module_text("File"), OBS_TEXT_DEFAULT);
	obs_properties_add_int(ppts,
		"width", obs_module_text("Width"), 1, 1920, 1);
	obs_properties_add_int(ppts,
		"height", obs_module_text("Height"), 1, 1080, 1);
	return ppts;
}

static void overlay_tick(void *data, float seconds)
{
	struct overlay_source *overlay = static_cast<struct overlay_source*>(data);
}

struct obs_source_info my_source;

bool obs_module_load(void)
{
	my_source.id = "overlay";
	my_source.type = OBS_SOURCE_TYPE_INPUT;
	my_source.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	my_source.get_name = overlay_getname;
	my_source.create = overlay_create;
	my_source.destroy = overlay_destroy;
	my_source.update = overlay_update;
	my_source.video_render = overlay_render;
	my_source.video_tick = overlay_tick;
	my_source.get_width = overlay_width;
	my_source.get_height = overlay_height;
	my_source.get_defaults = overlay_defaults;
	my_source.get_properties = overlay_properties;
	
	obs_register_source(&my_source);
	return true;
}

