//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_OBJECT_HPP
#define SOFGV_OBJECT_HPP


#include "IObject.hpp"
#include "Resources/Sprite.hpp"
#include "Resources/FrameData.hpp"

namespace SpiralOfFate
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

	class Object : public IObject {
	protected:
#pragma pack(push, 1)
		struct Data {
			Vector2f _position;
			Vector2f _speed;
			Vector2f _gravity;
			float _rotation;
			float _dir;
			unsigned _team;
			unsigned short _action;
			unsigned short _actionBlock;
			unsigned short _animation;
			unsigned short _animationCtr;
			unsigned short _hp;
			unsigned char _hitStop;
			bool _dead;
			bool _hasHit;
			bool _direction;
			char _cornerPriority;
		};
#pragma pack(pop)

		// Game State
		Vector2f _position = {0, 0};
		Vector2f _speed = {0, 0};
		Vector2f _gravity = {0, 0};
		float _rotation = 0;
		unsigned _team = 0;
		unsigned short _action = 0;
		unsigned short _actionBlock = 0;
		unsigned short _animation = 0;
		unsigned short _animationCtr = 0;
		unsigned short _hp = 0;
		unsigned char _hitStop = 0;
		bool _dead = false;
		bool _hasHit = false;
		bool _direction = false;
		char _cornerPriority = 0;
		float _dir = -1;

		// Non Game State
		mutable Sprite _sprite;
		Vector2f _airDrag = {0.99, 0.99};
		float _groundDrag = 0.75;
		std::map<unsigned, std::vector<std::vector<FrameData>>> _moves;
		float _baseRotation = 0;
		Vector2f _baseGravity = {0, 0};
		unsigned short _baseHp = 0;

		void _drawBox(const Rectangle &box, const sf::Color &color) const;
		std::vector<Rectangle> _getModifiedBoxes(const FrameData &data, const std::vector<Box> &) const;
		std::vector<Rectangle> _getModifiedHurtBoxes() const;
		std::vector<Rectangle> _getModifiedHitBoxes() const;
		bool _hasMove(unsigned action) const;
		virtual void _checkPlatforms(Vector2f oldPos);
		virtual void _forceStartMove(unsigned action);
		virtual void _onMoveEnd(const FrameData &lastData);
		virtual bool _canStartMove(unsigned action, const FrameData &data);
		virtual bool _startMove(unsigned action);
		virtual void _applyNewAnimFlags();
		virtual bool _isGrounded() const;
		virtual bool _isOnPlatform() const;
		virtual Box _applyModifiers(Box box) const;
		virtual void _tickMove();
		virtual void _applyMoveAttributes();
		virtual void _render(Vector2f spritePos, Vector2f scale) const;

	public:
		Object() = default;
		~Object() override = default;
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
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
		unsigned int getClassId() const override;

		friend class PracticeBattleManager;
		friend class BattleManager;
		friend class Character;
	};
}


#endif //SOFGV_OBJECT_HPP
