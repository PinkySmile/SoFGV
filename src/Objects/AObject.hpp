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
	struct Rectangle {
		Vector2f pt1;
		Vector2f pt2;
		Vector2f pt3;
		Vector2f pt4;

		bool intersect(const Rectangle &other);
		std::vector<std::vector<Vector2f>> getIntersectionPoints(const Rectangle &other);
		static bool intersect(const Vector2f &A, const Vector2f &B, const Vector2f &C, const Vector2f &D);
		bool isIn(const Rectangle &other);
	};

	class AObject : public IObject {
	protected:
		mutable Sprite _sprite;

		std::map<unsigned, std::vector<std::vector<FrameData>>> _moves;
		Vector2f _position = {0, 0};
		Vector2f _speed = {0, 0};
		Vector2f _gravity = {0, 0};
		Vector2f _airDrag = {0, 0};
		Vector2f _groundDrag = {0, 0};
		unsigned short _action = 0;
		unsigned short _actionBlock = 0;
		unsigned short _animation = 0;
		unsigned short _animationCtr = 0;
		short _hp = 0;
		float _rotation = 0;
		unsigned _team = 0;
		bool _dead = false;
		bool _hasHit = false;
		bool _direction = false;
		char _cornerPriority = 0;
		float _dir = -1;

		float _baseRotation = 0;
		Vector2f _baseGravity = {0, 0};
		Vector2f _baseGroundDrag = {0, 0};
		Vector2f _baseAirDrag = {0, 0};
		short _baseHp = 0;

		AObject() = default;
		void _drawBox(const Rectangle &box, const sf::Color &color) const;
		std::vector<Rectangle> _getModifiedBoxes(const FrameData &data, const std::vector<Box> &) const;
		std::vector<Rectangle> _getModifiedHurtBoxes() const;
		std::vector<Rectangle> _getModifiedHitBoxes() const;
		bool _hasMove(unsigned action) const;
		virtual void _forceStartMove(unsigned action);
		virtual void _onMoveEnd(const FrameData &lastData);
		virtual bool _canStartMove(unsigned action, const FrameData &data);
		virtual bool _startMove(unsigned action);
		virtual void _applyNewAnimFlags();
		virtual bool _isGrounded() const;
		virtual Box _applyModifiers(Box box) const;
		virtual void _tickMove();
		virtual void _applyMoveAttributes();

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
		bool hits(const IObject &other) const override;
		void hit(IObject &other, const FrameData *data) override;
		void getHit(IObject &other, const FrameData *data) override;
		void collide(IObject &other) override;
		bool collides(const IObject &other) const override;
		const FrameData *getCurrentFrameData() const override;
		void kill() override;

		friend class BattleManager;
		friend class ACharacter;
	};
}


#endif //BATTLE_AOBJECT_HPP
