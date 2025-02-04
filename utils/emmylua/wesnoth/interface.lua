---@meta

---Freeze the game for a specified time
---@param ms number
function wesnoth.interface.delay(ms) end

---Deselect the active hex, if any
function wesnoth.interface.deselect_hex() end

---Activate a specific hex
---@param loc location
---@overload fun(x:integer, y:integer)
function wesnoth.interface.highlight_hex(loc) end

---Animate a floating label on a hex
---@param loc location
---@param text tstring
---@overload fun(x:integer, y:integer, text:tstring)
function wesnoth.interface.float_label(loc, text) end

---Get the currently-selected unit, the one displayed in the sidebar
---@return unit
function wesnoth.interface.get_displayed_unit() end

---Get the currently-hovered hex
---@return integer x, integer y
function wesnoth.interface.get_hovered_hex() end

---Get the currently-selected hex
---@return integer x, integer y
function wesnoth.interface.get_selected_hex() end

---Get the viewwing side
---@return integer side
---@return boolean has_full_vision
function wesnoth.interface.get_viewing_side() end

---Lock the interface so the player can't scroll the map
---@param lock boolean
function wesnoth.interface.lock(lock) end

---Check if the interface is locked to prevent map scrolling
---@return boolean
function wesnoth.interface.is_locked() end

---Scroll the map to bring a specific hex into view
---@param loc location
---@param only_if_visible? boolean
---@param instant? boolean
---@param only_if_needed? boolean
---@overload fun(x:integer, y:integer, only_if_visible?:boolean, instant?:boolean, only_if_needed?:boolean)
function wesnoth.interface.scroll_to_hex(loc, only_if_visible, instant, only_if_needed) end

---Scroll the map in the given direction
---@param dx integer
---@param dy integer
function wesnoth.interface.scroll(dx, dy) end

---Zoom the map in or out
---@param factor number
---@param relative boolean
---@return number #The resulting absolute zoom level
function wesnoth.interface.zoom(factor, relative) end

---Set the skip messages flag
---@param skip boolean
function wesnoth.interface.skip_messages(skip) end

---Check if messages are being skipped
---@return boolean
function wesnoth.interface.is_skipping_messages() end

---Add a message to the onscreen chat
---@param speaker? string
---@param message string
function wesnoth.interface.add_chat_message(speaker, message) end

---Clear all messages in the onscreen chat
function wesnoth.interface.clear_chat_messages() end

---Place an overlay image on a hex
---@param location location
---@param cfg WML
---@overload fun(x:integer, y:integer, cfg?:WML)
function wesnoth.interface.add_hex_overlay(location, cfg) end

---Remove all overlay images from a hex, or just a specific one
---@param location location
---@param filename? string
---@overload fun(x:integer, y:integer, filename?:string)
function wesnoth.interface.remove_hex_overlay(location, filename) end

---End the current turn and optionally specify which side gets to move next
---@param next_side? integer
function wesnoth.interface.end_turn(next_side) end

---Set whether the player is allowed end their turn and the reason given if they try
---@param reason boolean|string
function wesnoth.interface.allow_end_turn(reason) end

---Set the current screen tint
---@param red integer
---@param green integer
---@param blue integer
function wesnoth.interface.color_adjust(red, green, blue) end

---@type table<string, fun()>
wesnoth.game_display = {}
