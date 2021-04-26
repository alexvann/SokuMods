#include <Windows.h>
#include <SokuLib.hpp>
#include <map>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <dinput.h>
#include "DrawUtils.hpp"

#define FONT_HEIGHT 16
#define TEXTURE_SIZE 0x200

#ifdef _DEBUG
#define _DEBUG_BUILD
#endif

//#define _DEBUG

#ifndef _DEBUG
#define puts(x)
#define printf(...)
#endif

static SokuLib::Trampoline *profileTrampoline;
static SokuLib::Trampoline *deckSavedTrampoline;
static int (SokuLib::Title::*s_originalTitleOnProcess)();
static int (SokuLib::Select::*s_originalSelectOnProcess)();
static int (SokuLib::SelectClient::*s_originalSelectCLOnProcess)();
static int (SokuLib::SelectServer::*s_originalSelectSVOnProcess)();
static int (SokuLib::Select::*s_originalSelectOnRender)();
static int (SokuLib::SelectClient::*s_originalSelectCLOnRender)();
static int (SokuLib::SelectServer::*s_originalSelectSVOnRender)();
static int (SokuLib::ObjectSelect::*s_originalInputMgrGet)();
static int (SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_OnProcess)();
static int (SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_OnRender)();
static void (*s_originalDrawGradiantBar)(float param1, float param2, float param3);
void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<unsigned short> &);

std::array<std::map<unsigned short, DrawUtils::Sprite>, SokuLib::CHARACTER_RANDOM + 1> cardsTextures;
std::map<SokuLib::Character, std::vector<unsigned short>> characterSpellCards{
	{SokuLib::CHARACTER_ALICE, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_AYA, {200, 201, 202, 203, 205, 206, 207, 208, 211, 212}},
	{SokuLib::CHARACTER_CIRNO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 210, 213}},
	{SokuLib::CHARACTER_IKU, {200, 201, 202, 203, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_KOMACHI, {200, 201, 202, 203, 204, 205, 206, 207, 211}},
	{SokuLib::CHARACTER_MARISA, {200, 202, 203, 204, 205, 206, 207, 208, 209, 211, 212, 214, 215, 219}},
	{SokuLib::CHARACTER_MEILING, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 211}},
	{SokuLib::CHARACTER_PATCHOULI, {200, 201, 202, 203, 204, 205, 206, 207, 210, 211, 212, 213}},
	{SokuLib::CHARACTER_REIMU, {200, 201, 204, 206, 207, 208, 209, 210, 214, 219}},
	{SokuLib::CHARACTER_REMILIA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
	{SokuLib::CHARACTER_SAKUYA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212}},
	{SokuLib::CHARACTER_SANAE, {200, 201, 202, 203, 204, 205, 206, 207, 210}},
	{SokuLib::CHARACTER_SUIKA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
	{SokuLib::CHARACTER_SUWAKO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212}},
	{SokuLib::CHARACTER_TENSHI, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
	{SokuLib::CHARACTER_REISEN, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_UTSUHO, {200, 201, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214}},
	{SokuLib::CHARACTER_YOUMU, {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
	{SokuLib::CHARACTER_YUKARI, {200, 201, 202, 203, 204, 205, 206, 207, 208, 215}},
	{SokuLib::CHARACTER_YUYUKO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 219}}
};
std::array<std::array<unsigned short, 20>, 20> defaultDecks = {
	std::array<unsigned short, 20>{1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 208, 208, 208},
	std::array<unsigned short, 20>{ 0, 1, 1, 1, 1, 100, 100, 101, 101, 102, 103, 200, 200, 200, 202, 202, 208, 208, 208, 208 },
	std::array<unsigned short, 20>{ 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 202, 202, 202, 203, 203 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 104, 104, 200, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 0, 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 202, 202, 202, 206, 206, 206 },
	std::array<unsigned short, 20>{ 0, 0, 1, 1, 1, 1, 100, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 205, 205, 205 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 202, 202, 202, 205, 205, 205 },
	std::array<unsigned short, 20>{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 0, 0, 0, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 203, 203, 203, 203 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 202, 202, 202, 202 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 202, 202, 202, 202, 204, 204, 204, 204 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 203, 203, 203, 203, 213, 213, 213, 213 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 204, 204, 204, 204 }
};
std::array<const char *, SokuLib::CHARACTER_RANDOM> names{
	"reimu",
	"marisa",
	"sakuya",
	"alice",
	"patchouli",
	"youmu",
	"remilia",
	"yuyuko",
	"yukari",
	"suika",
	"udonge",
	"aya",
	"komachi",
	"iku",
	"tenshi",
	"sanae",
	"chirno",
	"meirin",
	"utsuho",
	"suwako",
};

struct Deck {
	std::string name;
	std::array<unsigned short, 20> cards;
};

static char profilePath[1024 + MAX_PATH];
static int createDeckGuideTexture;
static int selectDeckGuideTexture;
static int dialogBoxGuideTexture;
static int editBoxGuideTexture;
static char editingBoxObject[0x164];
static bool escPressed = false;
static bool copyBoxDisplayed = false;
static bool renameBoxDisplayed = false;
static bool deleteBoxDisplayed = false;
static bool generated = false;
static bool saveError = false;
static bool side = false;
static bool firstConstructLoad = true;
static bool firstLoad = true;
static bool loaded = false;
static bool created = false;
static char nameBuffer[64];
static unsigned char copyBoxSelectedItem = 0;
static unsigned char deleteBoxSelectedItem = 0;
static unsigned char renameBoxSelectedItem = 0;
static unsigned char errorCounter = 0;
static unsigned upSelectedDeck = 0;
static unsigned downSelectedDeck = 0;
static unsigned editSelectedDeck = 0;
static unsigned editSelectedProfile = 0;
static std::string lastLoadedProfile;
static std::string leftLoadedProfile;
static std::string rightLoadedProfile;
static SokuLib::SWRFont font;
static SokuLib::SWRFont defaultFont;
static std::array<std::array<std::vector<Deck>, 20>, 3> loadedDecks;
static std::vector<Deck> editedDecks;
static SokuLib::Character lastLeft;
static SokuLib::Character lastRight;
static bool pickedRandom = false;
static DrawUtils::Sprite arrowSprite;
static DrawUtils::Sprite baseSprite;
static DrawUtils::Sprite nameSprite;
static DrawUtils::Sprite noSprite;
static DrawUtils::Sprite noSelectedSprite;
static DrawUtils::Sprite yesSprite;
static DrawUtils::Sprite yesSelectedSprite;
static std::unique_ptr<std::array<unsigned short, 20>> fakeLeftDeck;
static std::unique_ptr<std::array<unsigned short, 20>> fakeRightDeck;

#define SENDTO_JUMP_ADDR 0x00857290

unsigned short getRandomCard(const std::vector<unsigned short> &list, const std::map<unsigned short, unsigned char> &other)
{
	unsigned short card;

	try {
		do
			card = list[rand() % list.size()];
		while (other.at(card) >= 4);
	} catch (std::out_of_range &) {}
	return card;
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, const std::array<unsigned short, 20> *base, std::unique_ptr<std::array<unsigned short, 20>> &buffer)
{
	if (!base) {
		buffer.reset();
		return;
	}

	unsigned last = 100 + 3 * (4 + (chr == SokuLib::CHARACTER_PATCHOULI));
	std::map<unsigned short, unsigned char> used;
	unsigned char c = 0;
	int index = 0;

	buffer = std::make_unique<std::array<unsigned short, 20>>();
	for (int i = 0; i < 20; i++)
		if ((*base)[i] == 21)
			c++;
		else {
			(*buffer)[index] = (*base)[i];
			if (used.find((*buffer)[index]) == used.end())
				used[(*buffer)[index]] = 1;
			else
				used[(*buffer)[index]]++;
			index++;
		}
	if (!c)
		return;

	std::vector<unsigned short> cards;

	for (int i = 0; i < 21; i++)
		cards.push_back(i);
	for (int i = 100; i < last; i++)
		cards.push_back(i);
	for (auto &card : characterSpellCards[chr])
		cards.push_back(card);

	while (c--) {
		(*buffer)[19 - c] = getRandomCard(cards, used);
		if (used.find((*buffer)[19 - c]) == used.end())
			used[(*buffer)[19 - c]] = 1;
		else
			used[(*buffer)[19 - c]]++;
	}
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, unsigned id, const std::vector<Deck> &bases, std::unique_ptr<std::array<unsigned short, 20>> &buffer)
{
	std::array<unsigned short, 20> randomDeck{21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21};

 	pickedRandom = false;
	if (lastChr == SokuLib::CHARACTER_RANDOM) {
		generateFakeDeck(chr, chr, rand() % (bases.size() + 1), bases, buffer);
		pickedRandom = true;
		return;
	}
	if (id == bases.size())
		return generateFakeDeck(chr, lastChr, &defaultDecks[chr], buffer);
	if (id == bases.size() + 1)
		return generateFakeDeck(chr, lastChr, nullptr, buffer);
	if (id == bases.size() + 2)
		return generateFakeDeck(chr, lastChr, &randomDeck, buffer);
	if (id == bases.size() + 3)
		return generateFakeDeck(chr, lastChr, rand() % (bases.size() + 1), bases, buffer);
	return generateFakeDeck(chr, lastChr, &bases[id].cards, buffer);
}

void generateFakeDecks()
{
	if (generated)
		return;
	generated = true;
	generateFakeDeck(SokuLib::leftChar,  lastLeft,  upSelectedDeck,  loadedDecks[0][SokuLib::leftChar], fakeLeftDeck);
	generateFakeDeck(SokuLib::rightChar, lastRight, downSelectedDeck, loadedDecks[SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER][SokuLib::rightChar], fakeRightDeck);
}

void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::deckInfo &deck, int param4, SokuLib::mVC9Dequeue<unsigned short> &newDeck)
{
	generateFakeDecks();
	side = !side;
	while (newDeck.size < 20) {
		newDeck.data[newDeck.size / 8 + 1] = SokuLib::New<unsigned short>(8);
		newDeck.size += 8;
	}
	newDeck.size = 20;
	if (side && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER) { //Left
		if (fakeLeftDeck)
			for (int i = 0; i < 20; i++)
				newDeck[i] = (*fakeLeftDeck)[i];
		else
			newDeck.size = 0;
	} else if (!side && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {//Right
		if (fakeRightDeck)
			for (int i = 0; i < 20; i++)
				newDeck[i] = (*fakeRightDeck)[i];
		else
			newDeck.size = 0;
	}
	s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
}
static bool pressedRandom = false;

int __stdcall mySendTo(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen)
{
	auto packet = reinterpret_cast<SokuLib::Packet *>(buf);

	if (SokuLib::sceneId != SokuLib::SCENE_SELECTCL && SokuLib::sceneId != SokuLib::SCENE_SELECTSV && SokuLib::sceneId != SokuLib::SCENE_SELECT)
		return sendto(s, buf, len, flags, to, tolen);
	if (packet->type == SokuLib::CLIENT_GAME || packet->type == SokuLib::HOST_GAME) {
		bool needDelete = false;

		if (packet->game.event.type == SokuLib::GAME_MATCH) {
			generateFakeDecks();

			auto &fake = (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT ? fakeLeftDeck : fakeRightDeck);
			auto &replace = (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT ? packet->game.event.match.host : packet->game.event.match.client());

			replace.deckId = 0;
			if (fake)
				memcpy(replace.cards, fake->data(), fake->size() * sizeof(*fake->data()));
			else //We just send an invalid deck over if we want no decks
				memset(replace.cards, 0, 40);
		}
		if (packet->game.event.type == SokuLib::GAME_INPUT && packet->game.event.input.sceneId == SokuLib::SCENEID_CHARACTER_SELECT) {
			auto &scene = SokuLib::currentScene->to<SokuLib::Select>();

			if (scene.leftSelectionStage  != 1 && SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT)
				return sendto(s, buf, len, flags, to, tolen);
			if (scene.rightSelectionStage != 1 && SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
				return sendto(s, buf, len, flags, to, tolen);

			char *buffer = new char[len];

			memcpy(buffer, buf, len);
			packet = reinterpret_cast<SokuLib::Packet *>(buffer);
			//Me changing deck ? I would never do that !

			for (int i = 0; i < packet->game.event.input.inputCount; i++) {
				packet->game.event.input.inputs[i].charSelect.left = false;
				packet->game.event.input.inputs[i].charSelect.right = pickedRandom;
				pickedRandom = false;
				packet->game.event.input.inputs[i].charSelect.A = false;
			}
			pressedRandom = packet->game.event.input.inputs[0].charSelect.A;

			int bytes = sendto(s, buffer, len, flags, to, tolen);

			delete[] buffer;
			return bytes;
		}
	}
	return sendto(s, buf, len, flags, to, tolen);
}

static void sanitizeDeck(SokuLib::Character chr, Deck &deck)
{
	unsigned last = 100 + 3 * (4 + (chr == SokuLib::CHARACTER_PATCHOULI));
	std::map<unsigned short, unsigned char> used;
	std::vector<unsigned short> cards;

	for (int i = 0; i <= 21; i++)
		cards.push_back(i);
	for (int i = 100; i < last; i++)
		cards.push_back(i);
	for (auto &card : characterSpellCards[chr])
		cards.push_back(card);
	for (int i = 0; i < 20; i++) {
		auto &card = deck.cards[i];

		if (card == 21)
			continue;
		if (std::find(cards.begin(), cards.end(), card) == cards.end() || used[card] >= 4) {
			card = 21;
			continue;
		}
		used[card]++;
	}
}

static bool loadProfileFile(std::ifstream &stream, std::array<std::vector<Deck>, 20> &arr, int index)
{
	if (stream.fail()) {
		printf("Failed to open file: %s\n", strerror(errno));
		for (int i = 0; i < 20; i++) {
			arr[i].clear();
			if (index == 2)
				arr[i].push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		}
		return false;
	}

	nlohmann::json json;

	stream >> json;
	if (json.size() != 20)
		throw std::invalid_argument("Not 20 characters");
	for (auto &arr : json) {
		for (auto &elem : arr) {
			elem.contains("name") && (elem["name"].get<std::string>(), true);
			if (!elem.contains("cards") || elem["cards"].get<std::vector<unsigned short>>().size() != 20)
				throw std::invalid_argument(elem.dump());
		}
	}

	for (int i = 0; i < 20; i++) {
		auto &array = json[i];

		arr[i].clear();
		for (int j = 0; j < array.size(); j++) {
			auto &elem = array[j];
			Deck deck;

			if (!elem.contains("name"))
				deck.name = "Deck #" + std::to_string(j + 1);
			else
				deck.name = elem["name"];
			memcpy(deck.cards.data(), elem["cards"].get<std::vector<unsigned short>>().data(), sizeof(*deck.cards.data()) * deck.cards.size());
			sanitizeDeck(static_cast<SokuLib::Character>(i), deck);
			std::sort(deck.cards.begin(), deck.cards.end());
			arr[i].push_back(deck);
		}
		if (index == 2)
			arr[i].push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	return true;
}

static void handleProfileChange(int This, SokuLib::String *str)
{
	char *arg = *str;
	std::string profileName{arg, strstr(arg, ".pf")};
	std::string profile = "profile/" + profileName + ".json";
	int index = 2;
	bool hasBackup;

	if (This == 0x898868) {
		//P1
		index = 0;
		leftLoadedProfile = profile;
	} else if (This == 0x899054) {
		//P2
		index = 1;
		rightLoadedProfile = profile;
	} //Else is deck construct
	printf("Loading %s\n", profile.c_str());

	bool result = false;
	auto &arr = loadedDecks[index];
	std::ifstream stream{profile + ".bck"};

	hasBackup = !stream.fail();
	stream.close();
	if (hasBackup)
		printf("%s has backup data !\n", profile.c_str());

	stream.open(profile, std::ifstream::in);
	try {
		result = loadProfileFile(stream, arr, index);
	} catch (std::exception &e) {
		auto answer = IDNO;

		if (!hasBackup)
			MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
		else
			answer = MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what() + "\n\nDo you want to load backup file ?").c_str(), "Loading error", MB_ICONERROR | MB_YESNO);
		if (answer != IDYES)
			throw;
	}
	stream.close();

	if (!result && hasBackup) {
		try {
			stream.open(profile + ".bck", std::ifstream::in);
			printf("Loading %s\n", (profile + ".bck").c_str());
			result = loadProfileFile(stream, arr, index);
			stream.close();
		} catch (std::exception &e) {
			MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ".bck: " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
			throw;
		}
		unlink(profile.c_str());
		rename((profile + ".bck").c_str(), profile.c_str());
		lastLoadedProfile = profile;
		return;
	}

	lastLoadedProfile = profile;
	if (hasBackup)
		unlink((profile + ".bck").c_str());
}

static int weirdRand(int key, int delay)
{
	static std::map<int, std::pair<int, int>> elems;
	auto it = elems.find(key);

	if (it == elems.end() || it->second.first == 0) {
		int v = rand();

		elems[key] = {delay, v};
		return v;
	}
	it->second.first--;
	return it->second.second;
}

static void initFont() {
	SokuLib::FontDescription desc;

	if (created)
		return;
	created = true;
	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.height = FONT_HEIGHT;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 2;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "Tahoma");
	font.create();
	font.setIndirect(desc);
	strcpy(desc.faceName, SokuLib::defaultFontName);
	defaultFont.create();
	defaultFont.setIndirect(desc);
}

static void onProfileChanged()
{
	int This;
	char *arg;

	__asm mov This, esi;
	__asm mov arg, esp;

#ifdef _DEBUG_BUILD
	arg = *(char **)(arg + 0x28);
#else
	arg = *(char **)(arg + 0x24);
#endif
	arg += 0x1C;
	handleProfileChange(This, reinterpret_cast<SokuLib::String *>(arg));
}

bool saveDeckFromGame(SokuLib::ProfileDeckEdit *This, std::array<unsigned short, 20> &deck)
{
	auto vec = This->editedDeck->vector();
	unsigned index = 0;

	for (auto pair : vec) {
		auto i = pair->second;

		while (i) {
			if (index == 20)
				return false;
			deck[index] = pair->first;
			i--;
			index++;
		}
	}
	while (index < 20) {
		deck[index] = 21;
		index++;
	}
	std::sort(deck.begin(), deck.end());
	return true;
}

static void onDeckSaved()
{
	auto menu = SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>();
	nlohmann::json result;
	std::string path;

	if (!saveDeckFromGame(menu, editedDecks[editSelectedDeck].cards))
		return;

	loadedDecks[2][menu->editedCharacter] = editedDecks;
	if (editSelectedProfile != 2) {
		loadedDecks[editSelectedProfile] = loadedDecks[2];
		path = editSelectedProfile == 0 ? leftLoadedProfile : rightLoadedProfile;
	} else
		path = lastLoadedProfile;
	printf("Saving to %s\n", path.c_str());
	for (int i = 0; i < 20; i++) {
		result[i] = nlohmann::json::array();
		for (int j = 0; j < loadedDecks[2][i].size() - 1; j++) {
			result[i][j]["name"] = loadedDecks[2][i][j].name;
			result[i][j]["cards"] = std::vector<unsigned short>{
				loadedDecks[2][i][j].cards.begin(),
				loadedDecks[2][i][j].cards.end()
			};
		}
	}
	if (std::ifstream(path + ".bck").fail())
		rename(path.c_str(), (path + ".bck").c_str());

	auto resultStr = result.dump(4);
	std::ofstream stream{path};

	if (stream.fail()) {
		if (menu->displayedNumberOfCards == 20) {
			menu->editedDeck->vector()[0]->second++;
			saveError = true;
		}
		MessageBoxA(SokuLib::window, ("Cannot open \"" + lastLoadedProfile + "\". Please make sure you have proper permissions and enough space on disk.").c_str(), "Saving error", MB_ICONERROR);
		return;
	}
	stream << resultStr;
	if (stream.fail()) {
		stream.close();
		if (menu->displayedNumberOfCards == 20) {
			menu->editedDeck->vector()[0]->second++;
			saveError = true;
		}
		MessageBoxA(SokuLib::window, ("Cannot write to \"" + lastLoadedProfile + "\". Please make sure you have proper enough space on disk.").c_str(), "Saving error", MB_ICONERROR);
		return;
	}
	stream.close();

	auto cards = menu->editedDeck->vector();

	for (auto card : cards)
		card->second = 0;
	for (int i = 0; i < 5; i++)
		menu->editedDeck->operator[](i) = 4;
	unlink((path + ".bck").c_str());
}

void renderDeck(SokuLib::Character chr, unsigned select, const std::vector<Deck> &decks, DrawUtils::Vector2<int> pos, const char *overridingName = nullptr)
{
	std::vector<unsigned short> deck;
	std::string name;
	DrawUtils::Vector2<int> base = pos;

	if (select == decks.size()) {
		name = "Default deck";
		deck.resize(20, 21);
		memcpy(deck.data(), defaultDecks[chr].data(), 40);
	} else if (select == decks.size() + 1) {
		name = "No deck";
		deck.resize(0);
	} else if (select == decks.size() + 2) {
		name = "Randomized deck";
		deck.resize(20, 21);
	} else if (select == decks.size() + 3)
		return renderDeck(chr, weirdRand((int)&decks, 3) % (decks.size() + 1), decks, pos, "Any deck");
	else {
		name = decks[select].name;
		deck = {decks[select].cards.begin(), decks[select].cards.end()};
	}

	if (overridingName)
		name = overridingName;

	if (!deck.empty()) {
		for (int i = 0; i < 10; i++) {
			DrawUtils::Sprite &sprite = (deck[i] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i]] : cardsTextures[chr][deck[i]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = DrawUtils::DxSokuColor::White;
			sprite.draw();
			pos.x += 10;
		}
		pos.x = base.x;
		pos.y += 16;
		for (int i = 0; i < 10; i++) {
			DrawUtils::Sprite &sprite = (deck[i + 10] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i + 10]] : cardsTextures[chr][deck[i + 10]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = DrawUtils::DxSokuColor::White;
			sprite.draw();
			pos.x += 10;
		}
	}
	pos.y = base.y + 32;

	DrawUtils::Sprite sprite;
	int text;
	int width = 0;

	if (!SokuLib::textureMgr.createTextTexture(&text, name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}

	pos.x = base.x + 50 - width / 2;
	sprite.setPosition(pos);
	sprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	sprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	sprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	sprite.tint = DrawUtils::DxSokuColor::White;
	sprite.draw();
}

static int counter = 0;

int renderingCommon(int ret)
{
	initFont();

	std::string name;
	auto &scene = SokuLib::currentScene->to<SokuLib::Select>();

	if (
		scene.leftSelectionStage == 1 &&
		SokuLib::leftChar != SokuLib::CHARACTER_RANDOM &&
		SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER
	)
		renderDeck(SokuLib::leftChar,  upSelectedDeck,   loadedDecks[0][SokuLib::leftChar],  {28, 98});
	if (
		scene.rightSelectionStage == 1 &&
		SokuLib::rightChar != SokuLib::CHARACTER_RANDOM &&
		SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT
	)
		renderDeck(SokuLib::rightChar, downSelectedDeck, loadedDecks[SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER][SokuLib::rightChar], {28, 384});
	return ret;
}

int __fastcall CSelectCL_OnRender(SokuLib::SelectClient *This) {
	return renderingCommon((This->*s_originalSelectCLOnRender)());
}

int __fastcall CSelectSV_OnRender(SokuLib::SelectServer *This) {
	return renderingCommon((This->*s_originalSelectSVOnRender)());
}

int __fastcall CSelect_OnRender(SokuLib::Select *This) {
	return renderingCommon((This->*s_originalSelectOnRender)());
}

static void loadTexture(DrawUtils::Texture &container, const char *path)
{
	int text;
	DrawUtils::Vector2<unsigned> size;

	if (!SokuLib::textureMgr.loadTexture(&text, path, &size.x, &size.y) || !text) {
		MessageBoxA(SokuLib::window, ("Cannot load " + std::string(path)).c_str(), "Fatal error", MB_ICONERROR);
		abort();
	}
	container.setHandle(text, size);
}

static void loadTexture(DrawUtils::Sprite &container, const char *path)
{
	loadTexture(container.texture, path);
}

static void loadCardAssets()
{
	char buffer[128];
	DrawUtils::Texture tmp;

	puts("Loading card assets");
	for (int i = 0; i <= 20; i++) {
		sprintf(buffer, "data/card/common/card%03i.bmp", i);
		loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][i], buffer);
	}
	loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][21], "data/battle/cardFaceDown.bmp");
	for (int j = 0; j < 20; j++) {
		for (int i = (j == SokuLib::CHARACTER_PATCHOULI ? 15 : 12); i; i--) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j], 99 + i);
			loadTexture(cardsTextures[j][99 + i], buffer);
		}
		for (auto &card : characterSpellCards.at(static_cast<SokuLib::Character>(j))) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j], card);
			loadTexture(cardsTextures[j][card], buffer);
		}
	}
	loadTexture(baseSprite,        "data/menu/21_Base.bmp");
	loadTexture(nameSprite,        "data/profile/20_Name.bmp");
	loadTexture(arrowSprite,       "data/profile/deck2/sayuu.bmp");
	loadTexture(noSprite,          "data/menu/23a_No.bmp");
	loadTexture(noSelectedSprite,  "data/menu/23b_No.bmp");
	loadTexture(yesSprite,         "data/menu/22a_Yes.bmp");
	loadTexture(yesSelectedSprite, "data/menu/22b_Yes.bmp");

	loadTexture(tmp, "data/guide/09.bmp");
	editBoxGuideTexture = tmp.releaseHandle();

	loadTexture(tmp, "data/guide/02.bmp");
	dialogBoxGuideTexture = tmp.releaseHandle();

	DrawUtils::Texture::loadFromFile(tmp, (profilePath + std::string("/guides/createDeck.png")).c_str());
	createDeckGuideTexture = tmp.releaseHandle();

	DrawUtils::Texture::loadFromFile(tmp, (profilePath + std::string("/guides/selectDeck.png")).c_str());
	selectDeckGuideTexture = tmp.releaseHandle();
}

static int selectProcessCommon(int v)
{
	static int left = 0;
	static int right = 0;
	auto &scene = SokuLib::currentScene->to<SokuLib::Select>();

	if (scene.leftSelectionStage == 3 && scene.rightSelectionStage == 3) {
		if (counter < 60)
			counter++;
	} else {
		counter = 0;
		generated = false;
	}

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER && scene.rightSelectionStage == 1) {
		auto &decks = loadedDecks[0][SokuLib::rightChar];
		auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

		if (keys->keymapManager->input.horizontalAxis < 0)
			left++;
		else
			left = 0;

		if (keys->keymapManager->input.horizontalAxis > 0)
			right++;
		else
			right = 0;

		if (left == 1 || (left >= 36 && left % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			if (downSelectedDeck == 0) {
				downSelectedDeck = decks.size() + 3;
			} else {
				downSelectedDeck--;
			}
		} else if (right == 1 || (right >= 36 && right % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			if (downSelectedDeck == decks.size() + 3) {
				downSelectedDeck = 0;
			} else {
				downSelectedDeck++;
			}
		}
	}

	if (scene.leftRandomDeck && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER) {
		scene.leftRandomDeck = false;
		upSelectedDeck = loadedDecks[0][SokuLib::leftChar].size() + 3;
	}
	if (scene.rightRandomDeck && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {
		scene.rightRandomDeck = false;
		downSelectedDeck = loadedDecks[SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER][SokuLib::rightChar].size() + 3;
	}
	if (scene.leftSelectionStage != 3 || scene.rightSelectionStage != 3 || counter < 30) {
		if (lastLeft != SokuLib::leftChar)
			upSelectedDeck = 0;
		lastLeft = SokuLib::leftChar;
		if (lastRight != SokuLib::rightChar)
			downSelectedDeck = 0;
		lastRight = SokuLib::rightChar;
	}
	if (v != SokuLib::SCENE_SELECT && v != SokuLib::SCENE_SELECTSV && v != SokuLib::SCENE_SELECTCL) {
		loaded = false;
		return v;
	}
	if (loaded)
		return v;
	if (firstLoad)
		loadCardAssets();
	loaded = true;
	firstLoad = false;
	return v;
}

int __fastcall CSelectCL_OnProcess(SokuLib::SelectClient *This) {
	return selectProcessCommon((This->*s_originalSelectCLOnProcess)());
}

int __fastcall CSelectSV_OnProcess(SokuLib::SelectServer *This) {
	return selectProcessCommon((This->*s_originalSelectSVOnProcess)());
}

int __fastcall CSelect_OnProcess(SokuLib::Select *This) {
	if (SokuLib::menuManager.isInMenu && *SokuLib::getMenuObj<int>() == 0x859820) {
		auto obj = SokuLib::getMenuObj<int>();
		auto selected = obj[0x1A];

		if (selected >= 2 && selected <= 3)
			editSelectedProfile = selected - 2;
		else
			editSelectedProfile = 2;
	}
	if (!SokuLib::menuManager.isInMenu || *SokuLib::getMenuObj<int>() != SokuLib::ADDR_VTBL_DECK_CONSTRUCTION_CHR_SELECT_MENU)
		firstConstructLoad = true;
	return selectProcessCommon((This->*s_originalSelectOnProcess)());
}

static void handleInput(const SokuLib::KeyInput &inputs, int index)
{
	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
		return;

	bool isRight = index == 1 || SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER;
	auto &selectedDeck = (isRight ? downSelectedDeck : upSelectedDeck);
	auto &decks = loadedDecks[index][isRight ? SokuLib::rightChar : SokuLib::leftChar];

	if (inputs.horizontalAxis < 0) {
		if (selectedDeck == 0)
			selectedDeck = decks.size() + 3;
		else
			selectedDeck--;
	} else {
		if (selectedDeck == decks.size() + 3)
			selectedDeck = 0;
		else
			selectedDeck++;
	}
}

int __fastcall myGetInput(SokuLib::ObjectSelect *This)
{
	int ret = (This->*s_originalInputMgrGet)();
	auto &scene = SokuLib::currentScene->to<SokuLib::Select>();
	auto &battle = SokuLib::getBattleMgr();

	if (
		(SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER && This == &scene.leftSelect) ||
		(SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT && This == &scene.rightSelect)
	)
		This->deck = 0;
	if (ret) {
		int a = abs(This->keys->horizontalAxis);

		if (a != 1 && (a < 36 || a % 6 != 0))
			return ret;
		SokuLib::playSEWaveBuffer(0x27);
		handleInput(*This->keys, This != &scene.leftSelect && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER);
		return 0;
	}
	return ret;
}

void drawGradiantBar(float x, float y, float maxY)
{
	if (y == 114)
		y = 90;
	s_originalDrawGradiantBar(x, y, maxY);
}

void loadDeckToGame(SokuLib::ProfileDeckEdit *This, const std::array<unsigned short, 20> &deck)
{
	int count = 0;

	for (auto pair : This->editedDeck->vector())
		pair->second = 0;
	for (int i = 0; i < 20; i++)
		if (deck[i] != 21) {
			This->editedDeck->operator[](deck[i])++;
			count++;
		}
	This->displayedNumberOfCards = count;
}

void __fastcall CProfileDeckEdit_SwitchCurrentDeck(SokuLib::ProfileDeckEdit *This, int, int DeckID)
{
	auto FUN_0044f930 = SokuLib::union_cast<void (SokuLib::ProfileDeckEdit::*)(char param_1)>(0x44F930);

	This->selectedDeck = 0;
	if (!saveDeckFromGame(This, editedDecks[editSelectedDeck].cards)) {
		errorCounter = 120;
		return;
	}
	if (DeckID == 1) {
		if (editSelectedDeck == editedDecks.size() - 1)
			editSelectedDeck = 0;
		else
			editSelectedDeck++;
	} else {
		if (editSelectedDeck == 0)
			editSelectedDeck = editedDecks.size() - 1;
		else
			editSelectedDeck--;
	}
	loadDeckToGame(This, editedDecks[editSelectedDeck].cards);
	(This->*FUN_0044f930)('\0');
}

void renameBoxRender()
{
	int text;
	int width;
	DrawUtils::Sprite textSprite;
	DrawUtils::RectangleShape rect;

	nameSprite.setPosition({160, 192});
	nameSprite.setSize(nameSprite.texture.getSize());
	nameSprite.rect = {
		0, 0,
		static_cast<int>(nameSprite.texture.getSize().x),
		static_cast<int>(nameSprite.texture.getSize().y)
	};
	nameSprite.tint = DrawUtils::DxSokuColor::White;
	nameSprite.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, nameBuffer, font, TEXTURE_SIZE, 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	auto render = (int(__thiscall*) (void*, float, float))0x42a050;

	render(editingBoxObject, 276, 217);
}

void openRenameBox(SokuLib::ProfileDeckEdit *This)
{
	auto setup_global = (int(__thiscall*) (void*, bool))0x40ea10;
	auto init_fun = (void(__thiscall*) (void*, int, int))0x429e70;

	SokuLib::playSEWaveBuffer(0x28);
	if (editSelectedDeck == editedDecks.size() - 1) {
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}

	//:magic_wand:
	init_fun(editingBoxObject, 0x89A4F8, 24);
	setup_global((void*)0x8A02F0, true);
	strncpy((char *)0x8A02F8, editedDecks[editSelectedDeck].name.c_str(), 24);
	renameBoxDisplayed = true;
}

void renameBoxUpdate(SokuLib::KeyManager *keys)
{
	auto update = (char (__thiscall*) (void*))0x429F00;

	renameBoxDisplayed = *(bool *)0x8A0CFF;
	if (!renameBoxDisplayed)
		SokuLib::playSEWaveBuffer(0x29);
	else if (update(editingBoxObject) == 1) {
		SokuLib::playSEWaveBuffer(0x28);
		editedDecks[editSelectedDeck].name = (char *)0x8A02F8;
		renameBoxDisplayed = false;
	}
}

void deleteBoxRender()
{
	int text;
	DrawUtils::Sprite textSprite;
	DrawUtils::Sprite &yes = deleteBoxSelectedItem ? yesSelectedSprite : yesSprite;
	DrawUtils::Sprite &no  = deleteBoxSelectedItem ? noSprite : noSelectedSprite;

	if (deleteBoxSelectedItem == 2)
		return;

	baseSprite.setPosition({160, 192});
	baseSprite.setSize(baseSprite.texture.getSize());
	baseSprite.rect = {
		0, 0,
		static_cast<int>(baseSprite.texture.getSize().x),
		static_cast<int>(baseSprite.texture.getSize().y)
	};
	baseSprite.tint = DrawUtils::DxSokuColor::White;
	baseSprite.draw();

	yes.setPosition({242, 228});
	yes.setSize(yes.texture.getSize());
	yes.rect = {
		0, 0,
		static_cast<int>(yes.texture.getSize().x),
		static_cast<int>(yes.texture.getSize().y)
	};
	yes.tint = DrawUtils::DxSokuColor::White;
	yes.draw();

	no.setPosition({338, 228});
	no.setSize(no.texture.getSize());
	no.rect = {
		0, 0,
		static_cast<int>(no.texture.getSize().x),
		static_cast<int>(no.texture.getSize().y)
	};
	no.tint = DrawUtils::DxSokuColor::White;
	no.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, ("Delete deck " + editedDecks[editSelectedDeck].name + " ?").c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, nullptr, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	textSprite.setPosition({164, 202});
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = DrawUtils::DxSokuColor::White;
	textSprite.draw();
}

void openDeleteBox()
{
	SokuLib::playSEWaveBuffer(0x28);
	deleteBoxDisplayed = true;
	deleteBoxSelectedItem = 0;
}

void deleteBoxUpdate(SokuLib::KeyManager *keys)
{
	auto horizontal = abs(keys->keymapManager->input.horizontalAxis);

	if (deleteBoxSelectedItem == 2) {
		if (!keys->keymapManager->input.a)
			deleteBoxDisplayed = false;
		return;
	}
	if (keys->keymapManager->input.b || SokuLib::checkKeyOneshot(1, false, false, false)) {
		SokuLib::playSEWaveBuffer(0x29);
		deleteBoxDisplayed = false;
	}
	if (horizontal == 1 || (horizontal >= 36 && horizontal % 6 == 0)) {
		deleteBoxSelectedItem = !deleteBoxSelectedItem;
		SokuLib::playSEWaveBuffer(0x27);
	}
	if (keys->keymapManager->input.a == 1) {
		if (deleteBoxSelectedItem) {
			editedDecks.erase(editedDecks.begin() + editSelectedDeck);
			loadDeckToGame(SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>(), editedDecks[editSelectedDeck].cards);
			SokuLib::playSEWaveBuffer(0x28);
		} else
			SokuLib::playSEWaveBuffer(0x29);
		deleteBoxSelectedItem = 2;
	}
}

void copyBoxRender()
{
	int text;
	int width;
	DrawUtils::Sprite textSprite;

	baseSprite.setPosition({160, 192});
	baseSprite.setSize(baseSprite.texture.getSize());
	baseSprite.rect = {
		0, 0,
		static_cast<int>(baseSprite.texture.getSize().x),
		static_cast<int>(baseSprite.texture.getSize().y)
	};
	baseSprite.tint = DrawUtils::DxSokuColor::White;
	baseSprite.draw();

	const std::string &name = copyBoxSelectedItem == editedDecks.size() - 1 ? "Default deck" : editedDecks[copyBoxSelectedItem].name;

	if (!SokuLib::textureMgr.createTextTexture(&text, name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}

	constexpr float increase = 1;
	DrawUtils::Vector2<int> pos{static_cast<int>(321 - (width / 2) * increase), 230};

	textSprite.setPosition(pos);
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({static_cast<unsigned>(TEXTURE_SIZE * increase), static_cast<unsigned>((FONT_HEIGHT + 18) * increase)});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = DrawUtils::DxSokuColor::White;
	textSprite.draw();

	pos.x -= 32 * increase;
	pos.y -= 6 * increase;
	arrowSprite.rect = {0, 0, 32, 32};
	arrowSprite.setPosition(pos);
	arrowSprite.setSize({static_cast<unsigned>(32 * increase + 1), static_cast<unsigned>(32 * increase + 1)});
	arrowSprite.tint = DrawUtils::DxSokuColor::White;
	arrowSprite.draw();

	pos.x += 32 * increase + width * increase;
	arrowSprite.rect.left = 32;
	arrowSprite.setPosition(pos);
	arrowSprite.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, "Choose a deck to copy", font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	textSprite.setPosition({166, 200});
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.draw();
}

void openCopyBox(SokuLib::ProfileDeckEdit *This)
{
	SokuLib::playSEWaveBuffer(0x28);
	copyBoxDisplayed = true;
	copyBoxSelectedItem = 0;
}

void copyBoxUpdate(SokuLib::KeyManager *keys)
{
	if (keys->keymapManager->input.b || SokuLib::checkKeyOneshot(1, false, false, false)) {
		SokuLib::playSEWaveBuffer(0x29);
		copyBoxDisplayed = false;
	}

	auto horizontal = abs(keys->keymapManager->input.horizontalAxis);

	if (horizontal == 1 || (horizontal >= 36 && horizontal % 6 == 0)) {
		if (keys->keymapManager->input.horizontalAxis < 0) {
			if (copyBoxSelectedItem == 0)
				copyBoxSelectedItem = editedDecks.size() - 1;
			else
				copyBoxSelectedItem--;
		} else {
			if (copyBoxSelectedItem == editedDecks.size() - 1)
				copyBoxSelectedItem = 0;
			else
				copyBoxSelectedItem++;
		}
		SokuLib::playSEWaveBuffer(0x27);
	}
	if (keys->keymapManager->input.a == 1) {
		auto menu = SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>();

		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		loadDeckToGame(SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>(), copyBoxSelectedItem == editedDecks.size() - 1 ? defaultDecks[menu->editedCharacter] : editedDecks[copyBoxSelectedItem].cards);
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		copyBoxDisplayed = false;
		openRenameBox(menu);
	}
}

int __fastcall CProfileDeckEdit_OnProcess(SokuLib::ProfileDeckEdit *This)
{
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);
	bool deleteBox = deleteBoxDisplayed;
	bool renameBox = renameBoxDisplayed;
	bool copyBox = copyBoxDisplayed;

	if (renameBox)
		renameBoxUpdate(keys);
	else if (deleteBox)
		deleteBoxUpdate(keys);
	else if (copyBox)
		copyBoxUpdate(keys);
	else if (keys->keymapManager->input.a == 1 && This->cursorOnDeckChangeBox) {
		if (editSelectedDeck == editedDecks.size() - 1)
			openCopyBox(This);
		else
			openRenameBox(This);
	} else if (keys->keymapManager->input.c && This->cursorOnDeckChangeBox && editSelectedDeck != editedDecks.size() - 1)
		openDeleteBox();
	if (renameBox || deleteBox || copyBox) {
		escPressed = ((int *)0x8998D8)[1];
		((int *)0x8998D8)[1] = 0;
		((int *)0x8998D8)[DIK_F1] = 0;
		memset(&keys->keymapManager->input, 0, sizeof(keys->keymapManager->input));
	} else if (escPressed) {
		escPressed = ((int *)0x8998D8)[1];
		((int *)0x8998D8)[1] = 0;
	} else if (editSelectedDeck == editedDecks.size() - 1 && This->displayedNumberOfCards != 0) {
		SokuLib::playSEWaveBuffer(0x28);
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	// This hides the deck select arrow
	((bool ***)This)[0x10][0x2][20] = false;
	return (This->*s_originalCProfileDeckEdit_OnProcess)();
}

int __fastcall CProfileDeckEdit_OnRender(SokuLib::ProfileDeckEdit *This)
{
	if (firstConstructLoad) {
		errorCounter = 0;
		editSelectedDeck = 0;
		if (editSelectedProfile != 2) {
			loadedDecks[2] = loadedDecks[editSelectedProfile];
			for (int i = 0; i < 20; i++)
				loadedDecks[2][i].push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		}
		editedDecks = loadedDecks[2][This->editedCharacter];
		loadDeckToGame(This, editedDecks[editSelectedDeck].cards);
		deleteBoxDisplayed = false;
		renameBoxDisplayed = false;
		copyBoxDisplayed = false;
	}

	if (saveError) {
		saveError = false;
		This->editedDeck->vector()[0]--;
	}

	int ret = (This->*s_originalCProfileDeckEdit_OnRender)();
	DrawUtils::Sprite &sprite = cardsTextures[SokuLib::CHARACTER_RANDOM][21];
	DrawUtils::Sprite textSprite;
	DrawUtils::Vector2<int> pos{38, 88};
	int text;
	int width = 0;

	if (firstLoad) {
		loadCardAssets();
		initFont();
	}
	firstConstructLoad = false;
	firstLoad = false;

	loadTexture(sprite, "data/battle/cardFaceDown.bmp");
	sprite.rect.top = sprite.rect.width = 0;
	sprite.rect.width = sprite.texture.getSize().x;
	sprite.rect.height = sprite.texture.getSize().y;
	sprite.tint = DrawUtils::DxSokuColor::White;
	sprite.setSize({20, 32});
	for (int i = 20; i > This->displayedNumberOfCards; i--) {
		sprite.setPosition({304 + 24 * ((i - 1) % 10), 260 + 38 * ((i - 1) / 10)});
		sprite.draw();
	}

	if (errorCounter) {
		float alpha = min(1.f, errorCounter / 30.f);

		errorCounter--;
		if (!SokuLib::textureMgr.createTextTexture(&text, "Please keep the number of cards in the deck at or below 20", font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
			puts("C'est vraiment pas de chance");
			return ret;
		}

		auto realX = 53;

		if (errorCounter >= 105) {
			realX += rand() % 31 - 15;
			if (errorCounter >= 115)
				alpha = 1 - (errorCounter - 115.f) / 5;
		}

		textSprite.setPosition({realX, pos.y - 20});
		textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
		textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
		textSprite.rect = {
			0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
		};
		textSprite.tint = DrawUtils::DxSokuColor::Red * alpha;
		textSprite.draw();
	}

	if (!SokuLib::textureMgr.createTextTexture(&text, editedDecks[editSelectedDeck].name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return ret;
	}
	pos.x = 153 - width / 2;
	textSprite.setPosition(pos);
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = DrawUtils::DxSokuColor::White;
	textSprite.draw();

	pos.x -= 32;
	pos.y -= 6;
	arrowSprite.rect = {0, 0, 32, 32};
	arrowSprite.setPosition(pos);
	arrowSprite.setSize({33, 33});
	arrowSprite.tint = DrawUtils::DxSokuColor::White;
	arrowSprite.draw();

	pos.x += 32 + width;
	arrowSprite.rect.left = 32;
	arrowSprite.setPosition(pos);
	arrowSprite.draw();

	if (renameBoxDisplayed)
		renameBoxRender();
	else if (deleteBoxDisplayed)
		deleteBoxRender();
	else if (copyBoxDisplayed)
		copyBoxRender();
	return ret;
}

int __fastcall CTitle_OnProcess(SokuLib::Title *This)
{
	if (!SokuLib::menuManager.isInMenu || *SokuLib::getMenuObj<int>() != SokuLib::ADDR_VTBL_DECK_CONSTRUCTION_CHR_SELECT_MENU) {
		firstConstructLoad = true;
		editSelectedProfile = 2;
	}
	return (This->*s_originalTitleOnProcess)();
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old;
	FILE *_;

	GetModuleFileName(hMyModule, profilePath, 1024);
	while (*profilePath && profilePath[strlen(profilePath) - 1] == '\\')
		profilePath[strlen(profilePath) - 1] = 0;

	auto result = strrchr(profilePath, '\\');

	if (result)
		*result = 0;
#ifdef _DEBUG
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif
	puts("Hey !");
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_originalSelectCLOnProcess          = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onProcess, CSelectCL_OnProcess);
	s_originalSelectSVOnProcess          = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onProcess, CSelectSV_OnProcess);
	s_originalSelectOnProcess            = SokuLib::TamperDword(&SokuLib::VTable_Select.onProcess, CSelect_OnProcess);
	s_originalSelectCLOnRender           = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onRender, CSelectCL_OnRender);
	s_originalSelectSVOnRender           = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onRender, CSelectSV_OnRender);
	s_originalSelectOnRender             = SokuLib::TamperDword(&SokuLib::VTable_Select.onRender, CSelect_OnRender);
	s_originalTitleOnProcess             = SokuLib::TamperDword(&SokuLib::VTable_Title.onRender, CTitle_OnProcess);
	s_originalCProfileDeckEdit_OnProcess = SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onProcess, CProfileDeckEdit_OnProcess);
	s_originalCProfileDeckEdit_OnRender  = SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onRender, CProfileDeckEdit_OnRender);
	SokuLib::TamperDword(SENDTO_JUMP_ADDR, mySendTo);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//Force deck icon to be hidden in character select
	*(unsigned char *)0x4210e2 = 0xEB;
	//Force deck icon to be hidden in deck construction
	memset((void *)0x0044E4ED, 0x90, 35);
	SokuLib::TamperNearJmpOpr(0x450230, CProfileDeckEdit_SwitchCurrentDeck);
	s_originalDrawGradiantBar = reinterpret_cast<void (*)(float, float, float)>(SokuLib::TamperNearJmpOpr(0x44E4C8, drawGradiantBar));
	s_originalInputMgrGet = SokuLib::union_cast<int (SokuLib::ObjectSelect::*)()>(SokuLib::TamperNearJmpOpr(0x4206B3, myGetInput));
	s_origLoadDeckData = reinterpret_cast<void (__stdcall *)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<unsigned short> &)>(
		SokuLib::TamperNearJmpOpr(0x437D23, loadDeckData)
	);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	profileTrampoline = new SokuLib::Trampoline(0x435377, onProfileChanged, 7);
	deckSavedTrampoline = new SokuLib::Trampoline(0x450121, onDeckSaved, 6);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	if (fdwReason == DLL_PROCESS_DETACH) {
		if (profileTrampoline) {
			delete profileTrampoline;
			delete deckSavedTrampoline;
		}
	}
	return TRUE;
}