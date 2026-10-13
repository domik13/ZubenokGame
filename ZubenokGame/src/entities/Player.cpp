#include "zubrenok/entities/Player.h"

#include "zubrenok/game/GameTypes.h"
#include "zubrenok/levels/Level.h"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cmath>
#include <stdexcept>

namespace zubrenok
{
	Player::Player()
		: sprite_(texture_)
	{
	}

	void Player::load()
	{
		sf::Image image;

		if (!image.loadFromFile("assets/textures/zubrenok.png"))
		{
			throw std::runtime_error(
				"Failed to load assets/textures/zubrenok.png"
			);
		}

		// Sheet sits on a white canvas — make white transparent.
		image.createMaskFromColor(sf::Color::White);

		if (!texture_.loadFromImage(image))
		{
			throw std::runtime_error(
				"Failed to create player texture"
			);
		}

		texture_.setSmooth(false);

		updateSpriteFrame();

		sprite_.setOrigin(
			{
				frameWidth_ / 2.0f,
				frameHeight_ / 2.0f
			}
		);

		sprite_.setScale(
			{
				spriteScale_,
				spriteScale_
			}
		);

		speed_ = config::playerSpeed;

		if (!footstepBuffer_.loadFromFile("assets/audio/zubr-step.wav")) {
			throw std::runtime_error(
				"Failed to load assets/audio/zubr-step.wav"
			);
		}

		footstepSound_.setVolume(10.0f);
	}

	void Player::setPosition(sf::Vector2f position)
	{
		sprite_.setPosition(position);
	}

	sf::Vector2f Player::getPosition() const
	{
		return sprite_.getPosition();
	}

	CollectibleType Player::update(float deltaTime, Level& level)
	{
		sf::Vector2f movement{ 0.0f, 0.0f };

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			movement.y -= 1.0f;
			facing_ = Facing::Up;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			movement.y += 1.0f;
			facing_ = Facing::Down;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			movement.x -= 1.0f;
			facing_ = Facing::Left;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			movement.x += 1.0f;
			facing_ = Facing::Right;
		}

		const bool wantsToMove =
			movement.x != 0.0f || movement.y != 0.0f;

		bool moved = false;

		if (wantsToMove)
		{
			const float length = std::sqrt(
				movement.x * movement.x + movement.y * movement.y
			);
			movement /= length;

			const sf::Vector2f current = sprite_.getPosition();
			const sf::Vector2f delta = movement * speed_ * deltaTime;

			sf::Vector2f next = current;
			next.x += delta.x;
			if (level.isWorldPositionWalkable(next, collisionRadius_))
			{
				moved = true;
			}
			else
			{
				next.x = current.x;
			}

			next.y += delta.y;
			if (level.isWorldPositionWalkable(next, collisionRadius_))
			{
				moved = true;
			}
			else
			{
				next.y = current.y;
			}

			sprite_.setPosition(next);
		}

		if (moved)
		{
			footStepTimer_ += deltaTime;

			if (footStepTimer_ >= footstepInterval_)
			{
				footStepTimer_ = 0.0f;
				footstepSound_.play();
			}
		}
		else {
			footStepTimer_ = footstepInterval_;
		}

		updateAnimation(deltaTime, moved);
		return level.consumeCollectible(sprite_.getPosition());
	}

	void Player::updateAnimation(float deltaTime, bool moving)
	{
		if (!moving)
		{
			animationTimer_ = 0.0f;
			animationFrame_ = 1;
			updateSpriteFrame();
			return;
		}

		animationTimer_ += deltaTime;
		if (animationTimer_ >= animationFrameTime_)
		{
			animationTimer_ -= animationFrameTime_;
			animationFrame_ = (animationFrame_ + 1) % framesPerDirection_;
			updateSpriteFrame();
		}
	}

	void Player::updateSpriteFrame()
	{
		int row = 0;
		int columnOffset = 0;

		switch (facing_)
		{
		case Facing::Right:
			row = 0;
			columnOffset = 0;
			break;
		case Facing::Left:
			row = 0;
			columnOffset = 3;
			break;
		case Facing::Up:
			row = 1;
			columnOffset = 0;
			break;
		case Facing::Down:
			row = 1;
			columnOffset = 3;
			break;
		}

		const int column = columnOffset + animationFrame_;

		sprite_.setTextureRect(
			sf::IntRect(
				{
					column * frameWidth_,
					row * frameHeight_
				},
				{
					frameWidth_,
					frameHeight_
				}
			)
		);
	}

	void Player::draw(sf::RenderTarget& target) const
	{
		target.draw(sprite_);
	}
}
