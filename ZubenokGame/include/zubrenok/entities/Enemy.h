#pragma once

#include "zubrenok/game/GameTypes.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

namespace zubrenok {
	class Level1;

	class Enemy {
	public:
		Enemy();

		void load();
		void setPosition(sf::Vector2f position);
		sf::Vector2f getPosition() const;

		void update(float deltatime, Level1& level);
		void draw(sf::RenderTarget& target) const;

	private:
		enum class Facing
		{
			Right,
			Left,
			Up,
			Down
		};

		void pickNewDirection(Level1& level);
		void updateAnimation(float deltaTime, bool moving);
		void updateSpriteFrame();

		sf::Texture texture_;
		sf::Sprite sprite_;

		Facing facing_ = Facing::Down;
		Direction moveDir_ = Direction::None;

		int animationFrame_ = 1;

		float speed_ = 122.0f;
		float animationTimer_ = 0.0f;
		float rethinkTimer_ = 0.0f;

		static constexpr int frameWidth_ = 130;
		static constexpr int frameHeight_ = 95;
		static constexpr int framePerDirection_ = 3;
		static constexpr float spriteScale_ = 0.55f;
		static constexpr float collisionRadius_ = 10.0f;

	};
}