//
// Created by PinkySmile on 23/02/2021.
//

#include <fstream>
#include <iostream>
#include <direct.h>
#include "State.hpp"
#include "Gui.hpp"
#include "Logic.hpp"
#include "Inputs.hpp"
#include "Hitboxes.hpp"

#define mkdir _mkdir
#define PAYLOAD_ADDRESS_GET_INPUTS 0x40A45E
#define PAYLOAD_NEXT_INSTR_GET_INPUTS (PAYLOAD_ADDRESS_GET_INPUTS + 4)

namespace Practice
{
	std::map<std::string, std::vector<unsigned short>> characterSpellCards{
		{"alice", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
		{"aya", {200, 201, 202, 203, 205, 206, 207, 208, 211, 212}},
		{"chirno", {200, 201, 202, 203, 204, 205, 206, 207, 208, 210, 213}},
		{"iku", {200, 201, 202, 203, 206, 207, 208, 209, 210, 211}},
		{"komachi", {200, 201, 202, 203, 204, 205, 206, 207, 211}},
		{"marisa", {200, 202, 203, 204, 205, 206, 207, 208, 209, 211, 212, 214, 215, 219}},
		{"meirin", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 211}},
		{"patchouli", {200, 201, 202, 203, 204, 205, 206, 207, 210, 211, 212, 213}},
		{"reimu", {200, 201, 204, 206, 207, 208, 209, 210, 214, 219}},
		{"remilia", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
		{"sakuya", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212}},
		{"sanae", {200, 201, 202, 203, 204, 205, 206, 207, 210}},
		{"suika", {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
		{"suwako", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212}},
		{"tenshi", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
		{"udonge", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
		{"utsuho", {200, 201, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214}},
		{"youmu", {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
		{"yukari", {200, 201, 202, 203, 204, 205, 206, 207, 208, 215}},
		{"yuyuko", {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 219}}
	};
	void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<short> &);
	static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();
	static const unsigned char patchCode[] = {
		0x31, 0xED,                  //xor ebp, ebp
		0xE9, 0x91, 0x01, 0x00, 0x00 //jmp pc+00000191
	};
	static unsigned char originalCode[sizeof(patchCode)];

	void __fastcall KeymapManagerSetInputs(SokuLib::KeymapManager *This)
	{
		(This->*s_origKeymapManager_SetInputs)();
		Practice::handleInput(*This);
	}

	void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::deckInfo &deck, int param4, SokuLib::mVC9Dequeue<short> &newDeck)
	{
		bool isPatchy = strcmp(charName, "patchouli") == 0;
		int number = (4 + isPatchy) * 3;
		unsigned short originalDeck[20];

		printf("Save old deck of %i cards\n", min(newDeck.size, 20));
		for (int i = 0; i < min(newDeck.size, 20); i++) {
			originalDeck[i] = newDeck[i];
		}

		printf("Loading deck for %s (%i skill cards)\n", charName, number);
		for (int i = 0; i < number; i++)
			newDeck[i] = 100 + i;
		newDeck.size = number;
		puts("Fake deck generated");
		s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);

		auto &entry = characterSpellCards[charName];

		printf("Loading deck for %s (%i spell cards)\n", charName, entry.size());
		for (int i = 0; i < entry.size(); i++)
			newDeck[i] = entry[i];
		puts("Fake deck generated");
		newDeck.size = entry.size();
		s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);

		puts("Placing old deck back");
		for (int i = 0; i < 20; i++)
			newDeck[i] = originalDeck[i];
		newDeck.size = 20;
		puts("Work done !");
	}

	Settings settings;
	sf::RenderWindow *sfmlWindow;
	char profilePath[1024 + MAX_PATH];
	char profileParent[1024 + MAX_PATH];
	SokuLib::KeyInput lastPlayerInputs;

	void placeHooks()
	{
		DWORD old;
		int newOffset;

		if (settings.activated)
			return;
		puts("Placing hooks");
		settings.activated = true;
		//Bypass the basic practice features by skipping most of the function.
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++) {
			originalCode[i] = ((unsigned char *)0x42A331)[i];
			((unsigned char *)0x42A331)[i] = patchCode[i];
		}

		newOffset = (int)KeymapManagerSetInputs - PAYLOAD_NEXT_INSTR_GET_INPUTS;
		s_origKeymapManager_SetInputs = SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(*(int *)PAYLOAD_ADDRESS_GET_INPUTS + PAYLOAD_NEXT_INSTR_GET_INPUTS);
		*(int *)PAYLOAD_ADDRESS_GET_INPUTS = newOffset;

		((unsigned *)0x00898680)[1] = 0x008986A8;
	}

	void activate()
	{
		if (sfmlWindow)
			return;

		settings.load();
		puts("Opening window");
		sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
		puts("Window opened");
		Practice::initInputDisplay(profileParent);
		Practice::initBoxDisplay(profileParent);
		Practice::init(profileParent);
		Practice::gui.setTarget(*sfmlWindow);
		try {
			Practice::loadAllGuiElements(profileParent);
		} catch (std::exception &e) {
			puts(e.what());
			throw;
		}
		placeHooks();
	}

	void removeHooks()
	{
		DWORD old;

		if (!settings.activated)
			return;
		puts("Removing hooks");
		settings.activated = false;
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), PAGE_EXECUTE_WRITECOPY, &old);
		for (unsigned i = 0; i < sizeof(patchCode); i++)
			((unsigned char *)0x42A331)[i] = originalCode[i];
		VirtualProtect((PVOID)0x42A331, sizeof(patchCode), old, &old);

		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, PAGE_EXECUTE_WRITECOPY, &old);
		*(int *)PAYLOAD_ADDRESS_GET_INPUTS = SokuLib::union_cast<int>(s_origKeymapManager_SetInputs) - PAYLOAD_NEXT_INSTR_GET_INPUTS;
		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, old, &old);
	}

	void deactivate()
	{
		if (!sfmlWindow)
			return;

		settings.save();
		delete sfmlWindow;
		sfmlWindow = nullptr;
		removeHooks();
	}

	Settings::Settings(bool activated) :
		activated(activated)
	{
	}

	void Settings::load()
	{
		this->leftState.load(SokuLib::leftChar, true);
		this->rightState.load(SokuLib::rightChar, false);

		std::cout << "Loading settings from APMSettings/APMLastSession.dat" << std::endl;

		std::ifstream stream{"APMSettings/APMLastSession.dat"};

		if (!stream) {
			std::cerr << "Error: Couldn't load settings from \"APMSettings/APMLastSession.dat\": " << strerror(errno) << std::endl;
			return;
		}
		stream.read(reinterpret_cast<char *>(&this->activated + 1), sizeof(*this) - sizeof(this->activated) - sizeof(CharacterState) * 2);
	}

	void Settings::save() const
	{
		std::cout << "Saving settings to APMSettings/APMLastSession.dat" << std::endl;
		mkdir("APMSettings");

		std::ofstream stream{"APMSettings/APMLastSession.dat"};

		if (!stream) {
			std::cerr << "Error: Couldn't save settings to \"APMSettings/APMLastSession.dat\": " << strerror(errno) << std::endl;
			MessageBoxA(SokuLib::window, ("Error: Couldn't save file to \"APMSettings/APMLastSession.dat\": " + std::string(strerror(errno))).c_str(), "Cannot save settings.", MB_ICONERROR);
			return;
		}
		stream.write(reinterpret_cast<const char *>(&this->activated + 1), sizeof(*this) - sizeof(this->activated) - sizeof(CharacterState) * 2);
		this->leftState.save(true);
		this->rightState.save(false);
	}

	Settings::~Settings()
	{
		if (this->activated)
			this->save();
	}

	void CharacterState::save(bool isLeft) const
	{
		std::string path = "APMSettings/APMLastSession" + std::string(isLeft ? ("Player") : ("Dummy")) + SokuLib::charactersName[this->_chr] + ".dat";

		for (size_t pos = path.find(' '); pos != std::string::npos; pos = path.find(' '))
			path.erase(path.begin() + pos);
		std::cout << "Saving character state to " << path << std::endl;

		std::ofstream stream{path};

		if (!stream) {
			std::cerr << "Error: Couldn't save file from \"" << path << "\": " << strerror(errno) << std::endl;
			MessageBoxA(SokuLib::window, ("Error: Couldn't save file to \"" + path + "\": " + strerror(errno)).c_str(), "Cannot save settings.", MB_ICONERROR);
			return;
		}
		stream.write(reinterpret_cast<const char *>(this), sizeof(*this));
	}

	void CharacterState::load(SokuLib::Character chr, bool isLeft)
	{
		this->_chr = chr;
		std::string path = "APMSettings/APMLastSession" + std::string(isLeft ? ("Player") : ("Dummy")) + SokuLib::charactersName[chr] + ".dat";

		for (size_t pos = path.find(' '); pos != std::string::npos; pos = path.find(' '))
			path.erase(path.begin() + pos);
		std::cout << "Loading character state from " << path << std::endl;

		std::ifstream stream{path};

		if (!stream) {
			SokuLib::Skill buffer[15] = {
				{0x00, false},
				{0x00, false},
				{0x00, false},
				{0x00, false},
				{
				 static_cast<unsigned char>((chr != SokuLib::CHARACTER_PATCHOULI) * 0x7F),
				       chr != SokuLib::CHARACTER_PATCHOULI
				},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
				{0x7F, true},
			};

			std::cerr << "Error: Couldn't load file to \"" << path << "\": " << strerror(errno) << std::endl;
			memcpy(&this->skillMap, &buffer, sizeof(buffer));
			return;
		}
		stream.read(reinterpret_cast<char *>(this), sizeof(*this));
	}
}