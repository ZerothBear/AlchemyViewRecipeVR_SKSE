#pragma once

#include "RE/Skyrim.h"
#include "SKSE/API.h"
#include "SKSE/SKSE.h"

#include "Plugin.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

using namespace std::literals;
