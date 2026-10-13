#include "zubrenok/app/Application.h"

#include "zubrenok/entities/Player.h"
#include "zubrenok/game/GameTypes.h"
#include "zubrenok/levels/Level.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <filesystem>
#include <string>

namespace zubrenok
{
	namespace
	{
		std::filesystem::path getDatabasePath()
		{
			return std::filesystem::path("data") / "zubrenok.db";
		}

		bool loadHudFont(sf::Font& font)
		{
			const char* candidates[] = {
				"assets/fonts/arial.ttf",
				"C:/Windows/Fonts/arial.ttf",
				"C:/Windows/Fonts/segoeui.ttf"
			};

			for (const char* path : candidates)
			{
				if (font.openFromFile(path))
				{
					return true;
				}
			}

			return false;
		}
	}

	Application::Application()
		: database_(getDatabasePath())
	{
		database_.initialize();
	}

	int Application::run()
	{
		sf::RenderWindow window(
			sf::VideoMode(
				{
					static_cast<unsigned int>(config::windowWidth),
					static_cast<unsigned int>(config::windowHeight)
				}
			),
			"Zubrenok"
		);
		window.setFramerateLimit(144);

		sf::Font font;
		const bool hasFont = loadHudFont(font);

		Level level;
		Player player;
		player.load();
		player.setPosition(level.tileToWorld(level.getPlayerSpawn()));

		int score = 0;
		GameState state = GameState::Playing;

		sf::Clock clock;
		float animationTime = 0.0f;

		const auto resetRound = [&]()
		{
			level.reset();
			player.setPosition(level.tileToWorld(level.getPlayerSpawn()));
			score = 0;
			state = GameState::Playing;
			animationTime = 0.0f;
		};

		while (window.isOpen())
		{
			while (const auto event = window.pollEvent())
			{
				if (event->is<sf::Event::Closed>())
				{
					window.close();
				}

				if (const auto* key =
						event->getIf<sf::Event::KeyPressed>())
				{
					if (key->code == sf::Keyboard::Key::Escape)
					{
						window.close();
					}

					if (key->code == sf::Keyboard::Key::R)
					{
						resetRound();
					}
				}
			}

			const float deltaTime = clock.restart().asSeconds();
			animationTime += deltaTime;

			sf::SoundBuffer bonusBuffer;
			if (!bonusBuffer.loadFromFile("assets/audio/bonus.wav"))
			{
				throw std::runtime_error("Failed to load assets/audio/bonus.wav");
			}

			sf::Sound bonusSound(bonusBuffer);
			bonusSound.setVolume(10.0f);

			if (state == GameState::Playing)
			{
				const CollectibleType collected =
					player.update(deltaTime, level);

				if (collected == CollectibleType::Berry)
				{
					score += config::berryScore;
				}
				else if (collected == CollectibleType::Acorn)
				{
					score += config::acornScore;
					bonusSound.play();
				}

				if (level.getRemainingCollectibles() <= 0)
				{
					score += config::victoryScore;
					state = GameState::Victory;
				}
			}

			window.clear(sf::Color(14, 22, 17));

			sf::RectangleShape hudBar(
				{
					static_cast<float>(config::windowWidth),
					static_cast<float>(config::hudHeight)
				}
			);
			hudBar.setFillColor(sf::Color(18, 28, 22));
			window.draw(hudBar);

			level.draw(window, animationTime);
			player.draw(window);

			if (hasFont)
			{
				sf::Text scoreText(font, "Score: " + std::to_string(score), 28);
				scoreText.setFillColor(sf::Color(230, 230, 220));
				scoreText.setPosition({ 24.0f, 24.0f });
				window.draw(scoreText);

				sf::Text leftText(
					font,
					"Left: " + std::to_string(level.getRemainingCollectibles()),
					28
				);
				leftText.setFillColor(sf::Color(180, 210, 170));
				leftText.setPosition({ 280.0f, 24.0f });
				window.draw(leftText);

				sf::Text helpText(font, "WASD / Arrows  |  R - restart  |  Esc - exit", 18);
				helpText.setFillColor(sf::Color(140, 160, 140));
				helpText.setPosition(
					{
						static_cast<float>(config::windowWidth) - 420.0f,
						32.0f
					}
				);
				window.draw(helpText);

				if (state == GameState::Victory)
				{
					sf::Text victory(
						font,
						"Victory!  Score: " + std::to_string(score) + "\nPress R to play again",
						40
					);
					victory.setFillColor(sf::Color(255, 220, 120));
					victory.setPosition(
						{
							static_cast<float>(config::windowWidth) * 0.28f,
							static_cast<float>(config::windowHeight) * 0.42f
						}
					);
					window.draw(victory);
				}
			}

			window.display();
		}

		return 0;
	}
}
