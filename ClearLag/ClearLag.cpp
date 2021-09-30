#include <iostream>
#include <string.h>
#include <lbpch.h>
#include <loader/Loader.h>
#include <string>
#include <JsonLoader.h>
#include <chrono>
#include <fstream>
#include <mc/Player.h>
#include <stl\varint.h>
#include <api/xuidreg/xuidreg.h>
#include <mc/Item.h>
#include <random>
#include <windows.h>
#include "log.h"
#include <api/scheduler/scheduler.h>
#include <api/myPacket.h>
#include <map>
#include <set>
#include"hash_Set.h"
#include "hash_set.cpp"

std::time_t getTimeStamp()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	std::time_t timestamp = tmp.count();
	return timestamp;
}
int _access(const char
	* path, int mode);

using namespace std;
#pragma warning(disable:4996)

inline string gettime()
{
	time_t rawtime;
	tm* LocTime;
	char timestr[20];
	time(&rawtime);
	LocTime = localtime(&rawtime);
	strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", LocTime);
	return string(timestr);
}

inline void fw(const string& filen, const string& instr)
{
	if (filen == "plugins\\ClearLag\\ClearLag.json")
		return;
	if ((_access(filen.c_str(), 0)) != -1)
	{
		ofstream outfile;
		outfile.open(filen, ios::app);
		if (!outfile)
		{
			cout << "[" << gettime() << u8" INFO] FileWriteFailed!!!" << endl;
		}
		outfile << instr << endl;
		outfile.close();
	}
	else
	{
		std::ofstream outfile(filen);
		if (!outfile)
		{
			cout << "[" << gettime() << u8" INFO] FileWriteFailed!!!" << endl;
		}
		unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		outfile.write((char*)bom, sizeof(bom));
		outfile.close();
		fw(filen, instr);
	}
}

rapidjson::Document config;
void loadconfig() {
	ifstream in("plugins\\ClearLag\\ClearLag.json");
	ostringstream tmp;
	tmp << in.rdbuf();
	in.close();
	string data = tmp.str();
	config.Parse(data.c_str());
	int size = config.Size();
	if (size == 0)
		cout << "[" << gettime() << u8" Error] No Member Found!!!" << endl;
}

SRWLOCK lock;

int nums = 0;
namespace bc {
	void bctext(string a1, TextType a2) {
		auto pls = liteloader::getAllPlayers();
		for (auto i : pls) {
			WPlayer(*i).sendText(a1, a2);
		}
	}
}

hash_set<void*> acc;

Actor* id2ac(ActorUniqueID id) {
	if (id) {
		return SymCall("?fetchEntity@Level@@UEBAPEAVActor@@UActorUniqueID@@_N@Z"
			, Actor*, Level*, ActorUniqueID, bool)(LocateService<Level>(), id, 0);
	}
	return nullptr;
}



#include <mc/OffsetHelper.h>
Tick* Raw_GetEntityLastTick(Actor* ac)
{
	auto bs = offPlayer::getBlockSource(ac);
	auto bpos = ((Vec3)ac->getPos()).toBlockPos();
	void* lc = SymCall("?getChunkAt@BlockSource@@QEBAPEAVLevelChunk@@AEBVBlockPos@@@Z",
		void*, BlockSource*, BlockPos*)(bs, &bpos);
	return SymCall("?getLastTick@LevelChunk@@QEBAAEBUTick@@XZ"
		, Tick*, void*)(lc);
}

Dimension* Raw_GetDimByLevel(Level* lv, int id) {
	return SymCall("?getDimension@Level@@UEBAPEAVDimension@@V?$AutomaticID@VDimension@@H@@@Z",
		Dimension*, void*, int)(lv, id);
}

std::vector<Actor*> GetAllEntities(int dimid)
{
	auto lv = LocateService<Minecraft>()->getLevel();
	std::vector<Actor*> entityList;
	auto dim = Raw_GetDimByLevel(lv, dimid);
	if (!dim)
		return entityList;
	auto& list = *(std::unordered_map<long, void*>*)((uintptr_t)dim + 304);     //Dimension::getEntityIdMap

	//Check Valid
	auto currTick = SymCall("?getCurrentTick@Level@@UEBAAEBUTick@@XZ"
		, Tick*, Level*)(lv)->t;
	for (auto& i : list)
	{
		auto entity = SymCall("??$tryUnwrap@VActor@@$$V@WeakEntityRef@@QEBAPEAVActor@@XZ",
			Actor*, void*)(&i.second);
		if (!entity)
			continue;
		auto lastTick = Raw_GetEntityLastTick(entity)->t;
		if (currTick - lastTick == 0 || currTick - lastTick == 1)
			entityList.push_back(entity);
	}
	return entityList;
}

std::vector<Actor*> getAllEntities()
{
	auto lv = (uintptr_t)LocateService<Minecraft>()->getLevel();
	std::vector<Actor*> entityList;
	auto dim0 = GetAllEntities(0);
	auto dim1 = GetAllEntities(1);
	auto dim2 = GetAllEntities(2);
	entityList.insert(entityList.end(), dim0.begin(), dim0.end());
	entityList.insert(entityList.end(), dim1.begin(), dim1.end());
	entityList.insert(entityList.end(), dim2.begin(), dim2.end());
	return entityList;
}

bool remove(Actor* a1) {
	ActorUniqueID id;
	void* isValid;
	__try
	{
		id = a1->getUniqueID();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	auto acc = id2ac(id);
	if (acc != nullptr) {
		SymCall("?remove@Actor@@UEAAXXZ", void, Actor*)(acc);
		nums++;
		return true;
	}
	return false;
}

#include <api/scheduler/scheduler.h>
#include <malloc.h>
map<string, int > tick;
void schTask() {
	std::thread t([] {
		bc::bctext(u8"§6[§eClearLag§6]§r 服务器生物与掉落物将在60秒后清理", RAW);
		//std::this_thread::sleep_for(std::chrono::seconds(30));
		bc::bctext(u8"§6[§eClearLag§6]§r 服务器生物与掉落物将在30秒后清理", RAW);
		//std::this_thread::sleep_for(std::chrono::seconds(20));
		bc::bctext(u8"§6[§eClearLag§6]§r 服务器生物与掉落物将在10秒后清理", RAW);
		//std::this_thread::sleep_for(std::chrono::seconds(7));
		bc::bctext(u8"§6[§eClearLag§6]§r 服务器生物与掉落物将在3秒后清理", RAW);
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		bc::bctext(u8"§6[§eClearLag§6]§r 服务器生物与掉落物将在2秒后清理", RAW);
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		bc::bctext(u8"§6[§eClearLag§6]§r 服务器生物与掉落物将在1秒后清理", RAW);
		long start = getTimeStamp();
		for (auto& v : config["ClearList"].GetArray()) {
			long starts = getTimeStamp();
			int num = 0;
			auto list = getAllEntities();
			for (auto actor : list)
			{
				auto name = SymCall("?getActorName@CommandUtils@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBVActor@@@Z", string, Actor*)(actor); {
					if (name == v.GetString()) {
						auto bo = remove(actor);
						if (bo) {
							num++;
						}
					}
				}
			}
			long ends = getTimeStamp();
			tick.insert(std::map < string, int > ::value_type(v.GetString(), num));
		}
		long end = getTimeStamp();
		bc::bctext(u8"§6[§eClearLag§6]§r 清理成功！共计清除 " + to_string(nums) + u8"个生物 总耗时：" + to_string(end - start) + u8"ms\n详细如下", RAW);
		//cout << u8"§6[§eClearLag§6]§r 清理成功！共计清除 " << to_string(nums) << u8"个生物 总耗时：" << to_string(end - start) << u8"ms\n详细如下" << endl;
		map<string, int>::reverse_iterator iter;
		for (iter = tick.rbegin(); iter != tick.rend(); iter++) {
			if (iter->second == 0) {
				continue;
			}
			bc::bctext(u8"类型：" + iter->first + u8" 共计：" + to_string(iter->second), RAW);
			//cout << u8"类型：" << iter->first << u8" 共计："  << to_string(iter->second) << endl;
		}
		map<string, int >().swap(tick);
		map<string, int >().clear();
		nums = 0;
		});
	t.detach();
}
void timer() {
	Handler::schedule(RepeatingTask([] {
		schTask();
		}, config["timer"].GetInt() * 2));
}
void entry() {
	loadconfig();
	Event::addEventListener([](ServerStartedEV ev) {
		InitializeSRWLock(&lock);
		timer();
		});
}
