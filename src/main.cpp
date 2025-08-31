#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/Geode.hpp>
#include <matjson.hpp>
using namespace geode::prelude;

struct ToggleSaveData {
	std::string key;
	bool toggled;
    int saved_time;
	ToggleSaveData(std::string k, bool t, int s)
        : key(std::move(k)), toggled(t), saved_time(s) {}
};

template<>
struct matjson::Serialize<std::vector<ToggleSaveData>> {
    static Result<std::vector<ToggleSaveData>> fromJson(matjson::Value const& value) {
        auto vec = std::vector<ToggleSaveData> {};
        for (auto const& item : value) {
            vec.push_back(ToggleSaveData(item["key"].asString().unwrap(), item["toggled"].asBool().unwrap(), item["original_time"].asInt().unwrap()));
        }
        return Ok(vec);
    }

    static matjson::Value toJson(std::vector<ToggleSaveData> const& vec) {
        auto arr = matjson::Value{}.array();
        for (auto const& item : vec) {
            arr.push(matjson::makeObject ({
				{"key", item.key},
                { "toggled", item.toggled },
                { "original_time", item.saved_time },
            }));
        }
        return arr;
    }

};



std::string getLevelKey(GJGameLevel* level){
	std::string levelKey;
	if(level->m_levelID == 0){
		levelKey = fmt::format("{}_{}", std::string(level->m_levelName), level->m_levelRev);
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

	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects){
		if(!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
		std::string levelKey = getLevelKey(level);
		auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
		auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level); });
		bool saveDataIsNull = saveData == vec.end();
		int current_timestamp = level->m_timestamp != 0 || saveDataIsNull ? level->m_timestamp : saveData->saved_time; 
		m_fields->current_timestamp = current_timestamp;
		bool forceEnabled = Mod::get()->getSettingValue<bool>("force-enable");
		bool saveDataToggle = saveDataIsNull ? false : saveData->toggled;

		if (!saveDataIsNull && saveData->saved_time != current_timestamp) {
			saveData->saved_time = current_timestamp;
		}
		else {
			vec.push_back(ToggleSaveData(
				levelKey,
				saveDataToggle,
				current_timestamp
			));
		}

		if(!level->isPlatformer() && (saveDataToggle || forceEnabled)){
			level->m_timestamp = 0;
		}
		return true;
	}
	void onQuit(){
		m_level->m_timestamp = m_fields->current_timestamp;
		PlayLayer::onQuit();
	}
};


class $modify (EditLevelLayer){
	bool init(GJGameLevel* level){
		if(!EditLevelLayer::init(level)) return false;
		std::string levelKey = getLevelKey(level);
		
		auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
		auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level);});
		auto toggle_holder = CCMenu::create();
		toggle_holder->setContentWidth(50);
		toggle_holder->setLayout(RowLayout::create());

        auto checkbox = CCMenuItemExt::createTogglerWithStandardSprites(
          0.75,
            [this, levelKey](auto){
				auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
				auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level); });
					if (saveData != vec.end()) {
						saveData->toggled = !saveData->toggled;
					}
					else {
						vec.push_back(ToggleSaveData(
							levelKey,
							true,
							m_level->m_timestamp
						));
					}
				Mod::get()->setSavedValue("toggle-save-data", vec);
			 });
		checkbox->toggle(saveData == vec.end() ? false : saveData->toggled );
		

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
		std::string levelKey = getLevelKey(level);

		auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
		auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level);});
		auto toggle_holder = CCMenu::create();
		toggle_holder->setContentWidth(50);
		toggle_holder->setLayout(RowLayout::create());

        auto checkbox = CCMenuItemExt::createTogglerWithStandardSprites(
          0.75,
            [this, levelKey](auto){
				auto vec = Mod::get()->getSavedValue<std::vector<ToggleSaveData>>("toggle-save-data", {});
				auto saveData = std::find_if(vec.begin(), vec.end(), [this](ToggleSaveData const& item) { return item.key == getLevelKey(m_level); });
					if (saveData != vec.end()) {
						saveData->toggled = !saveData->toggled;
					}
					else {
						vec.push_back(ToggleSaveData(
							levelKey,
							true,
							m_level->m_timestamp
						));
					}
				Mod::get()->setSavedValue("toggle-save-data", vec);
			 });
		checkbox->toggle(saveData == vec.end() ? false : saveData->toggled );
		

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