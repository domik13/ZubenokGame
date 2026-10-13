#pragma once

#include "zubrenok/game/GameTypes.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace zubrenok
{
	class Level;

	class Player
	{
	public:
		Player();

		void load();
		void setPosition(sf::Vector2f position);
		sf::Vector2f getPosition() const;

		CollectibleType update(float deltaTime, Level& level);
		void draw(sf::RenderTarget& target) const;

	private:
		sf::SoundBuffer footstepBuffer_;
		sf::Sound footstepSound_{ footstepBuffer_ };

		float footStepTimer_ = 0.0f;
		static constexpr float footstepInterval_ = 0.28f;

		enum class Facing
		{
			Right,
			Left,
			Up,
			Down
		};

		void updateAnimation(float deltaTime, bool moving);
		void updateSpriteFrame();

		sf::Texture texture_;
		sf::Sprite sprite_;

		Facing facing_ = Facing::Down;

		int animationFrame_ = 1;

		float speed_ = 220.0f;
		float animationTimer_ = 0.0f;
		float animationFrameTime_ = 0.12f;

		// Sprite sheet layout (top-left of zubrenok.png):
		// row 0: side Right (0-2), side Left (3-5)
		// row 1: Up/back (0-2), Down/front (3-5)
		static constexpr int frameWidth_ = 130;
		static constexpr int frameHeight_ = 95;
		static constexpr int framesPerDirection_ = 3;
		static constexpr float spriteScale_ = 0.55f;
		static constexpr float collisionRadius_ = 10.0f;
	};
}
