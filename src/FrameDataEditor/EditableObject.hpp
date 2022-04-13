//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_EditableObject_HPP
#define BATTLE_EditableObject_HPP


#include <SoFGV.hpp>

class EditableObject : public SpiralOfFate::IObject {
public:
	mutable SpiralOfFate::Sprite _sprite;

	std::map<unsigned, std::vector<std::vector<SpiralOfFate::FrameData>>> _moves;
	SpiralOfFate::Vector2f _position = {0, 0};
	SpiralOfFate::Vector2f _speed = {0, 0};
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
	bool collides(const SpiralOfFate::IObject &other) const override;
	void reset() override;
	bool isDead() const override;
	bool hits(const SpiralOfFate::IObject &other) const override;
	void hit(SpiralOfFate::IObject &other, const SpiralOfFate::FrameData *data) override;
	void getHit(SpiralOfFate::IObject &other, const SpiralOfFate::FrameData *data) override;
	const SpiralOfFate::FrameData *getCurrentFrameData() const override;
	void collide(SpiralOfFate::IObject &other) override;
	void kill() override;
	unsigned int getBufferSize() const override;
	void copyToBuffer(void *data) const override;
	void restoreFromBuffer(void *data) override;
	unsigned int getClassId() const override;
};


#endif //BATTLE_EditableObject_HPP
