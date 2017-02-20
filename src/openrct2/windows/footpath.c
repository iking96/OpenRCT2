#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "../audio/audio.h"
#include "../cheats.h"
#include "../game.h"
#include "../input.h"
#include "../interface/themes.h"
#include "../interface/viewport.h"
#include "../interface/widget.h"
#include "../localisation/localisation.h"
#include "../rct2.h"
#include "../sprites.h"
#include "../world/footpath.h"
#include "dropdown.h"

enum {
	PATH_CONSTRUCTION_MODE_LAND,
	PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL_TOOL,
	PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL
};

enum {
	SELECTED_PATH_TYPE_NORMAL,
	SELECTED_PATH_TYPE_QUEUE
};

enum WINDOW_FOOTPATH_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,

	WIDX_TYPE_GROUP,
	WIDX_FOOTPATH_TYPE,
	WIDX_QUEUELINE_TYPE,

	WIDX_DIRECTION_GROUP,
	WIDX_DIRECTION_NW,
	WIDX_DIRECTION_NE,
	WIDX_DIRECTION_SW,
	WIDX_DIRECTION_SE,

	WIDX_SLOPE_GROUP,
	WIDX_SLOPEDOWN,
	WIDX_LEVEL,
	WIDX_SLOPEUP,
	WIDX_CONSTRUCT,
	WIDX_REMOVE,

	WIDX_MODE_GROUP,
	WIDX_CONSTRUCT_ON_LAND,
	WIDX_CONSTRUCT_BRIDGE_OR_TUNNEL,
};

static rct_widget window_footpath_widgets[] = {
	{ WWT_FRAME,	0,		0,		105,	0,		380,	0xFFFFFFFF,							STR_NONE },
	{ WWT_CAPTION,	0,		1,		104,	1,		14,		STR_FOOTPATHS,						STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,	0,		93,		103,	2,		13,		STR_CLOSE_X,						STR_CLOSE_WINDOW_TIP },
	// Type group
	{ WWT_GROUPBOX,	0,		3,		102,	17,		71,		STR_TYPE,							STR_NONE },
	{ WWT_FLATBTN,	1,		6,		52,		30,		65,		0xFFFFFFFF,							STR_FOOTPATH_TIP },
	{ WWT_FLATBTN,	1,		53,		99,		30,		65,		0xFFFFFFFF,							STR_QUEUE_LINE_PATH_TIP },

	// Direction group
	{ WWT_GROUPBOX,	0,		3,		102,	75,		151,	STR_DIRECTION,						STR_NONE },
	{ WWT_FLATBTN,	1,		53,		97,		87,		115,	SPR_CONSTRUCTION_DIRECTION_NE,		STR_DIRECTION_TIP },
	{ WWT_FLATBTN,	1,		53,		97,		116,	144,	SPR_CONSTRUCTION_DIRECTION_SE,		STR_DIRECTION_TIP },
	{ WWT_FLATBTN,	1,		8,		52,		116,	144,	SPR_CONSTRUCTION_DIRECTION_SW,		STR_DIRECTION_TIP },
	{ WWT_FLATBTN,	1,		8,		52,		87,		115,	SPR_CONSTRUCTION_DIRECTION_NW,		STR_DIRECTION_TIP },

	// Slope group
	{ WWT_GROUPBOX,	0,		3,		102,	155,	195,	STR_SLOPE,							STR_NONE },
	{ WWT_FLATBTN,	1,		17,		40,		167,	190,	SPR_RIDE_CONSTRUCTION_SLOPE_DOWN,	STR_SLOPE_DOWN_TIP },
	{ WWT_FLATBTN,	1,		41,		64,		167,	190,	SPR_RIDE_CONSTRUCTION_SLOPE_LEVEL,	STR_LEVEL_TIP },
	{ WWT_FLATBTN,	1,		65,		88,		167,	190,	SPR_RIDE_CONSTRUCTION_SLOPE_UP,		STR_SLOPE_UP_TIP },
	{ WWT_FLATBTN,	1,		8,		97,		202,	291,	0xFFFFFFFF,							STR_CONSTRUCT_THE_SELECTED_FOOTPATH_SECTION_TIP },
	{ WWT_FLATBTN,	1,		30,		75,		295,	318,	SPR_DEMOLISH_CURRENT_SECTION,		STR_REMOVE_PREVIOUS_FOOTPATH_SECTION_TIP },

	// Mode group
	{ WWT_GROUPBOX,	0,		3,		102,	321,	374,	0xFFFFFFFF,							STR_NONE },
	{ WWT_FLATBTN,	1,		13,		48,		332,	367,	SPR_CONSTRUCTION_FOOTPATH_LAND,		STR_CONSTRUCT_FOOTPATH_ON_LAND_TIP },
	{ WWT_FLATBTN,	1,		57,		92,		332,	367,	SPR_CONSTRUCTION_FOOTPATH_BRIDGE,	STR_CONSTRUCT_BRIDGE_OR_TUNNEL_FOOTPATH_TIP },
	{ WIDGETS_END },
};

static void window_footpath_close(rct_window *w);
static void window_footpath_mouseup(rct_window *w, sint32 widgetIndex);
static void window_footpath_mousedown(sint32 widgetIndex, rct_window *w, rct_widget *widget);
static void window_footpath_dropdown(rct_window *w, sint32 widgetIndex, sint32 dropdownIndex);
static void window_footpath_update(rct_window *w);
static void window_footpath_toolupdate(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y);
static void window_footpath_tooldown(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y);
static void window_footpath_tooldrag(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y);
static void window_footpath_toolup(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y);
static void window_footpath_invalidate(rct_window *w);
static void window_footpath_paint(rct_window *w, rct_drawpixelinfo *dpi);

static rct_window_event_list window_footpath_events = {
	window_footpath_close,
	window_footpath_mouseup,
	NULL,
	window_footpath_mousedown,
	window_footpath_dropdown,
	NULL,
	window_footpath_update,
	NULL,
	NULL,
	window_footpath_toolupdate,
	window_footpath_tooldown,
	window_footpath_tooldrag,
	window_footpath_toolup,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_footpath_invalidate,
	window_footpath_paint,
	NULL
};

static money32 _window_footpath_cost;
static sint8 _window_footpath_provisional_path_arrow_timer;
static uint8 _lastUpdatedCameraRotation = UINT8_MAX;
static bool _footpathErrorOccured;


enum
{
	FOOTHPATH_IS_SLOPED = (1 << 2),
	IRREGULAR_SLOPE = (1 << 3),
};

/** rct2: 0x0098D8B4 */
const uint8 default_path_slope[] = {
	0,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	FOOTHPATH_IS_SLOPED | 2,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	FOOTHPATH_IS_SLOPED | 3,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	FOOTHPATH_IS_SLOPED | 1,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	FOOTHPATH_IS_SLOPED | 0,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
	IRREGULAR_SLOPE,
};

/** rct2: 0x0098D7E0 */
uint8 footpath_construction_preview_images[][4] = {
	{5,  10, 5,  10}, // Flat
	{16, 17, 18, 19}, // Upwards
	{18, 19, 16, 17}, // Downwards
};

static void window_footpath_mousedown_direction(sint32 direction);
static void window_footpath_mousedown_slope(sint32 slope);
static void window_footpath_show_footpath_types_dialog(rct_window *w, rct_widget *widget, sint32 showQueues);
static void window_footpath_set_provisional_path_at_point(sint32 x, sint32 y);
static void window_footpath_set_selection_start_bridge_at_point(sint32 screenX, sint32 screenY);
static void window_footpath_place_path_at_point(sint32 x, sint32 y);
static void window_footpath_start_bridge_at_point(sint32 screenX, sint32 screenY);
static void window_footpath_construct();
static void window_footpath_remove();
static void window_footpath_set_enabled_and_pressed_widgets();
static void footpath_get_next_path_info(sint32 *type, sint32 *x, sint32 *y, sint32 *z, sint32 *slope);
static void footpath_select_default();

/**
 *
 *  rct2: 0x006A7C43
 */
void window_footpath_open()
{
	// Check if window is already open
	rct_window *window = window_bring_to_front_by_class(WC_FOOTPATH);
	if (window != NULL)
		return;

	window = window_create(
		0,
		29,
		106,
		381,
		&window_footpath_events,
		WC_FOOTPATH,
		0
	);
	window->widgets = window_footpath_widgets;
	window->enabled_widgets =
		(1 << WIDX_CLOSE) |
		(1 << WIDX_FOOTPATH_TYPE) |
		(1 << WIDX_QUEUELINE_TYPE) |
		(1 << WIDX_DIRECTION_NW) |
		(1 << WIDX_DIRECTION_NE) |
		(1 << WIDX_DIRECTION_SW) |
		(1 << WIDX_DIRECTION_SE) |
		(1 << WIDX_SLOPEDOWN) |
		(1 << WIDX_LEVEL) |
		(1 << WIDX_SLOPEUP) |
		(1 << WIDX_CONSTRUCT) |
		(1 << WIDX_REMOVE) |
		(1 << WIDX_CONSTRUCT_ON_LAND) |
		(1 << WIDX_CONSTRUCT_BRIDGE_OR_TUNNEL);

	window_init_scroll_widgets(window);
	window_push_others_right(window);
	show_gridlines();

	// If a restricted path was selected when the game is no longer in Sandbox mode, reset it
	rct_footpath_entry *pathEntry = get_footpath_entry(gFootpathSelectedId);
	if (pathEntry != (rct_footpath_entry*)-1 && (pathEntry->flags & FOOTPATH_ENTRY_FLAG_SHOW_ONLY_IN_SCENARIO_EDITOR) && !gCheatsSandboxMode) {
		pathEntry = (rct_footpath_entry*)-1;
	}

	// Select the default path if we don't have one
	if (pathEntry == (rct_footpath_entry*)-1) {
		footpath_select_default();
	}

	tool_cancel();
	gFootpathConstructionMode = PATH_CONSTRUCTION_MODE_LAND;
	tool_set(window, WIDX_CONSTRUCT_ON_LAND, 17);
	gInputFlags |= INPUT_FLAG_6;
	_footpathErrorOccured = false;
	window_footpath_set_enabled_and_pressed_widgets();
}

/**
 *
 *  rct2: 0x006A852F
 */
static void window_footpath_close(rct_window *w)
{
	footpath_provisional_update();
	viewport_set_visibility(0);
	map_invalidate_map_selection_tiles();
	gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_CONSTRUCT;
	window_invalidate_by_class(WC_TOP_TOOLBAR);
	hide_gridlines();
}

/**
 *
 *  rct2: 0x006A7E92
 */
static void window_footpath_mouseup(rct_window *w, sint32 widgetIndex)
{
	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_CONSTRUCT:
		window_footpath_construct();
		break;
	case WIDX_REMOVE:
		window_footpath_remove();
		break;
	case WIDX_CONSTRUCT_ON_LAND:
		if (gFootpathConstructionMode == PATH_CONSTRUCTION_MODE_LAND)
			break;

		_window_footpath_cost = MONEY32_UNDEFINED;
		tool_cancel();
		footpath_provisional_update();
		map_invalidate_map_selection_tiles();
		gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_CONSTRUCT;
		gFootpathConstructionMode = PATH_CONSTRUCTION_MODE_LAND;
		tool_set(w, WIDX_CONSTRUCT_ON_LAND, 17);
		gInputFlags |= INPUT_FLAG_6;
		_footpathErrorOccured = false;
		window_footpath_set_enabled_and_pressed_widgets();
		break;
	case WIDX_CONSTRUCT_BRIDGE_OR_TUNNEL:
		if (gFootpathConstructionMode == PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL_TOOL)
			break;

		_window_footpath_cost = MONEY32_UNDEFINED;
		tool_cancel();
		footpath_provisional_update();
		map_invalidate_map_selection_tiles();
		gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_CONSTRUCT;
		gFootpathConstructionMode = PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL_TOOL;
		tool_set(w, WIDX_CONSTRUCT_BRIDGE_OR_TUNNEL, 12);
		gInputFlags |= INPUT_FLAG_6;
		_footpathErrorOccured = false;
		window_footpath_set_enabled_and_pressed_widgets();
		break;
	}
}

/**
 *
 *  rct2: 0x006A7EC5
 */
static void window_footpath_mousedown(sint32 widgetIndex, rct_window*w, rct_widget* widget)
{
	switch (widgetIndex) {
	case WIDX_FOOTPATH_TYPE:
		window_footpath_show_footpath_types_dialog(w, widget, 0);
		break;
	case WIDX_QUEUELINE_TYPE:
		window_footpath_show_footpath_types_dialog(w, widget, 1);
		break;
	case WIDX_DIRECTION_NW:
		window_footpath_mousedown_direction(0);
		break;
	case WIDX_DIRECTION_NE:
		window_footpath_mousedown_direction(1);
		break;
	case WIDX_DIRECTION_SW:
		window_footpath_mousedown_direction(2);
		break;
	case WIDX_DIRECTION_SE:
		window_footpath_mousedown_direction(3);
		break;
	case WIDX_SLOPEDOWN:
		window_footpath_mousedown_slope(6);
		break;
	case WIDX_LEVEL:
		window_footpath_mousedown_slope(0);
		break;
	case WIDX_SLOPEUP:
		window_footpath_mousedown_slope(2);
		break;
	}
}

/**
 *
 *  rct2: 0x006A7F18
 */
static void window_footpath_dropdown(rct_window *w, sint32 widgetIndex, sint32 dropdownIndex)
{
	if (widgetIndex == WIDX_FOOTPATH_TYPE)
		gFootpathSelectedType = SELECTED_PATH_TYPE_NORMAL;
	else if (widgetIndex == WIDX_QUEUELINE_TYPE)
		gFootpathSelectedType = SELECTED_PATH_TYPE_QUEUE;
	else
		return;

	// Get path id
	sint32 pathId = dropdownIndex;
	if (pathId == -1) {
		pathId = gFootpathSelectedId;
	} else {
		sint32 flags = 4;
		if ((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) || gCheatsSandboxMode)
			flags = 0;

		sint32 i = 0, j = 0;
		for (; i < 16; i++) {
			rct_footpath_entry *pathType = get_footpath_entry(i);
			if (pathType == (rct_footpath_entry*)-1)
				continue;
			if (pathType->flags & flags)
				continue;
			// Skip queue lines of scenario editor-only paths (only applicable when the game is in sandbox mode)
			if(widgetIndex == WIDX_QUEUELINE_TYPE && (pathType->flags & FOOTPATH_ENTRY_FLAG_SHOW_ONLY_IN_SCENARIO_EDITOR))
				continue;

			if (j == pathId)
				break;
			j++;
		}
		pathId = i;
	}

	// Set selected path id
	gFootpathSelectedId = pathId;
	footpath_provisional_update();
	_window_footpath_cost = MONEY32_UNDEFINED;
	window_invalidate(w);
}

/**
 *
 *  rct2: 0x006A8032
 */
static void window_footpath_toolupdate(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y)
{
	if (widgetIndex == WIDX_CONSTRUCT_ON_LAND) {
		window_footpath_set_provisional_path_at_point(x, y);
	} else if (widgetIndex == WIDX_CONSTRUCT_BRIDGE_OR_TUNNEL) {
		window_footpath_set_selection_start_bridge_at_point(x, y);
	}
}

/**
 *
 *  rct2: 0x006A8047
 */
static void window_footpath_tooldown(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y)
{
	if (widgetIndex == WIDX_CONSTRUCT_ON_LAND)
		window_footpath_place_path_at_point(x, y);
	else if (widgetIndex == WIDX_CONSTRUCT_BRIDGE_OR_TUNNEL)
		window_footpath_start_bridge_at_point(x, y);
}

/**
 *
 *  rct2: 0x006A8067
 */
static void window_footpath_tooldrag(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y)
{
	if (widgetIndex == WIDX_CONSTRUCT_ON_LAND) {
		window_footpath_place_path_at_point(x, y);
	}
}

/**
 *
 *  rct2: 0x006A8066
 */
static void window_footpath_toolup(rct_window* w, sint32 widgetIndex, sint32 x, sint32 y)
{
	if (widgetIndex == WIDX_CONSTRUCT_ON_LAND) {
		_footpathErrorOccured = false;
	}
}

/**
 *
 *  rct2: 0x006A7760
 */
static void window_footpath_update_provisional_path_for_bridge_mode(rct_window *w)
{
	sint32 type, x, y, z, slope;

	if (gFootpathConstructionMode != PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL)
		return;

	// Recheck area for construction. Set by ride_construction window
	if (gFootpathProvisionalFlags & PROVISIONAL_PATH_FLAG_2) {
		footpath_provisional_remove();
		gFootpathProvisionalFlags &= ~PROVISIONAL_PATH_FLAG_2;
	}

	// Update provisional bridge mode path
	if (!(gFootpathProvisionalFlags & PROVISIONAL_PATH_FLAG_1)) {
		footpath_get_next_path_info(&type, &x, &y, &z, &slope);
		_window_footpath_cost = footpath_provisional_set(type, x, y, z, slope);
		widget_invalidate(w, WIDX_CONSTRUCT);
	}

	// Update little directional arrow on provisional bridge mode path
	if (--_window_footpath_provisional_path_arrow_timer < 0) {
		_window_footpath_provisional_path_arrow_timer = 5;
		gFootpathProvisionalFlags ^= PROVISIONAL_PATH_FLAG_SHOW_ARROW;
		footpath_get_next_path_info(&type, &x, &y, &z, &slope);
		gMapSelectArrowPosition.x = x;
		gMapSelectArrowPosition.y = y;
		gMapSelectArrowPosition.z = z * 8;
		gMapSelectArrowDirection = gFootpathConstructDirection;
		if (gFootpathProvisionalFlags & PROVISIONAL_PATH_FLAG_SHOW_ARROW)
			gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE_ARROW;
		else
			gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_ARROW;
		map_invalidate_tile_full(x, y);
	}
}

/**
 *
 *  rct2: 0x006A84BB
 */
static void window_footpath_update(rct_window *w)
{
	widget_invalidate(w, WIDX_CONSTRUCT);
	window_footpath_update_provisional_path_for_bridge_mode(w);

	// #2502: The camera might have changed rotation, so we need to update which directional buttons are pressed
	uint8 currentRotation = get_current_rotation();
	if (_lastUpdatedCameraRotation != currentRotation) {
		_lastUpdatedCameraRotation = currentRotation;
		window_footpath_set_enabled_and_pressed_widgets();
	}

	// Check tool
	if (gFootpathConstructionMode == PATH_CONSTRUCTION_MODE_LAND) {
		if (!(gInputFlags & INPUT_FLAG_TOOL_ACTIVE))
			window_close(w);
		else if (gCurrentToolWidget.window_classification != WC_FOOTPATH)
			window_close(w);
		else if (gCurrentToolWidget.widget_index != WIDX_CONSTRUCT_ON_LAND)
			window_close(w);
	} else if (gFootpathConstructionMode == PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL_TOOL) {
		if (!(gInputFlags & INPUT_FLAG_TOOL_ACTIVE))
			window_close(w);
		else if (gCurrentToolWidget.window_classification != WC_FOOTPATH)
			window_close(w);
		else if (gCurrentToolWidget.widget_index != WIDX_CONSTRUCT_BRIDGE_OR_TUNNEL)
			window_close(w);
	}
}

/**
 *
 *  rct2: 0x006A7D1C
 */
static void window_footpath_invalidate(rct_window *w)
{
	sint32 selectedPath;
	rct_footpath_entry *pathType;

	colour_scheme_update(w);

	// Press / unpress footpath and queue type buttons
	w->pressed_widgets &= ~(1 << WIDX_FOOTPATH_TYPE);
	w->pressed_widgets &= ~(1 << WIDX_QUEUELINE_TYPE);
	w->pressed_widgets |= gFootpathSelectedType == SELECTED_PATH_TYPE_NORMAL ?
		(1 << WIDX_FOOTPATH_TYPE) :
		(1 << WIDX_QUEUELINE_TYPE);

	// Enable / disable construct button
	window_footpath_widgets[WIDX_CONSTRUCT].type =
		gFootpathConstructionMode == PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL ?
			WWT_IMGBTN : WWT_EMPTY;

	// Set footpath and queue type button images
	selectedPath = gFootpathSelectedId;
	pathType = get_footpath_entry(selectedPath);

	// TODO: Should probably add constants for object sprites
	sint32 pathImage = 71 + pathType->image;
	window_footpath_widgets[WIDX_FOOTPATH_TYPE].image = pathImage;

	// Disable queue line button when the path is scenario editor-only (and therefore usually shouldn't have one)
	if(!(pathType->flags & FOOTPATH_ENTRY_FLAG_SHOW_ONLY_IN_SCENARIO_EDITOR)) {
		window_footpath_widgets[WIDX_QUEUELINE_TYPE].image = pathImage + 1;
		window_footpath_widgets[WIDX_QUEUELINE_TYPE].type = WWT_FLATBTN;
	} else {
		window_footpath_widgets[WIDX_QUEUELINE_TYPE].type = WWT_EMPTY;
	}

	// Disable queue line button if in Scenario Editor
	if (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR)
		window_footpath_widgets[WIDX_QUEUELINE_TYPE].type = WWT_EMPTY;
}

/**
 *
 *  rct2: 0x006A7D8B
 */
static void window_footpath_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	window_draw_widgets(w, dpi);

	if (!(w->disabled_widgets & (1 << WIDX_CONSTRUCT))) {
		// Get construction image
		uint8 direction = (gFootpathConstructDirection + get_current_rotation()) % 4;
		uint8 slope = 0;
		if (gFootpathConstructSlope == 2)
			slope = 1;
		else if (gFootpathConstructSlope == 6)
			slope = 2;
		sint32 image = footpath_construction_preview_images[slope][direction];

		sint32 selectedPath = gFootpathSelectedId;
		rct_footpath_entry *pathType = get_footpath_entry(selectedPath);
		image += pathType->image;
		if (gFootpathSelectedType != SELECTED_PATH_TYPE_NORMAL)
			image += 51;

		// Draw construction image
		sint32 x = w->x + (window_footpath_widgets[WIDX_CONSTRUCT].left + window_footpath_widgets[WIDX_CONSTRUCT].right) / 2;
		sint32 y = w->y + window_footpath_widgets[WIDX_CONSTRUCT].bottom - 60;
		gfx_draw_sprite(dpi, image, x, y, 0);

		// Draw build this... label
		x = w->x + (window_footpath_widgets[WIDX_CONSTRUCT].left + window_footpath_widgets[WIDX_CONSTRUCT].right) / 2;
		y = w->y + window_footpath_widgets[WIDX_CONSTRUCT].bottom - 23;
		gfx_draw_string_centred(dpi, STR_BUILD_THIS, x, y, COLOUR_BLACK, NULL);
	}

	// Draw cost
	sint32 x = w->x + (window_footpath_widgets[WIDX_CONSTRUCT].left + window_footpath_widgets[WIDX_CONSTRUCT].right) / 2;
	sint32 y = w->y + window_footpath_widgets[WIDX_CONSTRUCT].bottom - 12;
	if (_window_footpath_cost != MONEY32_UNDEFINED)
		if (!(gParkFlags & PARK_FLAGS_NO_MONEY))
			gfx_draw_string_centred(dpi, STR_COST_LABEL, x, y, COLOUR_BLACK, &_window_footpath_cost);
}

/**
 *
 *  rct2: 0x006A7F88
 */
static void window_footpath_show_footpath_types_dialog(rct_window *w, rct_widget *widget, sint32 showQueues)
{
	sint32 i, flags, numPathTypes, image;
	rct_footpath_entry *pathType;

	numPathTypes = 0;
	flags = FOOTPATH_ENTRY_FLAG_SHOW_ONLY_IN_SCENARIO_EDITOR;
	// If the game is in sandbox mode, also show paths that are normally restricted to the scenario editor, but not their queues (since these usually shouldn't have one)
	if ((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) || (gCheatsSandboxMode && !showQueues))
		flags = 0;

	for (i = 0; i < 16; i++) {
		pathType = get_footpath_entry(i);
		if (pathType == (rct_footpath_entry*)-1)
			continue;
		if (pathType->flags & flags)
			continue;

		image = pathType->image + 71;
		if (showQueues)
			image++;

		gDropdownItemsFormat[numPathTypes] = -1;
		gDropdownItemsArgs[numPathTypes] = image;
		numPathTypes++;
	}

	window_dropdown_show_image(
		w->x + widget->left, w->y + widget->top, widget->bottom - widget->top + 1,
		w->colours[1],
		0,
		numPathTypes,
		47,
		36,
		gAppropriateImageDropdownItemsPerRow[numPathTypes]
	);
}

/**
 *
 *  rct2: 0x006A8111 0x006A8135 0x006A815C 0x006A8183
 */
static void window_footpath_mousedown_direction(sint32 direction)
{
	footpath_provisional_update();
	gFootpathConstructDirection = (direction - get_current_rotation()) & 3;
	_window_footpath_cost = MONEY32_UNDEFINED;
	window_footpath_set_enabled_and_pressed_widgets();
}

/**
*
*  rct2: 0x006A81AA 0x006A81C5 0x006A81E0
*/
static void window_footpath_mousedown_slope(sint32 slope)
{
	footpath_provisional_update();
	gFootpathConstructSlope = slope;
	_window_footpath_cost = MONEY32_UNDEFINED;
	window_footpath_set_enabled_and_pressed_widgets();
}

/**
 *
 *  rct2: 0x006A81FB
 */
static void window_footpath_set_provisional_path_at_point(sint32 x, sint32 y)
{
	map_invalidate_selection_rect();
	gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_ARROW;

	sint32 interactionType;
	rct_map_element *mapElement;
	rct_xy16 mapCoord = { 0 };
	get_map_coordinates_from_pos(x, y, VIEWPORT_INTERACTION_MASK_FOOTPATH & VIEWPORT_INTERACTION_MASK_TERRAIN, &mapCoord.x, &mapCoord.y, &interactionType, &mapElement, NULL);
	x = mapCoord.x;
	y = mapCoord.y;

	if (interactionType == VIEWPORT_INTERACTION_ITEM_NONE) {
		gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
		footpath_provisional_update();
	} else {
		// Check for change
		if (
			(gFootpathProvisionalFlags & PROVISIONAL_PATH_FLAG_1) &&
			gFootpathProvisionalPosition.x == x &&
			gFootpathProvisionalPosition.y == y &&
			gFootpathProvisionalPosition.z == mapElement->base_height
		) {
			return;
		}

		// Set map selection
		gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
		gMapSelectType = MAP_SELECT_TYPE_FULL;
		gMapSelectPositionA.x = x;
		gMapSelectPositionA.y = y;
		gMapSelectPositionB.x = x;
		gMapSelectPositionB.y = y;

		footpath_provisional_update();

		// Set provisional path
		sint32 slope = default_path_slope[mapElement->properties.surface.slope & 0x1F];
		if (interactionType == VIEWPORT_INTERACTION_ITEM_FOOTPATH)
			slope = mapElement->properties.surface.slope & 7;
		sint32 pathType = (gFootpathSelectedType << 7) + (gFootpathSelectedId & 0xFF);

		_window_footpath_cost = footpath_provisional_set(pathType, x, y, mapElement->base_height, slope);
		window_invalidate_by_class(WC_FOOTPATH);
	}
}

/**
*
*  rct2: 0x006A8388
*/
static void window_footpath_set_selection_start_bridge_at_point(sint32 screenX, sint32 screenY)
{
	sint32 x, y, direction;
	rct_map_element *mapElement;

	map_invalidate_selection_rect();
	gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
	gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_ARROW;

	footpath_bridge_get_info_from_pos(screenX, screenY, &x, &y, &direction, &mapElement);
	if (x == MAP_LOCATION_NULL)
		return;

	gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
	gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE_ARROW;
	gMapSelectType = MAP_SELECT_TYPE_FULL;
	gMapSelectPositionA.x = x;
	gMapSelectPositionB.x = x;
	gMapSelectPositionA.y = y;
	gMapSelectPositionB.y = y;

	gMapSelectArrowDirection = direction;
	gMapSelectArrowPosition.x = x;
	gMapSelectArrowPosition.y = y;

	sint32 z = mapElement->base_height;

	if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_SURFACE) {
		uint8 slope = mapElement->properties.surface.slope;
		if (slope & 0xf)
			z += 2; // Add 2 for a slope
		if (slope & 0x10)
			z += 2; // Add another 2 for a steep slope
	}

	gMapSelectArrowPosition.z = z << 3;

	map_invalidate_selection_rect();
}

/**
 *
 *  rct2: 0x006A82C5
 */
static void window_footpath_place_path_at_point(sint32 x, sint32 y)
{
	sint32 interactionType, presentType, selectedType, z, cost;
	rct_map_element *mapElement;

	if (_footpathErrorOccured)
		return;

	footpath_provisional_update();

	rct_xy16 mapCoord = { 0 };
	get_map_coordinates_from_pos(x, y, VIEWPORT_INTERACTION_MASK_FOOTPATH & VIEWPORT_INTERACTION_MASK_TERRAIN, &mapCoord.x, &mapCoord.y, &interactionType, &mapElement, NULL);
	x = mapCoord.x;
	y = mapCoord.y;

	if (interactionType == VIEWPORT_INTERACTION_ITEM_NONE)
		return;

	// Set path
	presentType = default_path_slope[mapElement->properties.path.type & 0x1F];
	if (interactionType == VIEWPORT_INTERACTION_ITEM_FOOTPATH)
		presentType = mapElement->properties.path.type & 7;
	z = mapElement->base_height;
	selectedType = (gFootpathSelectedType << 7) + (gFootpathSelectedId & 0xFF);

	// Try and place path
	gGameCommandErrorTitle = STR_CANT_BUILD_FOOTPATH_HERE;
	cost = footpath_place(selectedType, x, y, z, presentType, GAME_COMMAND_FLAG_APPLY);

	if (cost == MONEY32_UNDEFINED) {
		_footpathErrorOccured = true;
	} else if (gFootpathPrice != 0) {
		// bp = RCT2_ADDRESS_COMMAND_MAP_Z
		// dx = RCT2_ADDRESS_COMMAND_MAP_Y
		// cx = RCT2_ADDRESS_COMMAND_MAP_X
		audio_play_sound_at_location(SOUND_PLACE_ITEM, gCommandPosition.x, gCommandPosition.y, gCommandPosition.z);
	}
}

/**
 *
 *  rct2: 0x006A840F
 */
static void window_footpath_start_bridge_at_point(sint32 screenX, sint32 screenY)
{
	sint32 x, y, z, direction;
	rct_map_element *mapElement;

	footpath_bridge_get_info_from_pos(screenX, screenY, &x, &y, &direction, &mapElement);
	if (x == MAP_LOCATION_NULL)
		return;

	if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_SURFACE) {
		// If we start the path on a slope, the arrow is slightly raised, so we
		// expect the path to be slightly raised as well.
		uint8 slope = mapElement->properties.surface.slope;
		z = mapElement->base_height;
		if (slope & 0x10) {
			// Steep diagonal slope
			z += 4;
		} else if (slope & 0x0f) {
			// Normal slope
			z += 2;
		}
	} else {
		z = mapElement->base_height;
		if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH) {
			if (mapElement->properties.path.type & 4) {
				if (direction == (mapElement->properties.path.type & 3))
					z += 2;
			}
		}
	}

	tool_cancel();
	gFootpathConstructFromPosition.x = x;
	gFootpathConstructFromPosition.y = y;
	gFootpathConstructFromPosition.z = z * 8;
	gFootpathConstructDirection = direction;
	gFootpathProvisionalFlags = 0;
	_window_footpath_provisional_path_arrow_timer = 0;
	gFootpathConstructSlope = 0;
	gFootpathConstructionMode = PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL;
	gFootpathConstructValidDirections = 255;
	window_footpath_set_enabled_and_pressed_widgets();
}

/**
 * Construct a piece of footpath while in bridge building mode.
 *  rct2: 0x006A79B7
 */
static void window_footpath_construct()
{
	_window_footpath_cost = MONEY32_UNDEFINED;
	footpath_provisional_update();

	sint32 type, x, y, z, slope;
	footpath_get_next_path_info(&type, &x, &y, &z, &slope);

	// Try to place the path at the desired location
	gGameCommandErrorTitle = STR_CANT_BUILD_FOOTPATH_HERE;
	money32 cost = footpath_place(type, x, y, z, slope, 0);

	if (cost != MONEY32_UNDEFINED && !gCheatsDisableClearanceChecks) {
		// It is possible, let's remove walls between the old and new piece of path
		uint8 direction = gFootpathConstructDirection;
		wall_remove_intersecting_walls(x, y, z, z + 4 + ((slope & 0xf) ? 2 : 0), direction ^ 2);
		wall_remove_intersecting_walls(
			x - TileDirectionDelta[direction].x,
			y - TileDirectionDelta[direction].y,
			z, z + 4, direction
		);
	}

	// Actually place the path now
	gGameCommandErrorTitle = STR_CANT_BUILD_FOOTPATH_HERE;
	cost = footpath_place(type, x, y, z, slope, GAME_COMMAND_FLAG_APPLY);

	if (cost != MONEY32_UNDEFINED) {
		audio_play_sound_at_location(
			SOUND_PLACE_ITEM,
			gFootpathConstructFromPosition.x,
			gFootpathConstructFromPosition.y,
			gFootpathConstructFromPosition.z
		);

		if (gFootpathConstructSlope == 0) {
			gFootpathConstructValidDirections = 0xFF;
		} else {
			gFootpathConstructValidDirections = gFootpathConstructDirection;
		}

		if (gFootpathGroundFlags & ELEMENT_IS_UNDERGROUND)
			viewport_set_visibility(1);

		// If we have just built an upwards slope, the next path to construct is
		// a bit higher. Note that the z returned by footpath_get_next_path_info
		// already is lowered if we are building a downwards slope.
		if (gFootpathConstructSlope == 2)
			z += 2;

		gFootpathConstructFromPosition.x = x;
		gFootpathConstructFromPosition.y = y;
		gFootpathConstructFromPosition.z = z << 3;
	}

	window_footpath_set_enabled_and_pressed_widgets();
}

/**
 *
 *  rct2: 0x006A78EF
 */
static void footpath_remove_map_element(rct_map_element *mapElement)
{
	sint32 x, y, z;

	z = mapElement->base_height;
	sint32 pathType = mapElement->properties.path.type;
	if (pathType & 4) {
		pathType &= 3;
		pathType ^= 2;
		if (pathType == gFootpathConstructDirection)
			z += 2;
	}

	// Find a connected edge
	sint32 edge = gFootpathConstructDirection ^ 2;
	if (!(mapElement->properties.path.edges & (1 << edge))) {
		edge = (edge + 1) & 3;
		if (!(mapElement->properties.path.edges & (1 << edge))) {
			edge = (edge + 2) & 3;
			if (!(mapElement->properties.path.edges & (1 << edge))) {
				edge = (edge - 1) & 3;
				if (!(mapElement->properties.path.edges & (1 << edge)))
					edge ^= 2;
			}
		}
	}

	// Remove path
	gGameCommandErrorTitle = STR_CANT_REMOVE_FOOTPATH_FROM_HERE;
	footpath_remove(
		gFootpathConstructFromPosition.x,
		gFootpathConstructFromPosition.y,
		mapElement->base_height,
		1
	);

	// Move selection
	edge ^= 2;
	x = gFootpathConstructFromPosition.x - TileDirectionDelta[edge].x;
	y = gFootpathConstructFromPosition.y - TileDirectionDelta[edge].y;
	gFootpathConstructFromPosition.x = x;
	gFootpathConstructFromPosition.y = y;
	gFootpathConstructFromPosition.z = z << 3;
	gFootpathConstructDirection = edge;
	gFootpathConstructValidDirections = 255;
}

/**
 *
 *  rct2: 0x006A7873
 */
static rct_map_element *footpath_get_map_element_to_remove()
{
	rct_map_element *mapElement;
	sint32 x, y, z, zLow;

	x = gFootpathConstructFromPosition.x / 32;
	y = gFootpathConstructFromPosition.y / 32;
	if (x >= 256 || y >= 256)
		return NULL;

	z = (gFootpathConstructFromPosition.z >> 3) & 0xFF;
	zLow = z - 2;

	mapElement = map_get_first_element_at(x, y);
	do {
		if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH) {
			if (mapElement->base_height == z) {
				if (mapElement->properties.path.type & 4)
					if (((mapElement->properties.path.type & 3) ^ 2) != gFootpathConstructDirection)
						continue;

				return mapElement;
			} else if (mapElement->base_height == zLow) {
				if (!(mapElement->properties.path.type & 4))
					if ((mapElement->properties.path.type & 3) == gFootpathConstructDirection)
						continue;

				return mapElement;
			}
		}
	} while (!map_element_is_last_for_tile(mapElement++));

	return NULL;
}

/**
 *
 *  rct2: 0x006A7863
 */
static void window_footpath_remove()
{
	rct_map_element *mapElement;

	_window_footpath_cost = MONEY32_UNDEFINED;
	footpath_provisional_update();

	mapElement = footpath_get_map_element_to_remove();
	if (mapElement != NULL)
		footpath_remove_map_element(mapElement);

	window_footpath_set_enabled_and_pressed_widgets();
}

/**
 *
 *  rct2: 0x006A855C
 */
static void window_footpath_set_enabled_and_pressed_widgets()
{
	rct_window *w = window_find_by_class(WC_FOOTPATH);
	if (w == NULL)
		return;

	if (gFootpathConstructionMode == PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL) {
		map_invalidate_map_selection_tiles();
		gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE_CONSTRUCT;
		gMapSelectFlags |= MAP_SELECT_FLAG_GREEN;

		sint32 direction = gFootpathConstructDirection;
		gMapSelectionTiles[0].x = gFootpathConstructFromPosition.x + TileDirectionDelta[direction].x;
		gMapSelectionTiles[0].y = gFootpathConstructFromPosition.y + TileDirectionDelta[direction].y;
		gMapSelectionTiles[1].x = -1;
		map_invalidate_map_selection_tiles();
	}

	uint64 pressedWidgets = w->pressed_widgets & 0xFFFF887F;
	uint64 disabledWidgets = 0;
	sint32 currentRotation = get_current_rotation();
	if (gFootpathConstructionMode >= PATH_CONSTRUCTION_MODE_BRIDGE_OR_TUNNEL) {
		// Set pressed directional widget
		sint32 direction = (gFootpathConstructDirection + currentRotation) & 3;
		pressedWidgets |= (1LL << (WIDX_DIRECTION_NW + direction));

		// Set pressed slope widget
		sint32 slope = gFootpathConstructSlope;
		if (slope == 6)
			pressedWidgets |= (1 << WIDX_SLOPEDOWN);
		else if (slope == 0)
			pressedWidgets |= (1 << WIDX_LEVEL);
		else
			pressedWidgets |= (1 << WIDX_SLOPEUP);

		// Enable / disable directional widgets
		direction = gFootpathConstructValidDirections;
		if (direction != 255) {
			disabledWidgets |=
				(1 << WIDX_DIRECTION_NW) |
				(1 << WIDX_DIRECTION_NE) |
				(1 << WIDX_DIRECTION_SW) |
				(1 << WIDX_DIRECTION_SE);

			direction = (direction + currentRotation) & 3;
			disabledWidgets &= ~(1 << (WIDX_DIRECTION_NW + direction));
		}
	} else {
		// Disable all bridge mode widgets
		disabledWidgets |=
			(1 << WIDX_DIRECTION_GROUP) |
			(1 << WIDX_DIRECTION_NW) |
			(1 << WIDX_DIRECTION_NE) |
			(1 << WIDX_DIRECTION_SW) |
			(1 << WIDX_DIRECTION_SE) |
			(1 << WIDX_SLOPE_GROUP) |
			(1 << WIDX_SLOPEDOWN) |
			(1 << WIDX_LEVEL) |
			(1 << WIDX_SLOPEUP) |
			(1 << WIDX_CONSTRUCT) |
			(1 << WIDX_REMOVE);
	}

	w->pressed_widgets = pressedWidgets;
	w->disabled_widgets = disabledWidgets;
	window_invalidate(w);
}

/**
 *
 *  rct2: 0x006A7B20
 */
static void footpath_get_next_path_info(sint32 *type, sint32 *x, sint32 *y, sint32 *z, sint32 *slope)
{
	sint32 direction;

	direction = gFootpathConstructDirection;
	*x = gFootpathConstructFromPosition.x + TileDirectionDelta[direction].x;
	*y = gFootpathConstructFromPosition.y + TileDirectionDelta[direction].y;
	*z = gFootpathConstructFromPosition.z / 8;
	*type = (gFootpathSelectedType << 7) + (gFootpathSelectedId & 0xFF);
	*slope = 0;
	if (gFootpathConstructSlope != 0) {
		*slope = gFootpathConstructDirection | 4;
		if (gFootpathConstructSlope != 2) {
			*z -= 2;
			*slope ^= 2;
		}
	}
}

static void footpath_select_default()
{
	// Select first available footpath
	gFootpathSelectedId = 0;
	for (sint32 i = 0; i < object_entry_group_counts[OBJECT_TYPE_PATHS]; i++) {
		rct_footpath_entry *pathEntry = get_footpath_entry(i);
		if (pathEntry != (rct_footpath_entry*)-1) {
			gFootpathSelectedId = i;

			// Prioritise non-restricted path
			if (!(pathEntry->flags & FOOTPATH_ENTRY_FLAG_SHOW_ONLY_IN_SCENARIO_EDITOR)) {
				break;
			}
		}
	}
}
