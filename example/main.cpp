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
		{
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
				std::from_chars(s1.c_str() + 12, s1.c_str() + n1, i1);
				std::from_chars(s2.c_str() + 12, s2.c_str() + n2, i2);
				return i1 < i2;

				});
			for (const auto& f : fileList)
			{
				AM_AtlasAddFromFile(build, f.c_str());
			}
		}
		{
			std::filesystem::path path = "Assets/fox";
			std::vector<std::string> fileList;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".png")
				{
					fileList.emplace_back(entry.path().string());
				}
			}
			for (const auto& f : fileList)
			{
				AM_AtlasAddFromFile(build, f.c_str());
			}
		}
		{
			std::filesystem::path path = "Assets/slime";
			std::vector<std::string> fileList;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".png")
				{
					fileList.emplace_back(entry.path().string());
				}
			}
			for (const auto& f : fileList)
			{
				AM_AtlasAddFromFile(build, f.c_str());
			}
		}
		AM_AtlasAddGlyphRangeFromFile(build, "Assets/consola.ttf", 'A', 'z' + 1, 20.0f);
		manager->atlas = AM_EndTextureAtlas(build, false);
	}

	{
		WavFile* clackWav = AU_LoadFile(game->audio, "Assets/clack.wav");
		WavFile* slimeDieWav = AU_LoadFile(game->audio, "Assets/slime_die.wav");
		WavFile* slimeAttackWav = AU_LoadFile(game->audio, "Assets/slime_attack.wav");
		manager->audioFiles.push_back(clackWav);
		manager->audioFiles.push_back(slimeDieWav);
		manager->audioFiles.push_back(slimeAttackWav);

	}
	
	GM_FillScene();
	



	UpateGameState();

	CleanUpGameState(game);
	return 0;
}