//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_AOBJECT_HPP
#define BATTLE_AOBJECT_HPP


#include "IObject.hpp"
#include "../Resources/Sprite.hpp"
#include "../Resources/FrameData.hpp"

namespace Battle
{
	class AObject : public IObject {
	protected:
		mutable Sprite _sprite;

		std::map<unsigned, std::vector<std::vector<FrameData>>> _moves;
		Vector2f _position = {0, 0};
		Vector2f _speed = {0, 0};
		Vector2f _gravity = {0, 0};
		unsigned short _action = 0;
		unsigned short _actionBlock = 0;
		unsigned short _animation = 0;
		unsigned short _animationCtr = 0;
		unsigned short _hp = 0;
		float _rotation = 0;
		unsigned _team = 0;
		bool _dead = false;
		bool _direction = false;

		float _baseRotation = 0;
		Vector2f _baseGravity = {0, 0};
		unsigned short _baseHp = 0;

		AObject() = default;
		Box applyModifiers(Box box) const;

	public:
		#ifdef _DEBUG
		bool showBoxes = true;
		#else
		bool showBoxes = false;
		#endif

		~AObject() override = default;
		void render() const override;
		void update() override;
		void reset() override;
		bool isDead() const override;
		void hit(IObject &other, const FrameData *data) override;
		bool hits(IObject &other) const override;
		void getHit(IObject &other, const FrameData *data) override;
		const FrameData *getCurrentFrameData() const override;
	};
}


#endif //BATTLE_AOBJECT_HPP
