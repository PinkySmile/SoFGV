//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_EditableObject_HPP
#define BATTLE_EditableObject_HPP


#include <SoFGV.hpp>

class EditableObject : public Battle::IObject {
public:
	mutable Battle::Sprite _sprite;

	std::map<unsigned, std::vector<std::vector<Battle::FrameData>>> _moves;
	Battle::Vector2f _position = {0, 0};
	Battle::Vector2f _speed = {0, 0};
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
	bool collides(const Battle::IObject &other) const override;
	void reset() override;
	bool isDead() const override;
	bool hits(const Battle::IObject &other) const override;
	void hit(Battle::IObject &other, const Battle::FrameData *data) override;
	void getHit(Battle::IObject &other, const Battle::FrameData *data) override;
	const Battle::FrameData *getCurrentFrameData() const override;
	void collide(Battle::IObject &other) override;
	void kill() override;
	unsigned int getBufferSize() const override;
	void copyToBuffer(void *data) const override;
	void restoreFromBuffer(void *data) override;
	unsigned int getClassId() const override;
};


#endif //BATTLE_EditableObject_HPP
