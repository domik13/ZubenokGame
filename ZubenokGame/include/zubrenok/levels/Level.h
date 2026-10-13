#pragma once

#include "zubrenok/game/GameTypes.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <vector>

namespace zubrenok
{
	


	class Level
	{
	public:
		Level();

		void reset();

		void draw(
			sf::RenderTarget& target,
			float animationTime
		) const;

		bool isInside(TilePosition position) const;
		bool isWalkable(TilePosition position) const;

		bool isWorldPositionWalkable(
			sf::Vector2f worldPosition,
			float collisionRadius
		) const;

		bool canMove(
			TilePosition position,
			Direction direction
		) const;

		bool isNearTileCenter(
			sf::Vector2f worldPosition,
			float tolerance
		) const;

		TilePosition worldToTile(
			sf::Vector2f worldPosition
		) const;

		sf::Vector2f tileToWorld(
			TilePosition tilePosition
		) const;

		CollectibleType consumeCollectible(
			sf::Vector2f worldPosition
		);

		int getRemainingCollectibles() const;

		TilePosition getPlayerSpawn() const;

		const std::vector<TilePosition>&
			getEnemySpawns() const;

		int calculateShortestDistance(
			TilePosition start,
			TilePosition target
		) const;

	private:
		std::vector<std::string> tiles_;

		TilePosition playerSpawn_;
		std::vector<TilePosition> enemySpawns_;

		int remainingCollectibles_ = 0;

		sf::Texture presetsTexture_;
		mutable sf::Sprite presetsSprite_{ presetsTexture_ };

		void loadPresets();

		void loadDefaultMap();
		void parseMapObjects();

		void drawFloor(
			sf::RenderTarget& target
		) const;

		void drawWall(
			sf::RenderTarget& target,
			TilePosition position
		) const;

		void drawBerry(
			sf::RenderTarget& target,
			sf::Vector2f position,
			float animationTime,
			int seed
		) const;

		void drawAcorn(
			sf::RenderTarget& target,
			sf::Vector2f position,
			float animationTime
		) const;

		TilePosition findNearestWalkableTile(
			TilePosition target
		) const;
	};
}