//
// Created by PinkySmile on 18/09/2021.
//

#ifndef BATTLE_EditableObject_HPP
#define BATTLE_EditableObject_HPP


#include <LibCore.hpp>

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
	std::string _folder;

	EditableObject() = default;
	EditableObject(const std::string &frameData);
	~EditableObject() override = default;
	void render() const override;
	void update() override;
	bool isDead() const override;
	void kill() override;
	unsigned int getBufferSize() const override;
	void copyToBuffer(void *data) const override;
	void restoreFromBuffer(void *data) override;
	unsigned int getClassId() const override;
	size_t printDifference(const char *msgStart, void *, void *, unsigned) const override;
	size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
	int getLayer() const override;
};


#endif //BATTLE_EditableObject_HPP
