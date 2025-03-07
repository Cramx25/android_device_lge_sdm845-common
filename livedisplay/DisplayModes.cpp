/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DisplayModes.h"
#include <android-base/logging.h>
#include <fstream>

namespace vendor {
namespace lineage {
namespace livedisplay {
namespace V2_0 {
namespace implementation {

static constexpr const char* kModePath = "/sys/devices/virtual/panel/img_tune/screen_mode";
static constexpr const char* kDefaultPath = "/data/vendor/display/default_display_mode";

const std::map<int32_t, DisplayModes::ModeInfo> DisplayModes::kModeMap = {
    {0, {"Cinema", "1"}},
    {1, {"Sports", "4"}},
    {2, {"Game", "5"}},
    {3, {"Photos", "2"}},
    {4, {"Web", "3"}},
//    {5, {"Expert", "10"}},
};

DisplayModes::DisplayModes() {
    std::ifstream defaultFile(kDefaultPath);
    std::string value;

    defaultFile >> value;
    LOG(DEBUG) << "Default file read result " << value << " fail " << defaultFile.fail();
    if (defaultFile.fail()) {
        return;
    }

    for (const auto& entry : kModeMap) {
        if (value == entry.second.value) {
            mDefaultModeId = entry.first;
            break;
        }
    }

    setDisplayMode(mDefaultModeId, false);
}

// Methods from ::vendor::lineage::livedisplay::V2_0::IDisplayModes follow.
Return<void> DisplayModes::getDisplayModes(getDisplayModes_cb resultCb) {
    std::vector<DisplayMode> modes;
    for (const auto& entry : kModeMap) {
        modes.push_back({entry.first, entry.second.name});
    }
    resultCb(modes);
    return Void();
}

Return<void> DisplayModes::getCurrentDisplayMode(getCurrentDisplayMode_cb resultCb) {
    int32_t currentModeId = mDefaultModeId;
    std::ifstream modeFile(kModePath);
    std::string value;

    modeFile >> value;
    if (!modeFile.fail()) {
        for (const auto& entry : kModeMap) {
            if (value == entry.second.value) {
                currentModeId = entry.first;
                break;
            }
        }
    }
    resultCb({currentModeId, kModeMap.at(currentModeId).name});
    return Void();
}

Return<void> DisplayModes::getDefaultDisplayMode(getDefaultDisplayMode_cb resultCb) {
    resultCb({mDefaultModeId, kModeMap.at(mDefaultModeId).name});
    return Void();
}

Return<bool> DisplayModes::setDisplayMode(int32_t modeID, bool makeDefault) {
    const auto iter = kModeMap.find(modeID);
    if (iter == kModeMap.end()) {
        return false;
    }
    std::ofstream modeFile(kModePath);
    modeFile << iter->second.value;
    if (modeFile.fail()) {
        return false;
    }

    if (makeDefault) {
        std::ofstream defaultFile(kDefaultPath);
        defaultFile << iter->second.value;
        if (defaultFile.fail()) {
            return false;
        }
        mDefaultModeId = iter->first;
    }
    return true;
}

}  // namespace sdm
}  // namespace V2_0
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
