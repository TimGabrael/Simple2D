#include "GameManager.h"
#include <filesystem>
#include <charconv>
#include "Entitys.h"

int main()
{
	GameState* game = CreateGameState("Example2D", 1600, 900, -10.0f);
	GameManager* manager = GM_CreateGameManager(game);
	game->manager = manager;

	{
		AtlasBuildData* build = AM_BeginAtlasTexture();
		std::filesystem::path path = "Assets/food";
		std::vector<std::string> fileList;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".png")
			{
				fileList.emplace_back(entry.path().string());
			}
		}
		std::sort(fileList.begin(), fileList.end(), [](std::string& s1, std::string& s2) {
			size_t n1 = s1.find('_', 0);
			size_t n2 = s2.find('_', 0);
			std::string_view v1(s1.c_str(), n1);
			std::string_view v2(s2.c_str(), n2);
			int i1 = 0;
			int i2 = 0;
			std::from_chars(s1.c_str() + 12, s1.c_str()+n1, i1);
			std::from_chars(s2.c_str() + 12, s2.c_str()+n2, i2);
			return i1 < i2;

		});
		for (const auto& f : fileList)
		{
			AM_AtlasAddFromFile(build, f.c_str());
		}
		manager->atlas = AM_EndTextureAtlas(build, false);
	}

	{
		SceneObject* base = CreateBaseObject(game->scene);
		SceneObject* ball = CreateBallObject(game->scene, {0.0f, 1.5f}, {0.0f, -1.0f }, 0.05f);
		for (uint32_t i = 0; i < 30; i++)
		{
			SceneObject* peg = CreatePegObject(game->scene, { GetRandomFloat(-1.5f, 1.5f), GetRandomFloat(-1.5f, 1.5f) }, 0.05f);
		}
	}



	UpateGameState();
	return 0;
}