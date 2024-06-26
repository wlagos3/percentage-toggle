#include "Geode/cocos/base_nodes/Layout.hpp"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/modify/Modify.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/Geode.hpp>
using namespace geode::prelude;

struct ToggleSaveData {
	std::string key;
	bool toggled;
    int saved_time;
};

template<>
struct matjson::Serialize<std::vector<ToggleSaveData>> {
    static std::vector<ToggleSaveData> from_json(matjson::Value const& value) {
        auto vec = std::vector<ToggleSaveData> {};
        for (auto const& item : value.as_array()) {
            vec.push_back({
				.key = item["key"].as_string(),
                .toggled = item["toggled"].as_bool(),
                .saved_time = item["original_time"].as_int(),
            });
        }
        return vec;
    }

    static matjson::Value to_json(std::vector<ToggleSaveData> const& vec) {
        auto arr = matjson::Array {};
        for (auto const& item : vec) {
            arr.push_back(matjson::Object {
				{"key", item.key},
                { "toggled", item.toggled },
                { "original_time", item.saved_time },
            });
        }
        return arr;
    }

    static bool is_json(matjson::Value const& value) {
        return value.is_array();
    }
};



std::string getLevelKey(GJGameLevel* level){
	std::string levelKey;
	if(level->m_levelID == 0){
		std::ostringstream s;
		s << level->m_levelName << "_" << level->m_levelRev;
		levelKey = s.str();
	}
	else{
		levelKey = std::to_string(level->m_levelID);
	}
	return levelKey;
}

void addToggleToMenu(std::string menuName){
	
}

class $modify (PlayLayer){
	struct Fields {
		int current_timestamp;
	};

	//rename these values

	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects){
		if(!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
		gd::string levelKey = getLevelKey(level);
		auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
		auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level); });
		int current_timestamp = level->m_timestamp != 0 ? level->m_timestamp : saveData->saved_time; 
		m_fields->current_timestamp = current_timestamp;
		bool forceEnabled = Mod::get()->getSettingValue<bool>("force-enable");
		bool saveDataToggle = saveData.base() == nullptr ? false : saveData->toggled;
		std::cout<< current_timestamp << std::endl;
		if(saveData.base() != nullptr){
			std::cout << saveData->saved_time << std::endl;
		}


		//optimize by only doing this if it appears to be wrong.
		if (saveData != vec.end() && saveData->saved_time != current_timestamp) {
			saveData->saved_time = current_timestamp;
		}
		else {
			vec.push_back({
				.key = levelKey,
				.toggled = saveDataToggle,
				.saved_time = current_timestamp
			});
		}

		if(!level->isPlatformer() && (saveDataToggle || forceEnabled)){
			level->m_timestamp = 0;
		}
		return true;
	}
	void onQuit(){
		m_level->m_timestamp = m_fields->current_timestamp;
		std::cout << "setting to :" << m_fields->current_timestamp << std::endl;
		PlayLayer::onQuit();
	}
};


class $modify (EditLevelLayer){
	bool init(GJGameLevel* level){
		if(!EditLevelLayer::init(level)) return false;
		gd::string levelKey = getLevelKey(level);
		
		auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
		auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level);});
		auto toggle_holder = CCMenu::create();
		toggle_holder->setContentWidth(50);
		toggle_holder->setLayout(RowLayout::create());

        auto checkbox = CCMenuItemExt::createTogglerWithStandardSprites(
          0.75,
            [this, levelKey](auto){
				std::cout << levelKey << std::endl;
				auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
				auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level); });
					if (saveData != vec.end()) {
						std::cout << "hi" << std::endl;
						saveData->toggled = !saveData->toggled;
					}
					else {
						vec.push_back({
							.key = levelKey,
							.toggled = true,
							.saved_time = m_level->m_timestamp
						});
					}
				Mod::get()->setSavedValue("toggle-save-data", vec);
			 });
		checkbox->toggle(saveData.base() == nullptr ? false : saveData->toggled );
		

		auto label = CCLabelBMFont::create("2.1", "bigFont.fnt");
		toggle_holder->addChild(checkbox);
		toggle_holder->setScale(0.25);

		toggle_holder->addChild(label);
		toggle_holder->updateLayout();
	    auto folderMenu = getChildByID("folder-menu");

		folderMenu->addChildAtPosition(toggle_holder, Anchor::Center, CCPoint(50, 0));
        folderMenu->updateLayout();
		return true;
	}
};

class $modify (LevelInfoLayer){
	bool init(GJGameLevel* level, bool p1){
		if(!LevelInfoLayer::init(level, p1)) return false;
		gd::string levelKey = getLevelKey(level);

		auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
		auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level);});
		auto toggle_holder = CCMenu::create();
		toggle_holder->setContentWidth(50);
		toggle_holder->setLayout(RowLayout::create());

        auto checkbox = CCMenuItemExt::createTogglerWithStandardSprites(
          0.75,
            [this, levelKey](auto){
				std::cout << levelKey << std::endl;
				auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
				auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level); });
					if (saveData != vec.end()) {
						std::cout << "hi" << std::endl;
						saveData->toggled = !saveData->toggled;
					}
					else {
						vec.push_back({
							.key = levelKey,
							.toggled = true,
							.saved_time = m_level->m_timestamp
						});
					}
				Mod::get()->setSavedValue("toggle-save-data", vec);
			 });
		checkbox->toggle(saveData.base() == nullptr ? false : saveData->toggled );
		

		auto label = CCLabelBMFont::create("2.1", "bigFont.fnt");
		toggle_holder->addChild(checkbox);
		toggle_holder->setScale(0.25);

		toggle_holder->addChild(label);
		toggle_holder->updateLayout();
	    auto leftSideMenu = getChildByID("left-side-menu");

		leftSideMenu->addChildAtPosition(toggle_holder, Anchor::Center, CCPoint(50, 0));
        leftSideMenu->updateLayout();
    
		
		return true;
	}
};




