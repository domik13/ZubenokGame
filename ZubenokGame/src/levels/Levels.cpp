#include "zubrenok/levels/Level.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Image.hpp>
#include <stdexcept>

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace zubrenok
{

	constexpr int presetCellW = 160;
	constexpr int presetCellH = 180;
	constexpr int treeCount = 12;

	namespace
	{
		constexpr char wallTile = '#';
		constexpr char berryTile = '.';
		constexpr char acornTile = 'o';
		constexpr char playerSpawnTile = 'P';
		constexpr char enemySpawnTile = 'E';
		constexpr char emptyTile = ' ';
	}

	Level::Level()
	{
		reset();
	}

	void Level::reset() {
		loadDefaultMap();

		loadPresets();

		

		parseMapObjects();
	}

	void Level::loadDefaultMap() {
		tiles_.assign(
			config::mapHeight,
			std::string(
				config::mapWidth,
				berryTile
			)
		);

		for (int x = 0; x < config::mapWidth; ++x)
		{
			tiles_[0][x] = wallTile;
			tiles_[config::mapHeight - 1][x] = wallTile;
		}

		for (int y = 0; y < config::mapHeight; ++y)
		{
			tiles_[y][0] = wallTile;
			tiles_[y][config::mapWidth - 1] = wallTile;
		}

		auto addHorizontWall = [this](
			int startX,
			int y,
			int lenght
			)
			{
				for (int x = startX; x < startX + lenght; x++)
				{
					if (x <= 0 || x >= config::mapWidth - 1 || y <= 0 || y >= config::mapHeight - 1)
					{
						continue;
					}

					tiles_[y][x] = wallTile;
				}
			};

		auto addVerticalWall = [this](
			int x,
			int startY,
			int length
			)
			{
				for (int y = startY; y < startY + length; ++y)
				{
					if (x <= 0 || x >= config::mapWidth - 1 || y <= 0 || y >= config::mapHeight - 1)
					{
						continue;
					}

					tiles_[y][x] = wallTile;
				}
			};

		addHorizontWall(3, 3, 8);
		addHorizontWall(14, 3, 12);
		addHorizontWall(29, 3, 8);

		addHorizontWall(5, 7, 11);
		addHorizontWall(24, 7, 11);

		addHorizontWall(3, 11, 9);
		addHorizontWall(15, 11, 10);
		addHorizontWall(28, 11, 9);

		addVerticalWall(10, 4, 6);
		addVerticalWall(20, 5, 5);
		addVerticalWall(30, 4, 6);

		addHorizontWall(8, 15, 10);
		addHorizontWall(22, 15, 10);
		addHorizontWall(3, 19, 8);
		addHorizontWall(29, 19, 8);

		addVerticalWall(15, 12, 8);
		addVerticalWall(24, 12, 8);

		addHorizontWall(12, 23, 16);
		addVerticalWall(7, 20, 5);
		addVerticalWall(32, 20, 5);

		tiles_[2][2] = acornTile;
		tiles_[2][config::mapWidth - 3] = acornTile;
		tiles_[2][20] = acornTile;
		tiles_[config::mapHeight - 3][2] = acornTile;
		tiles_[config::mapHeight - 3][config::mapWidth - 3] = acornTile;
		tiles_[14][5] = acornTile;
		tiles_[14][config::mapWidth - 6] = acornTile;

		// Keep spawn and enemy den clear of walls.
		for (int y = 19; y <= 21; ++y)
		{
			for (int x = 19; x <= 21; ++x)
			{
				tiles_[y][x] = berryTile;
			}
		}

		for (int y = 12; y <= 14; ++y)
		{
			for (int x = 17; x <= 22; ++x)
			{
				tiles_[y][x] = emptyTile;
			}
		}

		tiles_[20][20] = playerSpawnTile;

		tiles_[3][3] = enemySpawnTile;
		tiles_[3][config::mapWidth - 4] = enemySpawnTile;
		tiles_[config::mapHeight - 4][3] = enemySpawnTile;
		tiles_[config::mapHeight - 4][config::mapWidth - 4] = enemySpawnTile;
	}

	void Level::parseMapObjects()
	{
		playerSpawn_ = {1,1};
		enemySpawns_.clear();

		remainingCollectibles_ = 0;

		for (int y = 0; y < config::mapHeight; ++y)
		{
			for (int x = 0; x < config::mapWidth; ++x)
			{
				char& tile = tiles_[y][x];

				switch (tile)
				{
				case playerSpawnTile:
					playerSpawn_ = { x, y };
					tile = emptyTile;
					break;

				case enemySpawnTile:
					enemySpawns_.push_back(
						{ x, y }
					);

					tile = emptyTile;
					break;

				case berryTile:
					++remainingCollectibles_;
					break;

				case acornTile:
					++remainingCollectibles_;
					break;


				default:
					break;
				}
			}
		}
	}


	bool Level::isInside(
		TilePosition position
	) const
	{
		return
			position.x >= 0 &&
			position.y >= 0 &&
			position.x < config::mapWidth &&
			position.y < config::mapHeight;
	}

	bool Level::isWalkable(
		TilePosition position
	) const
	{
		if (!isInside(position))
		{
			return false;
		}

		return tiles_[position.y][position.x] != wallTile;
	}

	bool Level::isWorldPositionWalkable(
		sf::Vector2f worldPosition,
		float collisionRadius
	) const
	{
		const sf::Vector2f sampleOffsets[] = {
			{ 0.0f, 0.0f },
			{ -collisionRadius, -collisionRadius },
			{ collisionRadius, -collisionRadius },
			{ -collisionRadius, collisionRadius },
			{ collisionRadius, collisionRadius }
		};

		for (const sf::Vector2f& offset : sampleOffsets)
		{
			if (!isWalkable(worldToTile(worldPosition + offset)))
			{
				return false;
			}
		}

		return true;
	}

	bool Level::canMove(
		TilePosition position,
		Direction direction
	) const
	{
		if (direction == Direction::None)
		{
			return false;
		}

		const TilePosition offset = directionToOffset(direction);

		const TilePosition target{
			position.x + offset.x,
			position.y + offset.y
		};

		return isWalkable(target);
	}

	bool Level::isNearTileCenter(
		sf::Vector2f worldPosition,
		float tolerance
	) const
	{
		const TilePosition tile = worldToTile(worldPosition);

		if (!isInside(tile))
		{
			return false;
		}

		const sf::Vector2f center = tileToWorld(tile);

		return std::abs(worldPosition.x - center.x) <= tolerance &&
			std::abs(worldPosition.y - center.y) <= tolerance;
	}

	TilePosition Level::worldToTile(
		sf::Vector2f worldPosition
	) const
	{
		const float mapX = worldPosition.x;

		const float mapY = worldPosition.y - static_cast<float>(config::hudHeight);

		return {
			static_cast<int>(
					std::floor(
						mapX / static_cast<float>(config::tileSize)
					)
				),
			static_cast<int>(
					std::floor(
						mapY / static_cast<float>(config::tileSize)
					)
				)
		};
	}

	sf::Vector2f Level::tileToWorld(
		TilePosition tilePosition
	) const
	{
		const float halfTile = static_cast<float>(config::tileSize) / 2.0f;

		return {
			static_cast<float>(
					tilePosition.x * config::tileSize
				) + halfTile,

			static_cast<float>(
				config::hudHeight + 
					tilePosition.y * config::tileSize
				) + halfTile
		};
	}

	CollectibleType Level::consumeCollectible(
		sf::Vector2f worldPosition
	)
	{
		const TilePosition tilePostion = worldToTile(worldPosition);

		if (!isInside(tilePostion))
		{
			return CollectibleType::None;
		}

		char& tile = tiles_[tilePostion.y][tilePostion.x];

		if (tile == berryTile)
		{
			tile = emptyTile;

			remainingCollectibles_ = std::max(0, remainingCollectibles_ - 1);

			return CollectibleType::Berry;
		}

		if (tile == acornTile)
		{
			tile = emptyTile;

			remainingCollectibles_ = std::max(0, remainingCollectibles_ - 1);

			return CollectibleType::Acorn;
		}

		return CollectibleType::None;
	}

	int Level::getRemainingCollectibles() const
	{
		return remainingCollectibles_;
	}

	TilePosition Level::getPlayerSpawn() const {
		return playerSpawn_;
	}

	const std::vector<TilePosition>&
		Level::getEnemySpawns() const
	{
		return enemySpawns_;
	}
	
	int Level::calculateShortestDistance(
		TilePosition start,
		TilePosition target
	) const
	{
		if (!isWalkable(start))
		{
			return std::numeric_limits<int>::max();
		}

		target = findNearestWalkableTile(target);

		if (!isWalkable(target))
		{
			return std::numeric_limits<int>::max();
		}

		if (start == target)
		{
			return 0;
		}

		std::vector<std::vector<int>> distances(
			config::mapHeight,
			std::vector<int>(
				config::mapWidth,
				-1
			)
		);

		std::queue<TilePosition> queue;

		distances[start.y][start.x] = 0;
		queue.push(start);

		const Direction directions[] = {
			Direction::Up,
			Direction::Down,
			Direction::Left,
			Direction::Right
		};

		while (!queue.empty())
		{
			const TilePosition current = queue.front();
			queue.pop();

			for (Direction direction : directions)
			{
				const TilePosition offset = directionToOffset(direction);

				const TilePosition next{
					current.x + offset.x,
					current.y + offset.y
				};

				if (!isWalkable(next))
				{
					continue;
				}

				if (distances[next.y][next.x] != -1) {
					continue;
				}

				distances[next.y][next.x] = distances[current.y][current.x] + 1;

				if (next == target)
				{
					return distances[next.y][next.x];
				}

				queue.push(next);
			}
		}
		return std::numeric_limits<int>::max();
	}

	TilePosition Level::findNearestWalkableTile(
		TilePosition target
	) const
	{
		target.x = std::clamp(target.x, 0, config::mapWidth - 1);

		target.y = std::clamp(target.y, 0, config::mapHeight - 1);

		if (isWalkable(target))
		{
			return target;
		}

		std::queue<TilePosition> queue;

		std::vector<std::vector<bool>> visited(
			config::mapHeight,
			std::vector<bool>(
				config::mapWidth,
				false
			)
		);

		queue.push(target);
		visited[target.y][target.x] = true;

		const Direction directions[] = {
			Direction::Up,
			Direction::Down,
			Direction::Left,
			Direction::Right
		};

		while (!queue.empty())
		{
			const TilePosition current = queue.front();

			queue.pop();

			for (Direction direction : directions)
			{
				const TilePosition offset = directionToOffset(direction);

				const TilePosition next{
					current.x + offset.x,
					current.y + offset.y
				};

				if (!isInside(next))
				{
					continue;
				}

				if (visited[next.y][next.x])
				{
					continue;
				}

				visited[next.y][next.x] = true;

				if (isWalkable(next))
				{
					return next;
				}

				queue.push(next);
			}
		}

		return target;
	}

	void Level::draw(
		sf::RenderTarget& target,
		float animationTime
	) const
	{
		drawFloor(target);

		int berrySeed = 0;

		for (int y = 0; y < config::mapHeight; ++y)
		{
			for (int x = 0; x < config::mapWidth; ++x)
			{
				const TilePosition tilePosition{
					x, y
				};

				const char tile = tiles_[y][x];

				if (tile == wallTile)
				{
					drawWall(
						target,
						tilePosition
					);

					continue;
				}

				const sf::Vector2f worldPosition = tileToWorld(tilePosition);

				if (tile == berryTile)
				{
					drawBerry(
						target,
						worldPosition,
						animationTime,
						berrySeed
					);

					++berrySeed;
				}
				else if (tile == acornTile)
				{
					drawAcorn(
						target,
						worldPosition,
						animationTime
					);
				}
			}
		}
	}

	void Level::drawFloor(
		sf::RenderTarget& target
	) const
	{
		sf::RectangleShape background(
			{
				static_cast<float>(
						config::windowWidth
					),

					static_cast<float>(
							config::mapHeight * config::tileSize
						)
			}
		);

		background.setPosition(
			{
				0.0f,
				static_cast<float>(
						config::hudHeight
					)
			}
		);

		background.setFillColor(
			sf::Color(
				20,
				35,
				25
			)
		);

		target.draw(background);

		sf::RectangleShape tileShape(
			{
				static_cast<float>(
						config::tileSize
					),

				static_cast<float>(
						config::tileSize
					)
			}
		);

		for (int y = 0; y < config::mapHeight; ++y)
		{
			for (int x = 0; x < config::mapWidth; ++x)
			{
				if (tiles_[y][x] == wallTile) {
					continue;					
				}

				tileShape.setPosition(
					{
						static_cast<float>(
								x * config::tileSize
							),

						static_cast<float>(
								config::hudHeight + y * config::tileSize
							)
					}
				);

				if ((x + y) % 2 == 0) {
					tileShape.setFillColor(
						sf::Color(
							30, 50, 35
						)
					);
				}
				else {
					tileShape.setFillColor(
						sf::Color(
							25, 45, 30
						)
					);
				}

				target.draw(tileShape);
			}
		}
	}

	void Level::drawWall(
		sf::RenderTarget& target,
		TilePosition position
	) const
	{
		const int column =
			(position.x * 31 + position.y * 17) % treeCount;

		presetsSprite_.setTextureRect(
			sf::IntRect(
				{ column * presetCellW, 0 },
				{ presetCellW, presetCellH }
			)
		);

		const float scale =
			static_cast<float>(config::tileSize) / static_cast<float>(presetCellW);

		presetsSprite_.setScale({ scale, scale });
		presetsSprite_.setOrigin(
			{
				presetCellW * 0.5f,
				static_cast<float>(presetCellH)
			}
		);

		const sf::Vector2f center = tileToWorld(position);
		presetsSprite_.setPosition(
			{
				center.x,
				center.y + static_cast<float>(config::tileSize) * 0.5f
			}
		);

		target.draw(presetsSprite_);
	}

	void Level::drawBerry(
		sf::RenderTarget& target,
		sf::Vector2f position,
		float animationTime,
		int seed
	) const
	{
		const float phase = animationTime * 4.0f + static_cast<float>(seed) * 0.35f;

		const float pulse = 1.0f + std::sin(phase) * 0.2f;

		const float radius = 4.0f * pulse;

		sf::CircleShape berry(radius);

		berry.setOrigin(
			{
				radius,
				radius
			}
		);

		berry.setPosition(position);

		berry.setFillColor(
			sf::Color(
				200,
				50,
				70
			)
		);

		target.draw(berry);

		sf::CircleShape highlight(
			radius * 0.32f
		);

		highlight.setOrigin(
			{
				radius * 0.32f,
				radius * 0.32f
			}
		);

		highlight.setPosition(
			{
				position.x - radius * 0.25f,
				position.y - radius * 0.25f
			}
		);

		highlight.setFillColor(
			sf::Color(
				255,
				166, 170
			)
		);

		target.draw(highlight);
	}

	void Level::drawAcorn(
		sf::RenderTarget& target,
		sf::Vector2f position,
		float animationTime
	) const
	{
		const float bob = std::sin(animationTime * 3.5f) * 2.0f;

		position.y += bob;

		sf::CircleShape glow(12.0f);

		glow.setOrigin(
			{
				12.0f,
				12.0f
			}
		);

		glow.setPosition(position);

		glow.setFillColor(
			sf::Color(
				230, 176, 67, 45
			)
		);

		target.draw(glow);

		sf::CircleShape body(
			7.0f
		);

		body.setOrigin(
			{
				7.0f,
				7.0f
			}
		);

		body.setScale(
			{
				0.85f, 1.2f
			}
		);

		body.setPosition(
			{
				position.x,
				position.y + 2.0f
			}
		);

		body.setFillColor(
			sf::Color(
				190, 125, 54
			)
		);

		target.draw(body);

		sf::RectangleShape cap(
			{
				12.0f,
				5.0f
			}
		);

		cap.setOrigin(
			{
				6.0f,
				2.5f
			}
		);

		cap.setPosition(
			{
				position.x,
				position.y - 4.0f
			}
		);

		cap.setFillColor(
			sf::Color(
				103,
				69, 38
			)
		);

		target.draw(cap);
	}

	void Level::loadPresets()
	{
		sf::Image image;
		if (!image.loadFromFile("assets/textures/background.png"))
		{
			throw std::runtime_error("Failed to load assets/textures/background.png");
		}

		image.createMaskFromColor(sf::Color::Black);

		if (!presetsTexture_.loadFromImage(image))
		{
			throw std::runtime_error("Failed to create presets texture");
		}

		presetsTexture_.setSmooth(false);
		presetsSprite_.setTexture(presetsTexture_, true);
	}

}
