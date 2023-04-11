// Copyright 2021 Charles Tytler

#include <format>

#include "TitleMonitor.h"

#include "Utilities/StringUtilities.h"

TitleMonitor::TitleMonitor(const json &settings) { update_settings(settings); }

void TitleMonitor::update_settings(const json &settings)
{
    const std::string dcs_id_string_monitor_raw = EPLJSONUtils::GetStringByName(settings, "dcs_id_string_monitor");
    // Set boolean from checkbox using default false value if it doesn't exist in "settings".
    const std::string string_monitor_vertical_spacing_raw =
        EPLJSONUtils::GetStringByName(settings, "string_monitor_vertical_spacing");
    string_monitor_passthrough_ = EPLJSONUtils::GetBoolByName(settings, "string_monitor_passthrough_check", true);
	string_monitor_format_ = EPLJSONUtils::GetBoolByName(settings, "string_monitor_formatting_check", false);
    string_monitor_format_raw_ = EPLJSONUtils::GetStringByName(settings, "string_monitor_mapping");
    std::stringstream string_monitor_mapping_raw;
    string_monitor_mapping_raw << EPLJSONUtils::GetStringByName(settings, "string_monitor_mapping");

    string_monitor_is_set_ = !dcs_id_string_monitor_raw.empty();

    if (string_monitor_is_set_) {
        if (is_integer(dcs_id_string_monitor_raw)) {
            dcs_id_string_monitor_ = SimulatorAddress(std::stoi(dcs_id_string_monitor_raw));

        } else {
            if (dcs_id_string_monitor_raw == "INTEGER") {
                dcs_id_string_monitor_ = SimulatorAddress(settings["string_monitor_address"],
                                                          settings["string_monitor_mask"],
                                                          settings["string_monitor_shift"]);
            } else if (dcs_id_string_monitor_raw == "STRING") {
                dcs_id_string_monitor_ =
                    SimulatorAddress(settings["string_monitor_address"], settings["string_monitor_max_length"]);
            }
        }

        if (is_integer(string_monitor_vertical_spacing_raw)) {
            string_monitor_vertical_spacing_ = std::stoi(string_monitor_vertical_spacing_raw);
        }
        if (!string_monitor_passthrough_ && !string_monitor_format_) {
            string_monitor_mapping_.clear();
            std::optional<std::pair<std::string, std::string>> maybe_token_pair;
            while (maybe_token_pair = pop_key_and_value(string_monitor_mapping_raw, ',', '=')) {
                string_monitor_mapping_[maybe_token_pair.value().first] = maybe_token_pair.value().second;
            }
        }
    }
}

std::string TitleMonitor::determineTitle(SimulatorInterface *simulator_interface)
{
    std::string updated_title = "";

    if (string_monitor_is_set_) {
        const std::optional<std::string> maybe_current_game_value =
            simulator_interface->get_string_at_addr(dcs_id_string_monitor_);
        if (maybe_current_game_value.has_value()) {
            updated_title = convertGameStateToTitle(maybe_current_game_value.value());
        }
    }

    return updated_title;
}

std::string TitleMonitor::convertGameStateToTitle(const std::string &current_game_value)
{
    std::string title;
    if (string_monitor_passthrough_) {
        title = current_game_value;
    } else if (string_monitor_format_) {
        title = std::vformat(string_monitor_format_raw_, std::make_format_args(current_game_value));
	} else  {
        title = string_monitor_mapping_[current_game_value];
    }
    // Apply vertical spacing.
    if (string_monitor_vertical_spacing_ < 0) {
        for (int i = 0; i > string_monitor_vertical_spacing_; --i) {
            title = "\n" + title;
        }
    } else {
        for (int i = 0; i < string_monitor_vertical_spacing_; ++i) {
            title = title + "\n";
        }
    }
    return title;
}