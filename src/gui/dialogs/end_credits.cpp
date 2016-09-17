/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/end_credits.hpp"

#include "about.hpp"
#include "config.hpp"
#include "config_assign.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/repeating_button.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/scrollbar_panel.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "marked-up_text.hpp"

#include "utils/functional.hpp"

#include <sstream>

namespace gui2
{

REGISTER_DIALOG(end_credits)

tend_credits::tend_credits(const std::string& campaign)
	: focus_on_(campaign)
	, backgrounds_()
	, timer_id_()
	, text_widget_(nullptr)
	, scroll_speed_(100)
{
}

tend_credits::~tend_credits()
{
	if(timer_id_ != 0) {
		remove_timer(timer_id_);
		timer_id_ = 0;
	}
}

static void parse_about_tags(const config& cfg, std::stringstream& str)
{
	for(const auto& about : cfg.child_range("about")) {
		if(!about.has_child("entry")) {
			continue;
		}

		str << "\n" << "<span size='x-large'>" << about["title"] << "</span>" << "\n";

		for(const auto& entry : about.child_range("entry")) {
			str << entry["name"] << "\n";
		}
	}
}

static void init_grid(tbuilder_grid& g)
{
	g.rows = g.widgets.size();
	g.cols = 1;
	g.row_grow_factor.resize(g.rows, 0);
	g.col_grow_factor.resize(g.cols, 0);
	g.flags.resize(g.rows, tgrid::BORDER_TOP | tgrid::BORDER_BOTTOM | tgrid::HORIZONTAL_ALIGN_CENTER | tgrid::VERTICAL_ALIGN_CENTER);
	g.border_size.resize(g.rows, 5);
}

void tend_credits::pre_show(twindow& window)
{
	// Delay a little before beginning the scrolling
	add_timer(3000, [this](size_t) {
		timer_id_ = add_timer(10, std::bind(&tend_credits::timer_callback, this), true);
		last_scroll_ = SDL_GetTicks();
	});

	connect_signal_pre_key_press(window, std::bind(&tend_credits::key_press_callback, this, _3, _4, _5));

	const config& credits_config = about::get_about_config();
	auto credit_sections = std::make_shared<tbuilder_grid>(config_of("id", "text"));

	// First, parse all the toplevel [about] tags
	credit_sections->widgets.push_back(std::make_shared<tbuilder_credits_grid>(credits_config));

	// Next, parse all the grouped [about] tags (usually by campaign)
	for(const auto& group : credits_config.child_range("credits_group")) {
		auto builder = std::make_shared<tbuilder_credits_grid>(group);
		if(group["id"] == focus_on_) {
			credit_sections->widgets.insert(credit_sections->widgets.begin(), builder);
		} else {
			credit_sections->widgets.push_back(builder);
		}
	}

	// Set flags, border sizes, grow factors
	init_grid(*credit_sections);

	// Get the appropriate background images
	backgrounds_ = about::get_background_images(focus_on_);

	if(backgrounds_.empty()) {
		backgrounds_.push_back(game_config::images::game_title_background);
	}

	// TODO: implement showing all available images as the credits scroll
	window.canvas()[0].set_variable("background_image", variant(backgrounds_[0]));

	tgrid& text_panel = get_parent<tgrid>(find_widget<twidget>(&window, "text", false));
	auto text_area = std::make_shared<implementation::tbuilder_scrollbar_panel>(config_of
		("id", "text")
		("definition", "default")
		("horizontal_scrollbar_mode", "never")
		("vertical_scrollbar_mode", "always")
		("definition", config())
	);
	text_area->grid = credit_sections;
	text_widget_ = text_area->build();
	//text_widget_->set_use_markup(true);
	delete text_panel.swap_child("text", text_widget_, false);
//	text_widget_->set_label(str.str());

	// HACK: always hide the scrollbar, even if it's needed.
	// This should probably be implemented as a scrollbar mode.
	// Also, for some reason hiding the whole grid doesn't work, and the elements need to be hidden manually
	if(tgrid* v_grid = dynamic_cast<tgrid*>(text_widget_->find("_vertical_scrollbar_grid", false))) {
		find_widget<tscrollbar_>(v_grid, "_vertical_scrollbar", false).set_visible(twidget::tvisible::hidden);
		find_widget<trepeating_button>(v_grid, "_half_page_up", false).set_visible(twidget::tvisible::hidden);
		find_widget<trepeating_button>(v_grid, "_half_page_down", false).set_visible(twidget::tvisible::hidden);
	}
}

void tend_credits::timer_callback()
{
	uint32_t now = SDL_GetTicks();
	uint32_t missed_time = now - last_scroll_;

	unsigned int cur_pos = text_widget_->get_vertical_scrollbar_item_position();

	// Calculate how far the text should have scrolled by now
	// The division by 1000 is to convert milliseconds to seconds.
	unsigned int needed_dist = missed_time * scroll_speed_ / 1000;

	text_widget_->set_vertical_scrollbar_item_position(cur_pos + needed_dist);

	last_scroll_ = now;

	if(text_widget_->vertical_scrollbar_at_end()) {
		remove_timer(timer_id_);
	}
}

void tend_credits::key_press_callback(bool&, bool&, const SDL_Keycode key)
{
	if(key == SDLK_UP && scroll_speed_ < 400) {
		scroll_speed_ <<= 1;
	}

	if(key == SDLK_DOWN && scroll_speed_ > 25) {
		scroll_speed_ >>= 1;
	}
}

static tbuilder_widget_ptr make_label(std::string text, std::string size = "")
{
	if(!size.empty()) {
		text = "<span size='" + size + "'>" + text + "</span>";
	}
	config cfg = config_of("label", config_of
		("label", text)
	);
	return create_builder_widget(cfg);
}

tbuilder_credits_grid::tbuilder_credits_grid(const config& cfg)
	: tbuilder_grid(config())
{
	// cfg is either a [credits_group] or the toplevel about config
	auto sections = cfg.child_range("about");
	bool have_title = cfg.has_attribute("title");
	rows = sections.size() + have_title;

	// First add the name, if present
	id = "credits";
	if(have_title) {
		widgets.push_back(make_label(cfg["title"], "xx-large"));
		id += '_' + cfg["title"];
	}
	for(const config& section : sections) {
		std::ostringstream text;
		if(section.has_attribute("title")) {
			text << "<span size='x-large'>" << section["title"] << "</span>\n";
		}
		for(const config& entry : section.child_range("entry")) {
			text << entry["name"] << "\n";
		}
		widgets.push_back(make_label(text.str()));
	}
	init_grid(*this);
}

} // namespace gui2
