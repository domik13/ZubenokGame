#include "zubrenok/entities/Enemy.h"

#include "zubrenok/game/GameTypes.h"
#include "zubrenok/levels/Level.h"

#include <SFML/Graphics/Image.hpp>

#include <cstdlib>
#include <stdexcept>

namespace zubrenok
{
	Enemy::Facing Enemy::directionToFacing(Direction direction)
	{
		switch (direction)
		{
		case Direction::Right:
			return Facing::Right;
		case Direction::Left:
			return Facing::Left;
		case Direction::Up:
			return Facing::Up;
		case Direction::Down:
			return Facing::Down;
		default:
			return Facing::Down;
		}
	}

	Enemy::Enemy()
		: sprite_(texture_)
	{
	}

	void Enemy::load()
	{
		sf::Image image;
		if (!image.loadFromFile("assets/textures/wolf.png"))
		{
			throw std::runtime_error("Failed to load assets/textures/wolf.png");
		}

		image.createMaskFromColor(sf::Color::White);

		if (!texture_.loadFromImage(image))
		{
			throw std::runtime_error("Failed to create wolf texture");
		}

		texture_.setSmooth(false);
		updateSpriteFrame();

		sprite_.setOrigin(
			{
				frameWidth_ / 2.0f,
				frameHeight_ / 2.0f
			}
		);
		sprite_.setScale({ spriteScale_, spriteScale_ });

		speed_ = config::enemySpeed;
	}

	void Enemy::setPosition(sf::Vector2f position)
	{
		sprite_.setPosition(position);
	}

	sf::Vector2f Enemy::getPosition() const
	{
		return sprite_.getPosition();
	}

	void Enemy::pickNewDirection(Level& level)
	{
		const Direction candidates[] = {
			Direction::Up,
			Direction::Down,
			Direction::Left,
			Direction::Right
		};

		Direction options[4];
		int count = 0;

		const TilePosition tile = level.worldToTile(sprite_.getPosition());

		for (Direction dir : candidates)
		{
			if (level.canMove(tile, dir))
			{
				options[count++] = dir;
			}
		}

		if (count == 0)
		{
			moveDir_ = Direction::None;
			return;
		}

		moveDir_ = options[std::rand() % count];
		facing_ = directionToFacing(moveDir_);
		rethinkTimer_ = 1.5f + static_cast<float>(std::rand() % 150) / 100.0f;
	}

	void Enemy::update(float deltaTime, Level& level)
	{
		rethinkTimer_ -= deltaTime;

		const TilePosition tile = level.worldToTile(sprite_.getPosition());

		if (moveDir_ == Direction::None ||
			rethinkTimer_ <= 0.0f ||
			!level.canMove(tile, moveDir_))
		{
			pickNewDirection(level);
		}

		bool moved = false;

		if (moveDir_ != Direction::None)
		{
			const sf::Vector2f delta =
				directionToVector(moveDir_) * speed_ * deltaTime;

			sf::Vector2f next = sprite_.getPosition();
			next.x += delta.x;

			if (level.isWorldPositionWalkable(next, collisionRadius_))
			{
				moved = true;
			}
			else
			{
				next.x = sprite_.getPosition().x;
			}

			next.y += delta.y;

			if (level.isWorldPositionWalkable(next, collisionRadius_))
			{
				moved = true;
			}
			else
			{
				next.y = sprite_.getPosition().y;
			}

			sprite_.setPosition(next);

			if (!moved)
			{
				pickNewDirection(level);
			}
		}

		updateAnimation(deltaTime, moved);
	}

	void Enemy::updateAnimation(float deltaTime, bool moving)
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

	void Enemy::updateSpriteFrame()
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

	void Enemy::draw(sf::RenderTarget& target) const
	{
		target.draw(sprite_);
	}
}
