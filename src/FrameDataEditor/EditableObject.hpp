//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_EditableObject_HPP
#define BATTLE_EditableObject_HPP


#include "../Objects/IObject.hpp"
#include "../Resources/Sprite.hpp"
#include "../Resources/FrameData.hpp"

namespace Battle
{
	class EditableObject : public IObject {
	public:
		mutable Sprite _sprite;

		std::map<unsigned, std::vector<std::vector<FrameData>>> _moves;
		Vector2f _position = {0, 0};
		Vector2f _speed = {0, 0};
		float _gravity = 0;
		unsigned _action = 0;
		unsigned _actionBlock = 0;
		unsigned _animation = 0;
		unsigned _animationCtr = 0;

		EditableObject() = default;
		EditableObject(const std::string &frameData);
		~EditableObject() override = default;
		void render() const override;
		void update() override;
		bool collides(const IObject &other) const override;
		void reset() override;
		bool isDead() const override;
		bool hits(const IObject &other) const override;
		void hit(IObject &other, const FrameData *data) override;
		void getHit(IObject &other, const FrameData *data) override;
		const FrameData *getCurrentFrameData() const override;
		void collide(IObject &other) override;
		void kill() override;
	};
}


#endif //BATTLE_EditableObject_HPP
